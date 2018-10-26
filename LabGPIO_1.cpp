#include "LabGPIO_1.hpp"

LabGPIO_1::LabGPIO_1(uint8_t vpin)
{
	pin = vpin;
}

void LabGPIO_1::setAsInput()
{
	LPC_GPIO1->FIODIR &= ~(1 << pin);
}

void LabGPIO_1::setAsOutput()
{
    LPC_GPIO1->FIODIR |= (1 << pin);
}

void LabGPIO_1::setDirection(bool output)
{
	if (output)
    {
        LPC_GPIO1->FIODIR  |= (1 << pin);
    }
    else
    {
        LPC_GPIO1->FIODIR &= ~(1 << pin);
    }
}

void LabGPIO_1::setHigh()
{
	LPC_GPIO1->FIOSET = (1 << pin);
}

void LabGPIO_1::setLow()
{
	LPC_GPIO1->FIOCLR = (1 << pin);
}

void LabGPIO_1::set(bool high)
{
	if (high)
	{
	    LPC_GPIO1->FIOSET = (1 << pin);
	}
    else
    {
        LPC_GPIO1->FIOCLR = (1 << pin);
    }
}

void LabGPIO_1::toggle()
{
    LPC_GPIO1->FIOPIN ^= (1 << pin);
}

bool LabGPIO_1::getLevel()
{
    bool ret;

    if (LPC_GPIO1->FIOPIN & (1 << pin))
    {
        ret = 1;
    }
    else
    {
        ret =  0;
    }

    return ret;
}

LabGPIO_1::~LabGPIO_1()
{
}