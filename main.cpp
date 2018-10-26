#include <stdio.h>
#include "tasks.hpp"
#include "utilities.h"
#include "io.hpp"

#include <string.h>
#include "LabGPIO_0.hpp"
#include "adcDriver.h"
#include "LabSPI.h"
#include "ff.h"
#include "storage.hpp"
#include "semphr.h"

#include "uart0_min.h"
#include "lcd.h"
#include "ssp1.h"
#include <string.h>

SemaphoreHandle_t xPauseSem;
SemaphoreHandle_t xVolSem;
TaskHandle_t file_handle;

bool vol_flag = 0;
uint16_t volume_def = 0x7F7F;

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

//Macro for DREQ
#define DREQ (LPC_GPIO1->FIOPIN & (1 << 29))

//split 16 bits into 8 bits for transfer
#define LOWER_BYTE(h) (char) (h)
#define UPPER_BYTE(h) (char) (h >> 8)

//SCI read and write modes
#define WRITE_OP            0x02
#define READ_OP             0x03

//SCI register addresses
#define MODE_ADDR           0x00
#define STATUS_ADDR         0x01
#define BASS_ADDR           0x02
#define CLOCKF_ADDR         0x03
#define DECODE_TIME_ADDR    0x04
#define AUDATA_ADDR         0x05
#define WRAM_ADDR           0x06
#define WRAMADDR_ADDR       0x07
#define HDAT0               0x08
#define HDAT1               0x09
#define AIADDR_ADDR         0x0A
#define VOL_ADDR            0x0B
#define AICTRL0_ADDR        0x0C
#define AICTRL1_ADDR        0x0D
#define AICTRL2_ADDR        0x0E
#define AICTRL3_ADDR        0x0F

ADCDriver ADC;
LabSPI SPI;

void gpio_init()
{
    LPC_PINCON->PINSEL1 &= ~(3 << 20); 
    LPC_GPIO0->FIODIR |= (1 << 26); //P0.26 GPIO out, RESET

    LPC_PINCON->PINSEL3 &= ~(3 << 30);
    LPC_GPIO1->FIODIR |= (1 << 31); //P1.31 GPIO out, XCS

    LPC_PINCON->PINSEL1 &= ~(3 << 28);
    LPC_GPIO1->FIODIR |= (1 << 30); //P1.30 GPIO out, XDCS

    LPC_PINCON->PINSEL3 &= ~(3 << 26);
    LPC_GPIO1->FIODIR &= ~(1 << 29); //P1.29 GPIO in, DREQ

    /*LPC_PINCON->PINSEL1 &= ~(3 << 20); 
    LPC_GPIO0->FIODIR &= ~(1 << 30); //P0.26 GPIO out, RESET*/

    //testing using button in place of DREQ
    /*LPC_PINCON->PINSEL3 &= ~(3 << 18);
    LPC_GPIO1->FIODIR &= ~(1 << 9); //P1.29 GPIO in, DREQ*/
}

void reset_mp3()
{
    LPC_GPIO0->FIOCLR = (1 << 26); //active low reset
    delay_ms(2);
    LPC_GPIO0->FIOSET = (1 << 26);
}

//SCI chip select
void CS()
{
    ssp1_set_max_clock(14);
    LPC_GPIO1->FIOCLR = (1 << 31); //active low CS
}
void d_CS()
{
    LPC_GPIO1->FIOSET = (1 << 31); //deselect CS
}

//SDI chip select
void DCS()
{
    ssp1_set_max_clock(7);
    LPC_GPIO1->FIOCLR = (1 << 30); //active low DCS
}
void d_DCS()
{
    LPC_GPIO1->FIOSET = (1 << 30); //deselect DCS
}

void write_SCI(char addr, unsigned int num)
{
    while (!(DREQ));
    d_DCS();
    CS();
    SPI.transfer(WRITE_OP);
    SPI.transfer(addr);
    SPI.transfer(UPPER_BYTE(num));
    //while (!(DREQ));
    SPI.transfer(LOWER_BYTE(num));
    d_CS();
}

//if DREQ is high then send SDI data or SCI command
void write_SDI_32(char buf[], int size)
{
    d_CS();
    DCS();
    int index = 0;

    while (index < size)
    {
        DCS();
        while (!(DREQ)); //wait for DREQ
        do
        {
            SPI.transfer(buf[index]);
            index++;
        } while (index%32 != 0);
        d_DCS();
    }
}

bool mp3_init()
{   
    gpio_init();
    SPI.init(static_cast<LabSPI::Peripheral>(1), 8, static_cast<LabSPI::FrameModes>(0), 4);
    reset_mp3();

    while (!(DREQ));

    write_SCI(MODE_ADDR, 0x4842); //LINE1, SPI mode, Stream mode, MPEG
    write_SCI(CLOCKF_ADDR, 0xe000); //clock init
    write_SCI(VOL_ADDR, 0x7F7F); //volume init, MSB controls left channel, LSB controls right channel
    
    return 1;
}

/*void volume_ISR()
{
    xSemaphoreGiveFromISR(xVolSem, NULL);
    //how are ADC interupts cleared?
    //LPC_GPIOINT -> IO0IntClr = (1 << 29);
    portYIELD_FROM_ISR(0);
}

void volume_task(void *p)
{ 
    ADC.adcInitBurstMode();
    ADC.adcSelectPin(3);
    xVolSem = xSemaphoreCreateBinary();
    uint16_t volume;

    duty_cycle = (ADC.readADCVoltageByChannel()/3.3)*100;
    volume = 0xFEFE*duty_cycle;
    //might need to check int by channel vs DONE flag
    LPC_ADC->ADINTEN |= (1 << 3); //enable interrupt on ADC channel 3

    while (1)
    {
        if (xSemaphoreTake(xVolSem, portMAX_DELAY))
        {
            printf("%x\n", volume);
            write_SCI(VOL_ADDR, volume);
        }
    }
}*/

void select_song(FILE *fp, char* txt_buffer)
{
    fgets(txt_buffer, 32, fp);
}

void pause_ISR()
{
    xSemaphoreGiveFromISR(xPauseSem, NULL);
    LPC_GPIOINT -> IO0IntClr = (1 << 29);
    portYIELD_FROM_ISR(0);
}

void pause_task(void *p)
{
    xPauseSem = xSemaphoreCreateBinary();
    bool pause_flag = 0;
    while (1)
    {
        if (xSemaphoreTake(xPauseSem,portMAX_DELAY))
        {
            if (pause_flag)
            {
                uart0_puts("pause\n");
                vTaskSuspend(file_handle);
                pause_flag = 0;
                vTaskDelay(100);
                xSemaphoreTake(xPauseSem, 0);
            }               
            else
            {
                uart0_puts("resume\n");
                vTaskResume(file_handle);
                pause_flag = 1;
                vTaskDelay(100);
                xSemaphoreTake(xPauseSem, 0);
            }     
        }
    }
}

char* scan_files ( char* path)        /* Start node to be scanned (***also used as work area***) */
{
    FILE * fp;
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    fp = fopen("1:songlist.txt", "w+");
    if (fp == NULL)
    {
        printf("error\n");
        exit(1);
    }

    int cnt = 0;
    char filenames[2];

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {  //how long?
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                //res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                         
                if (fno.fname[strlen(fno.fname)-1] == '3')
                {       
                    fprintf(fp, "1:%s\n", fno.fname);
                //printf("last char %c\n", fno.fname[strlen(fno.fname)-1]);
                //printf("%s%s\n", path, fno.fname);
                }
            }
        }
        f_closedir(&dir);
    }
    fclose(fp);
    return filenames;
}


void file_task(void *p)
{
    mp3_init();
    char buffer[1024];
    unsigned int fp_position = 0; //position of file pointer
    int size = ARRAY_SIZE(buffer);

    FATFS fs;
    FRESULT res;
    char buff[256];

    res = f_mount(&fs, "", 1);
    if (res == FR_OK)
    {
        strcpy(buff,"1:");
        scan_files(buff);
    }

    FILE *fp;
    char txt_buffer[32];
    fp = fopen("1:songlist.txt", "r");
    if (fp == NULL)
    {
        printf("error\n");
        exit(1);
    }

    select_song(fp, txt_buffer);
    lcd_run(txt_buffer);

    while(1)
    {
        //cout << Storage::read(txt_buffer, buffer, sizeof(buffer), fp_position) << endl;
        while (Storage::read(txt_buffer, buffer, sizeof(buffer), fp_position) == 0)
        {
            fp_position += sizeof(buffer);
            write_SDI_32(&buffer[0], size);
            //tweak delay to lower CPU usage
            vTaskDelay(70);
        }
        fp_position = 0;
    }

}

int main(void)
{
    SPI.init(static_cast<LabSPI::Peripheral>(1), 8, static_cast<LabSPI::FrameModes>(0), 2);

    //gpio interrupt for pause
    isr_register(EINT3_IRQn, pause_ISR);
    NVIC_EnableIRQ(EINT3_IRQn);
    LPC_GPIOINT -> IO0IntEnR |= (1 << 29);

    /*//adc interrupt for volume
    isr_register(ADC_IRQn, volume_ISR);
    NVIC_EnableIRQ(ADC_IRQn);*/



    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
	xTaskCreate(file_task, "ft", 1024, NULL, 1, &file_handle);
    xTaskCreate(pause_task, "pt", 1024, NULL, 2, NULL);
    //xTaskCreate(lcd_task, "lt", 1024, NULL, 3, NULL);
    //xTaskCreate(volume_task, 'vt', 1024, NULL, 2, NULL);
    scheduler_start();
    return -1;

}


