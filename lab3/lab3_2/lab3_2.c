#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

#define ranger_addr 0xE0



void XBR0_Init(void);
void PCA_ISR(void) interrupt 9;
void PCA_Init(void);
void i2c_Init(void);
unsigned int ReadRanger(void);

unsigned char r_count = 0;
bit new_range = 0;
unsigned int cmrange = 0;
unsigned char Data[2];

void main(void)
{
	Sys_Init();
	putchar(' ');
	XBR0_Init();
	PCA_Init();
	i2c_Init();

	while(1)
	{
		if (new_range)
		{
			new_range = 0;
			cmrange = ReadRanger();
			Data[0] = 0x51;  //write 0x51 to reg 0 of the ranger:
			i2c_write_data(ranger_addr, 0,  Data, 1) ;  // write one byte of data to reg 0 at addr
			printf("range = %d\r\n", cmrange);
		}
	}
}

void i2c_Init(void)
{
	SMB0CR = 0x93; //set SCL to 100KHz
	ENSMB = TRUE;
}

unsigned int ReadRanger(void)
{
	unsigned int range =0;
	i2c_read_data(ranger_addr, 2, Data, 2);  // read two bytes, starting at reg 2
	range = (((unsigned int)Data[0] << 8) | Data[1]);
	return range; 
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
	XBR0 = 0x27; //configure crossbar with UART, SPI, SMBus, and CEX 0-3
}

void PCA_ISR ( void ) interrupt 9
{
	if (CF) 
	{
		CF = 0;
		
		PCA0L = 0xFF & 28762;
		PCA0H = 28762>>8;
		
		r_count++;
		if(r_count>=4) 
		{
			new_range=1;  // 4 overflows is about 80 ms
			r_count = 0;
		}
	}
}

void PCA_Init(void)
{
// reference to the sample code in Example 4.5 - Pulse Width Modulation implemented 
	PCA0MD = 0x81;
	PCA0CPM2 = 0xC2;
	PCA0CN = 0x40;
	EIE1 |= 0x08;
	EA = 1;
	
// Use a 16 bit counter with SYSCLK/12.
}
