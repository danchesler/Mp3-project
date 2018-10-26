
//Main.cpp
#include "LPC17xx.h"
#include "tasks.hpp"
#include "utilities.h"
#include <stdio.h>



/* Configure the data bus and Control bus as per the hardware connection */
#define LcdDataBusPort      LPC_GPIO2->FIOPIN
#define LcdControlBusPort   LPC_GPIO0->FIOPIN

#define LcdDataBusDirnReg   LPC_GPIO2->FIODIR
#define LcdCtrlBusDirnReg   LPC_GPIO0->FIODIR

// #define LCD_D4     19 //green
// #define LCD_D5     20 //blue
// #define LCD_D6     22 //purple
// #define LCD_D7     23 //grey

#define LCD_D4     0 //green
#define LCD_D5     1 //blue
#define LCD_D6     2 //purple
#define LCD_D7     3 //grey

#define LCD_RS     0 // red
#define LCD_RW     29 // orange, dont need, should tie to ground
#define LCD_EN     1 // yellow



/* Masks for configuring the DataBus and Control Bus direction */
#define LCD_ctrlBusMask ((1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_EN))
#define LCD_dataBusMask   ((1<<LCD_D4)|(1<<LCD_D5)|(1<<LCD_D6)|(1<<LCD_D7))

/* local function to generate some delay */

void delay(int cnt)
{
    int i;
    for(i=0;i<cnt;i++);
}

/* Function send the a nibble on the Data bus (LCD_D4 to LCD_D7) */
void sendNibble(char nibble)
{
    //printf("hello%x\n",nibble);
    //int Data = LcdDataBusPort;
    delay_ms(1);

    LcdDataBusPort&=~(LCD_dataBusMask); 
    delay_ms(1);
    // Clear previous data
    LcdDataBusPort|= (((nibble >>0x00) & 0x01) << LCD_D4);
    delay_ms(1);
    LcdDataBusPort|= (((nibble >>0x01) & 0x01) << LCD_D5);
    delay_ms(1);
    LcdDataBusPort|= (((nibble >>0x02) & 0x01) << LCD_D6);
    delay_ms(1);
    LcdDataBusPort|= (((nibble >>0x03) & 0x01) << LCD_D7);
    delay_ms(1);
}


/* Function to send the command to LCD. 
   As it is 4bit mode, a byte of data is sent in two 4-bit nibbles */
void Lcd_CmdWrite(char cmd)
{
    sendNibble((cmd >> 0x04) & 0x0F);  //Send higher nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN);

    delay(10000);

    sendNibble(cmd & 0x0F);            //Send Lower nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN); 

    delay(10000);
}



void Lcd_DataWrite(char dat)
{
    sendNibble((dat >> 0x04) & 0x0F);  //Send higher nibble
    LcdControlBusPort |= (1<<LCD_RS);  // Send HIGH pulse on RS pin for selecting data register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN);

    delay(10000);

    sendNibble(dat & 0x0F);            //Send higher nibble
    LcdControlBusPort |= (1<<LCD_RS);  // Send HIGH pulse on RS pin for selecting data register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN); 

    delay(10000);
}

void lcd_run(char * txt)
{
    LcdDataBusDirnReg = LCD_dataBusMask;  // Configure all the LCD pins as output
    LcdCtrlBusDirnReg = LCD_ctrlBusMask;
    
    //LcdDataBusPort |= 0xf;
    LcdDataBusDirnReg = LCD_dataBusMask;  // Configure all the LCD pins as output
    LcdCtrlBusDirnReg = LCD_ctrlBusMask;

    sendNibble(0x0);  //Send higher nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN);

    delay(10000);

    sendNibble(0x2);            //Send Lower nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN); 

    delay(10000);

    sendNibble(0x2);  //Send higher nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN);

    delay(10000);

    sendNibble(0x8);            //Send Lower nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN); 

    delay(10000);

    sendNibble(0x0);  //Send higher nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN);

    delay(10000);

    sendNibble(0x1);            //Send Lower nibble
    LcdControlBusPort &= ~(1<<LCD_RS); // Send LOW pulse on RS pin for selecting Command register
    LcdControlBusPort &= ~(1<<LCD_RW); // Send LOW pulse on RW pin for Write operation
    LcdControlBusPort |= (1<<LCD_EN);  // Generate a High-to-low pulse on EN pin
    delay(1000);
    LcdControlBusPort &= ~(1<<LCD_EN); 

    delay(10000);
    Lcd_CmdWrite(0x0c);
    //character
    char now[]={"Now Playing"};
    //char song[]={"Blink182"};
    
    for(int i=0;now[i]!=0;i++) //draw array
    {
        Lcd_DataWrite(now[i]);
    }
        
    Lcd_CmdWrite(0xc0); //change row on display
    for(int i=0;txt[i]!=0;i++) //draw array
    {
        Lcd_DataWrite(txt[i]);
    }

    /*for(int i=0;i < 40;i++)//shift left
    {
        if(i < 16){
            delay_ms(750);
        }
        Lcd_CmdWrite(0x18);
        Lcd_CmdWrite(0x0c);
    }*/
}

// void lcd_run()
// {
//     printf("lcd\n");
// }
