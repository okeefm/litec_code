#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

#define ranger_addr 0xE0
#define PW_MIN 2028
#define PW_MAX 3502
#define PW_NEUT 2765

void XBR0_Init(void);
void PCA_ISR(void) interrupt 9;
void PCA_Init(void);
void i2c_Init(void);
void Port_Init(void);
unsigned int ReadRanger(void);
void Drive_Motor(unsigned int);
void wait(unsigned int);
void Drive_Init(void);
void Range_Update(void);

unsigned int MOTOR_PW = 0;
unsigned char r_count = 0;
unsigned int count = 0;
bit new_range = 0;
unsigned int cmrange = 0;
unsigned char Data[2];
sbit at 0xB6 SS;


void main(void)
{
	Sys_Init();
	putchar(' ');
	XBR0_Init();
	PCA_Init();
	Port_Init();
	i2c_Init();
	Drive_Init();

	while(1)
	{	
		if (SS)
		{
			Range_Update();
			if (cmrange <= 10)
				Drive_Motor(PW_MAX);
			else if (cmrange >= 90)
				Drive_Motor(PW_MIN);
			else if ((cmrange >= 40) && (cmrange <=50))
				Drive_Motor(PW_NEUT);
			else
			{
				Drive_Motor(3502 - (18.425*(cmrange-10)));
			}
		}
		else
			Drive_Motor(PW_NEUT);

			wait(20);
	}
}

void Range_Update(void)
{
	if (new_range)
	{
		new_range = 0;
		cmrange = ReadRanger();
		Data[0] = 0x51;  //write 0x51 to reg 0 of the ranger:
		i2c_write_data(ranger_addr, 0,  Data, 1) ;  // write one byte of data to reg 0 at addr
	}
}
		

void Drive_Init(void)
{
	//set initial value
	MOTOR_PW = PW_NEUT;
	
	PCA0CPL2 = 0xFFFF - MOTOR_PW;
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
	wait(1000);
}

void wait(unsigned int waitTime)
{
	count = 0;
	while ((count * 20) <= waitTime)
	{
		printf("");
	}
	count = 0;
}

void Port_Init()
{
	P1MDOUT = 0x04 ; //set output pin (P1.2) for CEX2 in push-pull mode
	P3MDOUT &= ~0x40; //set Port 3, pin 6 to open-drain mode (input)
	P3 |= 0x40; //Write a logic high to P3.6
}

void Drive_Motor(unsigned int motorval)
{
	MOTOR_PW = motorval;
	PCA0CPL2 = 0xFFFF - MOTOR_PW;
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
	if (SS)
		printf("Range = %d, motor pulsewidth = %d\r\n", cmrange, motorval);
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
		
		count++;
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
