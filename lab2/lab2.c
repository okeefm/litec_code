/* Name: Michael O'Keefe
Date: 9-30-09 (finished)
LITEC Memory game: Lab 2, the enhanced version */

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
void portSet(bit, bit, bit, bit, bit, bit); //sets output device states
void setTimes(void); //uses the ADC value to set the times for the LEDs on/off
unsigned char random(void); //gets a pseudo-random number between 0 and 2
void lightLEDS(unsigned char); //sets the LEDs based on the random number assigned to that step
void wait(unsigned int); //uses Timer 0 to wait for the specified number of milliseconds
unsigned char readButtons(void); //reads the buttons, and outputs a number that denotes the button pressed
void checkPB3(void); //checks to see if pushbutton 3 is pressed, used at the end of the game


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
		portSet(0,1,0,0,0,0); //only the red LED should be on
		incorrect = 0; //clear the incorrect bit
		if(!PB3 || (continueGame)) //if either the pushbutton 3 is pressed (new game) or continueGame is set (continue)
		{
			if (!continueGame) //if it's a new game
			{
				printf("\r\nNew Game\r\n");
			}
			setTimes(); //read the ADC input, and set the LED on times and off times (in milliseconds)
			for (i = 0; i < numLights; i++)
			{
				randNums[i] = random(); //get random numbers for the randNums array
			}
			portSet(0,0,0,0,0,0); //all LEDs off
			
			wait(1000); //wait a second
			for (i = 0; i < numLights; i++) //lights the LEDs for step i
			{
				lightLEDS(randNums[i]);
				wait(onTime);
				//printf("Debug mode: led for step %d is %d\r\n", i+1, randNums[i]+1); *this is here for debugging/testing purposes*
				portSet(0,0,0,0,0,0);
				wait(offTime);
			}
			
			portSet(1,0,0,0,0,0);
			for (i = 0; i < numLights; i++) //read the button presses
			{
				buttonPress = readButtons();
				while (buttonPress == (255)) //set buttonPress equal to the pressed button
				{
					buttonPress = readButtons();
				}
				while(!PB0 || !PB1 || !PB2) 
				{
				}
				printf("You pressed button # %d for step %d\r\n", buttonPress+1, i+1); //print out the button they pressed
				if (randNums[i] != buttonPress) //if they made a mistake
				{
					printf("You pressed the incorrect button for step %d. The correct button should have been %d\r\n", i+1, randNums[i]+1);
					portSet(1,0,1,0,0,0); //sound the buzzer
					wait(1500);
					portSet(0,0,0,0,0,0);
					incorrect = 1; //set the incorrect bit
					numLights = 3; //reset the number of lights in the sequence
					continueGame = 0;
					break;
				}
				wait(200); //debouncing the switches, just in case
			}
			if(!incorrect) //if you get all of them right
			{
				printf("Congratulations! You won the level!\r\n");
				for (i = 0; i < 3; i++) //flash the lights 3 times
				{
					portSet(1,0,0,1,1,1);
					wait(200);
					portSet(0,0,0,0,0,0);
					wait(200);
				}
				numLights++; //add a light to the sequence (up the difficulty
				if (numLights > 15) //if you've reached the end
				{
					printf("Congratulations! You're a memory master!\r\n"); 
					numLights = 3;
					for (i = 0; i < 10; i++) //flash the lights
					{
						portSet(1,0,0,1,1,1);
						wait(200);
						portSet(0,0,0,0,0,0);
						wait(200);
					}
				}
				printf("If you would like to keep playing, press the start game pushbutton within 5 seconds\r\n");
				for (i = 0; i < 5; i++) //print out the number of seconds remaining
				{
					printf("%d\r\n", 5-i);
					checkPB3(); ///check to see if they want to continue
					if (continueGame)
					{
						printf("You are now memorizing a sequence of %d LEDs\r\n", numLights);
						break;
					}
					continueGame = 0; //reset the continueGame bit
				}
				if (!continueGame)
				{
					printf("Quitting.\r\n\r\n"); //if they don't want to keep going, quit
					numLights = 3;
				}
			}
		}
	}
}

void checkPB3() //check PB3 every 10 milliseconds, and if it's pressed exit
{
	continueGame = 0;
	for (j = 0; j < 100; j++)
	{
		if(!PB3)
		{
			continueGame = 1;
			break;
		}
		wait(10);
	}
	
}
unsigned char readButtons() //return the number of the button pressed. indexed at zero
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

void wait(unsigned int waitTime) // wait the specified amount of time. Note: wait time is in milliseconds
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

void lightLEDS(unsigned char n) //light the LED indicated by n
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

void ADC_Init(void) //initialize the port for the ADC
{
    REF0CN = 0x03;                     /* Set Vref to use internal reference voltage (2.4 V) */
    ADC1CN = 0x80;                     /* Enable A/D converter (ADC1) */
	ADC1CF &= ~0x01;
    ADC1CF |= 0x02;                    /* Set A/D converter gain to 1 */
}

unsigned char read_AD_input(unsigned char n) //read from the 10k pot
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

//initialize the ports as noted
void Port_Init(void)
{	
	// Port 3
	P3MDOUT = 0x3F;	// set Port 3, Pins 0-5 to push-pull mode (output)

	// Port 2
	P2MDOUT =0xE0; //set Port 2, Pins 0-4 to open drain mode (input)
	P2 |=0x1F;		// set Port 2, Pins 0-4 to a high impedance state (input)

	//Port 1
	P1MDIN &= ~0x01; //Set Port 1, Pin 0 to analog input
	P1MDOUT &= ~0x01; //Set Port 1, Pin 0 to open drain mode (input)
	P1 |= 0x01; //Set Port 1, Pin 0 to logic high
	
}

//get the Timer 0 interrupt working
void Interrupt_Init(void)
{	
	IE |= 0x02; // enable Timer0 Interrupt request 
	EA = 1; // enable global interrupts 
}

//Set up all the bits for the timer
void Timer_Init(void)
{
	
	CKCON |= 0x08; 	// Timer0 uses SYSCLK as source
	TMOD &= 0xF0;		// clear the 4 least significant bits
	TMOD |= 0x01;	 	// Timer0 in mode 1
	TR0 = 0;		// Stop Timer0
	TL0 = 0; 		// Clear low byte of register T0
	TH0 = 0;		// Clear high byte of register T0

}	

//Interrupt service routine for Timer 0
void Timer0_ISR(void) interrupt 1
{
	TF0 = 0; // clear interrupt request 
	Count++; // increment overflow counter 
}	