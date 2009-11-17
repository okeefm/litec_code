/*lab 5 code
altmaw@rpi.edu--Walter Altmann's email
*/

#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>

#define ranger_addr 0xE0
#define PW_MIN 2028
#define PW_MAX 3502
#define PW_NEUT 2765
#define MIN_VOLT 6

void Port_Init(void);
void PCA_Init (void);
void SMB_Init(void);
void XBR0_Init(void);
void Drive_Init(void);
void wait(unsigned int);
unsigned int ReadRanger(void);
void ADC_Init();
unsigned char read_AD_input(unsigned char);
int findVoltage(void);
void Drive_Motor(unsigned int);
void Range_Update(void);
void PCA_ISR ( void ) interrupt 9;
void ranger_pd_init(void);
unsigned char get_keypress(void);
unsigned int ranger_pd(void);
void voltage_update(void);
int findvoltage(void);

unsigned int MOTOR_PW = 0;
unsigned char r_count = 0;
unsigned int count = 0;
bit new_range = 0;
unsigned int cmrange = 0;
unsigned char Data[2];
int adinput = 0;
unsigned int adcount = 0;
int voltage = 0;
sbit at 0xB6 SS;
unsigned char neutral_range = 50;
unsigned int ranger_kp = 0;
unsigned int ranger_kd = 0;
long int ranger_error = 0;

void main(void)
{
	Sys_Init();
	putchar(' ');
	XBR0_Init();
	SMB_Init();
	PCA_Init();
	Drive_Init();
	Port_Init();
	ADC_Init();
	ranger_pd_init();
	//fan angle initialization code goes here
	
	while(1)
	{
		voltage_update();
		if(SS)
		{
			Range_Update(); //update the range
			Drive_Motor(ranger_pd());
		}
	else
		Drive_Motor(PW_NEUT); //if ss is not flipped, put it in neutral

	}
}

unsigned int ranger_pd(void)
{
	long int tmp_pw;
	long int curr_err = neutral_range - cmrange;
	tmp_pw = (long)ranger_kp*curr_err + (long)ranger_kd*(curr_err - ranger_error);
	if (tmp_pw > (long)PW_MAX)
		tmp_pw = PW_MAX;
	if (tmp_pw < (long)PW_MIN)
		tmp_pw = PW_MIN;
	ranger_error = curr_err;
	return (unsigned int)tmp_pw;
}

void voltage_update()
{
	voltage = findvoltage();
	if (voltage < MIN_VOLT)
	{
		Drive_Motor(PW_MIN);
		while(1){}
	}
}

void ranger_pd_init()
{
	unsigned char tempnum = 0;
	lcd_clear();
	lcd_print("Please input a 4-digit ranger KP\r\n");
	tempnum = get_keypress();
	ranger_kp = tempnum * 1000;
	tempnum = get_keypress();
	ranger_kp += tempnum * 100;
	tempnum = get_keypress();
	ranger_kp += tempnum * 10;
	tempnum = get_keypress();
	ranger_kp += tempnum * 1;
	
	lcd_clear();
	lcd_print("The ranger KP is %d\r\n", ranger_kp);
	wait(1000);
	
	lcd_clear();
	lcd_print("Please input a 4-digit ranger KD\r\n");
	tempnum = get_keypress();
	ranger_kd = tempnum * 1000;
	tempnum = get_keypress();
	ranger_kd += tempnum * 100;
	tempnum = get_keypress();
	ranger_kd += tempnum * 10;
	tempnum = get_keypress();
	ranger_kd += tempnum * 1;
	
	lcd_clear();
	lcd_print("The ranger KD is %d\r\n", ranger_kp);
	wait(1000);
	
	lcd_clear();
	lcd_print("the ranger KP is: %d and KD is: %d\r\n");
	lcd_print("are these the correct values?\r\n1 = yes, other = no");
	
	tempnum = get_keypress();
	if (tempnum == 1)
	{
	}
	else
	{
		ranger_pd_init();
	}
	
	return;
	
}

unsigned char get_keypress(void)
{
	char tempval = 0;
	unsigned char tempnum = 0;
	tempval = read_keypad();
	while(tempval == -1)
	{
		tempval = read_keypad();
	}
	tempnum = tempval - 48;
	while(tempval != -1)
	{
		tempval = read_keypad();
	}
	return tempnum;
}

void Drive_Init(void)
{
	//set initial value
	MOTOR_PW = PW_NEUT;
	
	PCA0CPL2 = 0xFFFF - MOTOR_PW; //set low byte of left fan CCM PW register
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL3 = 0xFFFF - MOTOR_PW; //set low byte of right fan CCM PW register
	PCA0CPH3 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	wait(1000); //make sure the motor sits in neutral for a second
}

void wait(unsigned int waitTime)
{
	count = 0; //reset count
	while ((count * 20) <= waitTime)
	{
		printf(""); //this is necessary. Not sure why.
	}
	count = 0;
}

void Range_Update(void)
{
	if (new_range)
	{
		new_range = 0; //reset the new_range flag
		cmrange = ReadRanger(); //get the range back from the ranger
		Data[0] = 0x51;  //write 0x51 to reg 0 of the ranger:
		i2c_write_data(ranger_addr, 0,  Data, 1) ;  // write one byte of data to reg 0 at addr
	}
}
		

void Drive_Motor(unsigned int motorval)
{	
	MOTOR_PW = motorval; //set the motor_pw to whatever value was passed in
	PCA0CPL2 = 0xFFFF - MOTOR_PW; //set low byte
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL3 = 0xFFFF - MOTOR_PW; //set low byte
	PCA0CPH3 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
}

unsigned int ReadRanger(void)
{
	unsigned int range =0;
	i2c_read_data(ranger_addr, 2, Data, 2);  // read two bytes, starting at reg 2
	range = (((unsigned int)Data[0] << 8) | Data[1]); //concatenate the two bytes.
	return range; 
}

void ADC_Init(void)
{
    REF0CN = 0x03;                     /* Set Vref to use internal reference voltage (2.4 V) */
    ADC1CN = 0x80;                     /* Enable A/D converter (ADC1) */
    ADC1CF |= 0x01;                    /* Set A/D converter gain to 1 */
}

unsigned char read_AD_input(unsigned char n) //note: on gondola; voltage divider on Pin 1.5
{
	AMX1SL = n;                        /* Set P1.n as the analog input for ADC1 */
    ADC1CN = ADC1CN & ~0x20;           /* Clear the “Conversion Completed” flag */
    ADC1CN = ADC1CN | 0x10;            /* Initiate A/D conversion */
    while ((ADC1CN & 0x20) == 0x00);   /* Wait for conversion to complete */
    return ADC1;                       /* Return digital value in ADC1 register */
}

void SMB_Init(void){
	SMB0CR=0x93;
	ENSMB=1;
}


int findVoltage(void){
	float advolt;
	adinput = read_AD_input(5); //read the voltage on pin 1.7 and convert it to an unsigned char
	advolt = adinput;
	advolt = advolt/.236; //do some math, get a float out between 0-15(V)
	return advolt;
}

void Port_Init(){
	P1MDOUT = 0xFF; //set output pin for CEX0 in push-pull mode
 
	P3MDOUT &= ~0xC0; //set Port 3, pin 6 and P3.7 to open-drain mode (input)
	P3 |= 0xC0; //Write a logic high to P3.6 and P3.7

	P1MDIN &= ~0x20; //Set Port 1, Pin 5 to analog input
	P1MDOUT &= ~0x20; //Set Port 1, Pin 5 to open drain mode (input)
	P1 |= 0x20; //Set Port 1, Pin 5 to logic high

}

void XBR0_Init(){
XBR0 = 0x25;	//configure crossbar with UART, SPI, SMBus, and CEX channels 
}

void PCA_Init(void)
{
         PCA0MD = 0x81;            	// SYSCLK/12, enable CF interrupts, suspend when idle
         PCA0CPM0 = 0xC2;			// 16 bit, enable compare, enable PWM
		 PCA0CPM2 = 0xC2;			// 16 bit, enable compare, enable PWM
		 PCA0CPM3 = 0xC2;			// 16 bit, enable compare, enable PWM
         EIE1 |= 0x08;            	// enable PCA interrupts
         PCA0CN |= 0x40;          	// enable PCA
         EA = 1;                  	// enable all interrupts

}


void PCA_ISR (void) interrupt 9
{
         if (CF){
                 PCA0L = 0xFF;     // low byte of start count
                 PCA0H = 0x6F;     // high byte of start count
                 CF = 0;           // Very important - clear interrupt flag
         }
         else PCA0CN &= 0xC0; // all other type 9 interrupts
		 
		 h_count++;
		 if(h_count>=2){
			new_heading=1; // 2 overflows is about 40 ms
			h_count = 0;
		 }
		 count++;
		r_count++;
		if(r_count>=4) 
		{
			new_range=1;  // 4 overflows is about 80 ms
			r_count = 0;
		}
		
		adcount++;
}