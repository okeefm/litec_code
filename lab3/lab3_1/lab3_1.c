/* 
Lab 3.1: PWM speed control with user input
Michael O'Keefe
10/16/09
*/

#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#define PW_MIN 2028
#define PW_MAX 3502
#define PW_NEUT 2765
//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init(void);
void Drive_Motor(void);
void wait(unsigned int);
unsigned int MOTOR_PW = 0;
unsigned int count = 0;
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
	// initialize board
	Sys_Init();
	putchar(' '); //the quotes in this line may not format correctly
	Port_Init();
	XBR0_Init();
	PCA_Init();
	
	//print beginning message
	printf("Embedded Control Drive Motor Control\r\n");
	
	//set initial value
	MOTOR_PW = PW_NEUT;
	
	PCA0CPL2 = 0xFFFF - MOTOR_PW;
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
	wait(1000);
	
	printf("Press f to increase the speed, and press s to decrease it\r\n");
	
	while(1)
		Drive_Motor();
}
//-----------------------------------------------------------------------------
// Drive_Motor
//-----------------------------------------------------------------------------
//
// Vary the pulsewidth based on the user input to change the speed 
// of the drive motor

void Drive_Motor()
{
	char input;
	//wait for a key to be pressed
	input = getchar();
	if(input == 'f') //if 'f' is pressed by the user
	{
		if(MOTOR_PW < PW_MAX)
		MOTOR_PW = MOTOR_PW + 10; //increase the speed pulsewidth by 10
		printf("\r\n f pressed, pulsewidth now %d\r\n", MOTOR_PW);
	}
	else if(input == 's') //if 's' is pressed by the user
	{
		if(MOTOR_PW > PW_MIN)
		MOTOR_PW = MOTOR_PW - 10; //decrease the speed pulsewidth by 10
		printf("\r\n s pressed, pulsewidth now %d\r\n", MOTOR_PW);
	}
	PCA0CPL2 = 0xFFFF - MOTOR_PW;
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
}

//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
	P1MDOUT = 0x04 ;//set output pin (P1.2) for CEX2 in push-pull mode
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

//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//ffff
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

//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR ( void ) interrupt 9
{
	if (CF)
	{
		CF = 0;
		
		PCA0L = 0xFF & 28762;
		PCA0H = 28762>>8;
		count++;
	}
	
	PCA0CN &= 0xC0;
}

void wait(unsigned int waitTime)
{
	count = 0;
	while ((count * 20) <= waitTime)
	{
		//printf("wait count = %d\r\n", count);
	}
	count = 0;
}