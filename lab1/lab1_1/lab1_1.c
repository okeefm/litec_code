/*	Names: Virginia Hickox, Michael O'Keefe
	Section: 4
	Date: 9-4-09
	File name: lab1_1.c
	Program description: This program turns on and off LEDs and a buzzer based on the status of several switches
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);		// Initialize ports for input and output
void Set_outputs(void); 				// function to set output bits
void portSet(bit, bit, bit, bit);		//function which sets the output devices on/off

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
sbit at 0xB6 LED0;	// LED0, associated with Port 3 Pin 6
sbit at 0xB4 BILED0;// BILED0, associated with Port 3 Pin 4
sbit at 0xB5 BILED1;// BILED1, associated with Port 5 Pin 5
sbit at 0xB3 BUZZER;// Buzzer, associated with Port 3 Pin 3
sbit at 0xA0 SS; 	// Slide switch, associated with Port 2 Pin 0
sbit at 0xA1 PB0;	// Push button 0, associated with Port 2, Pin 1
sbit at 0xA2 PB1;	// Push button 1, associated with Port 2, Pin 2


//***************
void main(void)
{
	Sys_Init();				 		// System Initialization
	putchar(' '); 				 		// the quote fonts may not copy correctly into SiLabs IDE
	Port_Init();		 				// Initialize ports 2 and 3 
	
	while (1) 		// infinite loop 
		{
		// main program manages the function calls

			Set_outputs();
		}
}


//***************
/* Port_Init - Initializes Ports 2 and 3 in the desired modes for input and output */

void Port_Init(void)
{	
	// Port 3
	P3MDOUT = 0x3C;	// set Port 3, Pins 3-6 to push-pull mode (output)

	// Port 2
	P2MDOUT =0xF8; //set Port 2, Pins 0-2 to open drain mode (input)
	P2 |=0x07;		// set Port 2, Pins 0-2 to a high impedance state (input)
}

//***************
// Set outputs

void Set_outputs (void)
{ 

	if (SS)
		{
			portSet(1,0,0,0); //LED0 to on
			puts("LED0 on\n");
		}
		else
		{
			portSet(0,0,0,0); //Turn everything off
			if (!PB0)
			{
				if (!PB1)
				{
					portSet(0,0,0,1); //BUZZER to on
					puts("BUZZER on\n");
				}
				else
				{
					portSet(0,1,0,0); //BILED0 to on
					puts("BILED green\n");
				}
			}
			else if (!PB1)
			{
				portSet(0,0,1,0); //BILED1 to on
				puts("BILED red\n");
			}
		}

}

/*portSet does the actual setting of the sbits, abstracting it away from the user a little bit.
0 in the function call = output device in that spot is off; 1 = output device on. */

void portSet(bit onLED0, bit onBILED0, bit onBILED1, bit onBUZZER)
{
	LED0 = (1-onLED0);
	BILED0 = (1-onBILED0);
	BILED1 = (1-onBILED1);
	BUZZER = (1-onBUZZER);
}

