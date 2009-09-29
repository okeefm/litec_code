/* Names: Michael O'Keefe, Virginia Hickox
9-21-09
LITEC Memory game: Lab 2 */

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>
#include <stdlib.h>

//Function Prototypes
void Port_Init(void);		// Initialize ports for input and output
void Timer_Init(void);		// Initialize Timer 0 
void Interrupt_Init(void); 	//Initialize interrupts
void Timer0_ISR(void) interrupt 1; //ISR for Timer 0
void ADC_Init(void); 		//Initialize ADC
unsigned char read_AD_input(unsigned char); //Reads ADC
void portSet(bit, bit, bit, bit, bit, bit);
void setTimes(void);
unsigned char random(void);
void lightLEDS(unsigned char);
void wait(unsigned int);
unsigned char readButtons(void);
void checkPB3(void);


//Declare Global Variables
sbit at 0xB0 BILED0;// BILED0, associated with Port 3, Pin 0
sbit at 0xB1 BILED1;// BILED1, associated with Port 3, Pin 1
sbit at 0xB2 BUZZER; //Buzzer, associated with Port 3, Pin 2
sbit at 0xB3 LED0;	// LED0, associated with Port 3, Pin 3
sbit at 0xB4 LED1; 	// LED1, associated with Port 3, Pin 4
sbit at 0xB5 LED2;	// LED2, associated with Port 3, Pin 5
sbit at 0xA0 PB0;	// Push button 0, associated with Port 2, Pin 0
sbit at 0xA1 PB1;	// Push button 1, associated with Port 2, Pin 1
sbit at 0xA2 PB2;	// Push button 2, associated with Port 2, Pin 2
sbit at 0xA3 PB3;	// Push button 3, associated with Port 2, Pin 3

//non-sbit variables
unsigned char adInput;
unsigned char onTime;
unsigned char offTime;
unsigned char randNums[15];
unsigned char i;
unsigned char buttonPress;
bit incorrect;
unsigned int Count;
unsigned char numLights;
bit continueGame;
unsigned char j;



void main(void)
{
	Sys_Init();			// System Initialization
	Port_Init();		// Initialize ports 2 and 3 
	Interrupt_Init();	//Interrupt Initialization
	Timer_Init();		// Initialize Timer 0 
	ADC_Init(); 		//Initialize ADC
	putchar(' ');
	numLights = 3;
	continueGame = 0;

	printf("\r\n\r\n\r\n\r\n\r\nReady to start! \r\n\r\n");
	
	while(1)
	{
		portSet(0,1,0,0,0,0);
		incorrect = 0;
		if(!PB3 || (continueGame))
		{
			if (!continueGame)
			{
				printf("\r\nNew Game\r\n");
			}
			setTimes();
			for (i = 0; i < numLights; i++)
			{
				randNums[i] = random();
			}
			portSet(0,0,0,0,0,0);
			
			wait(1000);
			for (i = 0; i < numLights; i++)
			{
				lightLEDS(randNums[i]);
				wait(onTime);
				printf("Debug mode: led for step %d is %d\r\n", i+1, randNums[i]+1);
				portSet(0,0,0,0,0,0);
				wait(offTime);
			}
			
			portSet(1,0,0,0,0,0);
			for (i = 0; i < numLights; i++)
			{
				buttonPress = readButtons();
				while (buttonPress == (255))
				{
					buttonPress = readButtons();
				}
				while(!PB0 || !PB1 || !PB2)
				{
				}
				printf("You pressed button # %d\r\n", buttonPress+1);
				if (randNums[i] != buttonPress)
				{
					printf("You pressed the incorrect button for step %d. The correct button should have been %d\r\n", i+1, randNums[i]+1);
					portSet(1,0,1,0,0,0);
					wait(1500);
					portSet(0,0,0,0,0,0);
					incorrect = 1;
					numLights = 3;
					continueGame = 0;
					break;
				}
				wait(200);
			}
			if(!incorrect)
			{
				printf("Congratulations! You won the level!\r\n");
				for (i = 0; i < 3; i++)
				{
					portSet(1,0,0,1,1,1);
					wait(200);
					portSet(0,0,0,0,0,0);
					wait(200);
				}
				numLights++;
				if (numLights > 15)
				{
					printf("Congratulations! You're a memory master!\r\n");
					numLights = 3;
					for (i = 0; i < 10; i++)
					{
						portSet(1,0,0,1,1,1);
						wait(200);
						portSet(0,0,0,0,0,0);
						wait(200);
					}
				}
				printf("If you would like to keep playing, press the start game pushbutton within 5 seconds\r\n");
				for (i = 0; i < 5; i++)
				{
					printf("%d\r\n", 5-i);
					checkPB3();
					if (continueGame)
					{
						printf("You are now memorizing a sequence of %d LEDs\r\n", numLights);
						break;
					}
					continueGame = 0;
				}
				if (!continueGame)
				{
					printf("Quitting.\r\n\r\n");
					numLights = 3;
				}
			}
		}
	}
}

void checkPB3()
{
	continueGame = 0;
	for (j = 0; j < 100; j++)
	{
		if(!PB3)
		{
			continueGame = 1;
		}
		wait(10);
	}
	
}
unsigned char readButtons()
{
	while(PB0 && PB1 && PB2)
	{
	}
	if (!PB0)
		return 0;
	if (!PB1)
		return 1;
	if (!PB2)
		return 2;
	else
		return -1;
}

void wait(unsigned int waitTime) //note: wait time is in milliseconds
{
		TR0 = 1;
		while((Count / .337) < (waitTime))
		{
			//printf("count = %d\r\n", Count);
		}
		TR0 = 0;
		Count = 0;
		return;
}

void lightLEDS(unsigned char n)
{
	switch(n)
	{
		case 0:
			portSet(0,0,0,1,0,0);
			break;
		case 1:
			portSet(0,0,0,0,1,0);
			break;
		case 2:
			portSet(0,0,0,0,0,1);
			break;
	}
}

void ADC_Init(void)
{
    REF0CN = 0x03;                     /* Set Vref to use internal reference voltage (2.4 V) */
    ADC1CN = 0x80;                     /* Enable A/D converter (ADC1) */
	ADC1CF &= ~0x01;
    ADC1CF |= 0x02;                    /* Set A/D converter gain to 1 */
}

unsigned char read_AD_input(unsigned char n)
{
	AMX1SL = n;                       /* Set P1.n as the analog input for ADC1 */
    ADC1CN = ADC1CN & ~0x20;           /* Clear the “Conversion Completed” flag */
    ADC1CN = ADC1CN | 0x10;            /* Initiate A/D conversion */
    while ((ADC1CN & 0x20) == 0x00);   /* Wait for conversion to complete */
    return ADC1;                       /* Return digital value in ADC1 register */
}

//sets output ports based on the values passed to it. ports are in same order as in sbit declaration.
void portSet(bit onBILED0, bit onBILED1, bit onBUZZER, bit onLED0, bit onLED1, bit onLED2)
{
	BILED0 = (1 - onBILED0);
	BILED1 = (1 - onBILED1);
	BUZZER = (1 - onBUZZER);
	LED0 = (1 - onLED0);
	LED1 = (1 - onLED1);
	LED2 = (1 - onLED2);
}

//set on and off times
void setTimes(void)
{
	adInput = read_AD_input(0);
	//printf("adInput = %d", adInput);
	onTime = (adInput * 5);
	onTime += 200;
	offTime = onTime/2;
}

/*return a random integer number between 0 and 2*/
unsigned char random(void)
{
	return (rand()%3); /*rand returns a random number between 0 and 32767*/
							/*the mod command (%) returns the remainder of dividing this */
							/*value by 3 and returns the result, a value of either 0, 1 or 2*/
}

void Port_Init(void)
{	
	// Port 3
	P3MDOUT = 0x3F;	// set Port 3, Pins 0-5 to push-pull mode (output)

	// Port 2
	P2MDOUT =0xE0; //set Port 2, Pins 0-4 to open drain mode (input)
	P2 |=0x1F;		// set Port 2, Pins 0-4 to a high impedance state (input)

	P1MDIN &= ~0x01; //Set Port 1, Pin 0 to analog input
	P1MDOUT &= ~0x01; //Set Port 1, Pin 0 to open drain mode (input)
	P1 |= 0x01; //Set Port 1, Pin 0 to logic high
	
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