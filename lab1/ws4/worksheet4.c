/* This program demonstrates the use of T0 interrupt. 
The code will count the number of T0 timer overflows that occur 
while a slide switch is in Off position */ 

#include <c8051_SDCC.h> // include files. You need to include stdio and c8051_SCDD. 
#include <stdio.h>                  // Add lines as needed

//------------------------------------------------------------- 
// Function PROTOTYPES 
//------------------------------------------------------------- 
void T0_ISR (void) interrupt 1; // Function Prototype for Interrupt Service Routine
void Port_Init(void); // Initialize ports for input and output 
void Timer_Init(); // Initialize Timer 0 

//------------------------------------------------------------- 
// Global variables 
//------------------------------------------------------------- 


// one end of bicolor LED0 is associated with Port 3 Pin 3 
sbit at 0xB4 BiLED0_1; 
// other end of bicolor LED0 is associated with Port 3 Pin 4 
sbit at 0xB5 BiLED0_2;

sbit at 0xA0 SW; // Slide Switch associated with Port 2 Pin 0 

int Counts = 0; 

//*************** 
void main(void) 
{ 
   Sys_Init(); // System Initialization Always do this first.
   putchar(' ');  // line added to allow printf statements
   Port_Init(); // Initialize port 2 and 3 
   Timer_Init(); // Initialize Timer 0 

   IE |= 0x02; // enable Timer0 Interrupt request 
   EA = 1; // enable global interrupts 
   BiLED0_1 = 0; // Turn off bicolor LED 
   BiLED0_2 = 0; 
   printf("Start\r\n"); 
   while (1) 
   { 
      if ( !SW ) 
      { 
         TR0 = 1; 
         BiLED0_1 = 1; // Turn ON bicolor LED 
         BiLED0_2 = 0; 
         while ( !SW ); // Wait while switch is activated 
         TR0 = 0; // Switch is no longer activated, stop counting 
         BiLED0_1 = 0; // Turn off bicolor LED 
         BiLED0_2 = 0; 
         printf("Number of Overflows = %d, Number of Seconds = %d\n", Counts, (int)(Counts * .0355)); 
         Counts = 0; 
         TL0 = 0; 
         TH0 = 0; 
      } 
   } 
}
 

void Port_Init(void) 
{ 

   // Port 3
	P3MDOUT = 0x3C;	// set Port 3, Pins 3-6 to push-pull mode (output)

	// Port 2
	P2MDOUT =0xF8; //set Port 2, Pins 0-2 to open drain mode (input)
	P2 |=0x07;		// set Port 2, Pins 0-2 to a high impedance state (input)
} 

void Timer_Init(void) 
{ 
   CKCON &= ~0x08; // Make T1 intact and T0 use SYSCLK/12 
   TMOD &= 0xF0; // Clear the 4 least significant bits 
   TMOD |= 0x01; // Make T1 intact and T0 use mode 1 
   TR0 = 0; // Stop Timer0 
   TL0 = 0; // Clear low byte of register T0 
   TH0 = 0; // Clear high byte of register T0 
} 

void T0_ISR ( void ) interrupt 1 //Interrupt service routine 
{ 
   TF0 = 0; // clear interrupt request 
   Counts++; // increment overflow counter 
}
 