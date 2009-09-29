/* 	Names: Michael O'Keefe, Virginia Hickox
	Section: 4
	Date: 9-18-09
	File name: lab1_2.c
	Description: Simple LED Game
*/
/*This program demonstrates the use of T0 interrupt. The code will count the 
number of T0 timer overflows that occur while a slide switch is in ON position*/

#include <c8051_SDCC.h>// include files. This file is available online
#include <stdio.h>
#include <stdlib.h>
								
//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void Port_Init(void);		// Initialize ports for input and output
void Timer_Init(void);		// Initialize Timer 0 
void Interrupt_Init(void); //Initialize interrupts
void Timer0_ISR (void) interrupt 1;
void newGame(void); //game function
void lightOn(unsigned char);
unsigned char inCheck(unsigned char);
unsigned char random(void);
void wait(unsigned char);
void ssCheck(void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
sbit at 0xB6 LED0;	// LED0, associated with Port 3 Pin 6
sbit at 0xB7 LED1; //LED1, associated with Port 3 Pin 7
sbit at 0xB4 BILED0;// BILED0, associated with Port 3 Pin 4
sbit at 0xB5 BILED1;// BILED1, associated with Port 5 Pin 5
sbit at 0xA0 SS; 	// Slide switch, associated with Port 2 Pin 0
sbit at 0xA1 PB0;	// Push button 0, associated with Port 2, Pin 1
sbit at 0xA2 PB1;	// Push button 1, associated with Port 2, Pin 2
//non-sbit 
unsigned int Count = 0;
unsigned int i = 0;
unsigned char numCorrect = 0;
bit printed = 0;
unsigned char lastRand = 0;
unsigned char randNum = 0;
bit timerFlag = 0;
bit newGameFlag = 0;
bit pausedFlag = 0;


//***************
void main(void)
{
	Sys_Init();		// System Initialization
	Port_Init();		// Initialize ports 2 and 3 
	Interrupt_Init();
	Timer_Init();		// Initialize Timer 0 
	
	putchar(' '); //
	printf("\r\n\r\nStart\r\n\r\n");

	LED0 = 1;
	LED1 = 1;

	while (1) 
	{
		while (i < 10)
		{
			ssCheck();
			printf("Turn %d of 10\r\n", i+1);
			randNum = random();
			while (randNum == lastRand)
			{
				randNum = random();
			}
			lastRand = randNum;
			ssCheck();
			lightOn(randNum);
			wait(1);
			ssCheck();
			LED0 = 1;
			LED1 = 1;
			if(inCheck(randNum))
			{
				ssCheck();
				numCorrect++;
				BILED0 = 0;
				BILED1 = 1;
			}
			else
			{
				ssCheck();
				BILED0 = 1;
				BILED1 = 0;
			}
			wait(1);
			LED0 = 1;
			LED1 = 1;
			i++;
		}
		if (!printed)
		{
			printf("You got %d correct, and %d incorrect\n", numCorrect, (10-numCorrect));
			printed = 1;
			ssCheck();
		}
		ssCheck();
	}
}

void ssCheck(void)
{
	while (SS)
		{
			if(TR0 = 1)
			{
				TR0 = 0;
				timerFlag = 1;
			}
			if (!pausedFlag && (i != 0) && (i != 10))
			{
				pausedFlag = 1;
				printf("Paused\r\n");
			}
			if (i == 10)
			{
				i = 0;
				numCorrect = 0;
				printed = 0;
				newGameFlag = 1;
			}
		}
		if (timerFlag)
		{
			TR0 = 1;
			timerFlag = 0;
		}
		if (newGameFlag)
		{
			printf("\r\nNew Game\r\n");
			newGameFlag = 0;
		}
		pausedFlag = 0;
		return;
}

void wait(unsigned char waitTime)
{
			TR0 = 1;
			while((Count * .00296) < waitTime)
			{	
				ssCheck();		
			}
			TR0 = 0;
			Count = 0;
}

void newGame(void)
{
	unsigned char numCorrect = 0;
	while (i < 10)
	{
		unsigned char randNum = random();
		lightOn(randNum);
		TR0 = 1;
		if ((Count * .00296) == 1)
		{
			TR0 = 0;
			if(inCheck(randNum))
			{
				numCorrect++;
			}
		}
		i++;
	}
	printf("You got %d correct, and %d incorrect\n", numCorrect, (10-numCorrect));
}

void lightOn(unsigned char randNum)
{
	if (randNum == 0)
	{
		LED0 = 0;
		LED1 = 1;
	}
	else if (randNum == 1)
	{
		LED1 = 0;
		LED0 = 1;
	}
	else
	{
		LED0 = 0;
		LED1 = 0;
	}
	
}

unsigned char inCheck(unsigned char randNum)
{
	if (randNum == 0)
	{
		if ((PB0 == 1) && (PB1 == 0))
			return 1;
	}
	else if (randNum == 1)
	{
		if ((PB0 == 0) && (PB1 == 1))
			return 1;
	}
	else if (randNum == 2)
	{
		if ((PB0 == 0) && (PB1 == 0))
			return 1;
	}
		return 0;	
}

//***************
void Port_Init(void)
{	
	// Port 3
	P3MDOUT = 0xDB;	// set Port 3, Pins 3-6 to push-pull mode (output)

	// Port 2
	P2MDOUT =0xF8; //set Port 2, Pins 0-2 to open drain mode (input)
	P2 |=0x07;		// set Port 2, Pins 0-2 to a high impedance state (input)
	
}

void Interrupt_Init(void)
{	
	IE |= 0x02; // enable Timer0 Interrupt request 
	EA = 1; // enable global interrupts 
}

//***************
void Timer_Init(void)
{
	
	CKCON |= 0x08; 	// Timer0 uses SYSCLK as source
	TMOD &= 0xF0;		// clear the 4 least significant bits
	TMOD |= 0x01;	 	// Timer0 in mode 1
	TR0 = 0;		// Stop Timer0
	TL0 = 0; 		// Clear low byte of register T0
	TH0 = 0;		// Clear high byte of register T0

}	

//***************
void Timer0_ISR(void) interrupt 1
{
	TF0 = 0; // clear interrupt request 
	Count++; // increment overflow counter 
}	


/************************************************************************************/

/*return a random integer number between 0 and 2*/
unsigned char random(void)
{
	return (rand()%3); /*rand returns a random number between 0 and 32767*/
							/*the mod command (%) returns the remainder of dividing this */
							/*value by 3 and returns the result, a value of either 0, 1 or 2*/
}
