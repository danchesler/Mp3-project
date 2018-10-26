#include "LabGPIO_0.hpp"

LabGPIO_0::LabGPIO_0(uint8_t vpin)
{
	pin = vpin;
}

void LabGPIO_0::setAsInput()
{
	LPC_GPIO0->FIODIR &= ~(1 << pin);
}

void LabGPIO_0::setAsOutput()
{
    LPC_GPIO0->FIODIR |= (1 << pin);
}

void LabGPIO_0::setDirection(bool output)
{
	if (output)
    {
        LPC_GPIO0->FIODIR  |= (1 << pin);
    }
    else
    {
        LPC_GPIO0->FIODIR &= ~(1 << pin);
    }
}

void LabGPIO_0::setHigh()
{
	LPC_GPIO0->FIOSET = (1 << pin);
}

void LabGPIO_0::setLow()
{
	LPC_GPIO0->FIOCLR = (1 << pin);
}

void LabGPIO_0::set(bool high)
{
	if (high)
	{
	    LPC_GPIO0->FIOSET = (1 << pin);
	}
    else
    {
        LPC_GPIO0->FIOCLR = (1 << pin);
    }
}

void LabGPIO_0::toggle()
{
    LPC_GPIO0->FIOPIN ^= (1 << pin);
}

bool LabGPIO_0::getLevel()
{
    bool ret;

    if (LPC_GPIO0->FIOPIN & (1 << pin))
    {
        ret = 1;
    }
    else
    {
        ret =  0;
    }

    return ret;
}

LabGPIO_0::~LabGPIO_0()
{
}