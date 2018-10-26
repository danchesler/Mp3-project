#include "LabSPI.h"

LabSPI::LabSPI()
{

}

void LabSPI::cs()
{
	//active low chip select
	//LPC_GPIO0->FIOCLR = (1 << 6);
	LPC_GPIO0->FIOCLR = (1 << 26);
}

void LabSPI::ds()
{
	//LPC_GPIO0->FIOSET = (1 << 6);
	LPC_GPIO0->FIOSET = (1 << 26);
}

bool LabSPI::init(Peripheral peripheral, uint8_t data_size_select, FrameModes format, uint8_t divide)
{

	LPC_SC->PCONP |= (1 << 10); //SSP1 power on
	LPC_SC->PCLKSEL0 &= ~(3 << 20); 
	LPC_SC->PCLKSEL0 |= (1 << 20);	//base clock for SSP1

	//previously commented out
	LPC_PINCON->PINSEL0 &= ~(0xFF << 12); //clear pins
	LPC_PINCON->PINSEL0 |= (1 << 19); // MOSI1
    LPC_PINCON->PINSEL0 |= (1 << 17); // MISO1
    LPC_PINCON->PINSEL0 |= (1 << 15); // SCK1

    //LPC_PINCON->PINSEL0 &= ~(3 << 12); //GPIO
    //LPC_PINCON->PINSEL0 &= ~(3 << 2); //GPIO
    //LPC_GPIO0-> FIODIR |= (1 << 6); //set GPIO as output
    //LPC_GPIO0-> FIODIR |= (1 << 1); //set GPIO as output

    //Check peripheral
    if ((peripheral > 1) || (peripheral < 0))
    {
	    return 0;
	}
	else
	{

		switch (peripheral)
	    {
	    	case SSP0:
	    		LPC_SC->PCONP |= (1 << 21); //SSP0 power on
	    	break;

	    	case SSP1:
	    		LPC_SC->PCONP |= (1 << 10); //SSP1 power on
	    	break;
	    }
	}

	//Check data size select
	//Change bits [3:0] of CR0 to data_size_select-1
	if ((data_size_select > 16) || (data_size_select < 4))
	{
		return 0;
	}
	else
	{
		LPC_SSP1->CR0 = (data_size_select-1);
	}

	//Set FrameMode; will be using SPI mostly
	if ((format > 2) || (format < 0))
	{
		return 0;
	}
	else
	{
		switch (format)
		{
			case SPIMode:
				LPC_SSP1->CR0 &= ~(3<<4);//set spi mode, 00
			break;
			
			case TI:
				LPC_SSP1->CR0 |= (1<<4);//set TI mode, 01
			break;

			case Microwire:
				LPC_SSP1->CR0 |= (2<<4);//set microwire mode, 10
			break;
		}
	}

	//check divide
	if ((divide < 2) || (divide%2 !=0) || divide > 254)
	{
		return 0;
	}
	else
	{
		LPC_SSP1->CPSR = divide;
	}

	LPC_SSP1->CR1 = (1 << 1); //enable master

	return 1;
}

char LabSPI::transfer(char send)
{
	LPC_SSP1->DR = send;
	
	while (LPC_SSP1->SR & (1 << 4));

	return LPC_SSP1->DR;
}

LabSPI::~LabSPI()
{
	
}