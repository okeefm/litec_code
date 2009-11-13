//modified 9/23/09 sjd
//LCD Model: Devantech LCD02
//LCD Documentation: http://www.robot-electronics.co.uk/htm/Lcd02tech.htm
/*
Blimp Hover
Fall 2004
Use this program and set a height to have the blimp hover and output values through UART
*/
		  
#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>
#include <stdlib.h>
//#include "blimp_init.h"
#include <i2c.h>
								
//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void Port_Init(void);		// Initialize ports for input and output


/*void SMB0_Init() //Moved this to the bottom
{
	SMB0CR = 0x93;					   // set SCL to 100KHz
	ENSMB = 1;					   // enable SMBUS0
}*/

//Stuff will added
void SMB0_Init(void);
void Timer_Init(void);
void Interrupt_Init(void);
void Timer0_ISR(void) interrupt 1;
unsigned int Counts;
void wait(void);
void pause(void);
//**//

//***************
void main(void)
{

	char keypad;
	Sys_Init();		// System Initialization
	Port_Init();	// Initialize ports 2 and 3 
//**//
	Interrupt_Init();
	Timer_Init();
//**//


	SMB0_Init();


	putchar(' '); //


	printf("Start\r\n");

	delay_time(100000);
	
	lcd_clear();
	lcd_print("Calibration:\nHello world!\n012_345_678:\nabc def ghij");
	while (1)
	{
	//i2c_write_data(0xC6, 0x00, "abc", 3);	
		keypad = read_keypad();
		pause(); //This pauses for 1 clock cycle -Will.
		if (keypad != -1)
		{
		lcd_clear();
		lcd_print("Your key was:\n %c", keypad);
		printf("Your key was: %c", keypad);
		}	
	P1_0 = 0;
	}
}

//***************
void Port_Init(void)
{	
	XBR0 = 0x27;
	P0MDOUT = 0x15;	
	P1MDOUT = 0x01;
}

//Will's Timer stuff
void Interrupt_Init(void)
{
	IE |= 0x02;
	EA = 1;
}

void Timer_Init(void)
{
	CKCON &= ~0x08;
	TMOD &= 0xF0;
	TMOD |= 0x01;
	TR0=0;
	TL0=0;
	TH0=0;
}

void Timer0_ISR(void) interrupt 1
{
	TF0 = 0;
	Counts++;
}
/// Wait and Pause
void wait(void)
{
	TR0=1;
	Counts=0;
	while(Counts<10){}
}
void pause(void)
{
	TR0=1;
	Counts=0;
	while(Counts <1){}
}

void SMB0_Init() //This was at the top, moved it here to call wait()
{
	wait(); //waits for 10 clock cycles, This might be able to go lower
	SMB0CR = 0x93;					   // set SCL to 100KHz
	ENSMB = 1;					   // enable SMBUS0
}