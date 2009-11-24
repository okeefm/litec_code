//---------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------
#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
//---------------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------------
#define ranger_addr 0xE0
#define comp_addr 0xC0
#define PW_MIN 2000
#define PW_MAX 3500
#define PW_NEUT 2750
#define MIN_VOLT 6
#define PW_CENTER 2750
#define PW_RIGHT 3500
#define PW_LEFT 2000
//---------------------------------------------------------------------------------
// Init Function Prototypes
//---------------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void SMB_Init(void);
void XBR0_Init(void);
void Drive_Init(void);
void ADC_Init(void);
//---------------------------------------------------------------------------------
// Ranger/Drive Motor Function Prototypes
//---------------------------------------------------------------------------------
void Range_Update(void);
void Drive_Motor(unsigned int);
unsigned int ranger_pd(void);
//---------------------------------------------------------------------------------
// Compass/Steering Function Prototypes
//---------------------------------------------------------------------------------
void Compass_Update(void);
void Steering_Control(void);
void ReadCompass(void);
//---------------------------------------------------------------------------------
// Voltage Control Function Prototypes
//---------------------------------------------------------------------------------
unsigned char read_AD_input(unsigned char);
void voltage_update(void);
//---------------------------------------------------------------------------------
// General Function Prototypes
//---------------------------------------------------------------------------------
void PCA_ISR (void) interrupt 9;
int get_input(void);
void set_variable(void);
//---------------------------------------------------------------------------------
// Ranger/Drive Motor Variables
//---------------------------------------------------------------------------------
unsigned int MOTOR_PW = 0;
unsigned char r_count = 0;
bit new_range = 0;
unsigned int cmrange = 0;
unsigned char neutral_range = 50;
unsigned int ranger_kp = 1;
unsigned int ranger_kd = 1;
long int ranger_error = 0;
long int curr_err = 0;
//---------------------------------------------------------------------------------
// Compass/Steering Variables
//---------------------------------------------------------------------------------
unsigned int SERVO_PW = 0;
bit new_heading = 0;
unsigned char h_count = 0;
int desired_heading = 0;
int current_heading = 0;
int compass_calibration = 0;
long steering_error;
long steering_pre_error;
int steering_kp = 1;
int steering_kd = 1;
//---------------------------------------------------------------------------------
// General Variables
//---------------------------------------------------------------------------------
unsigned char printcount = 0;
unsigned char Data[5];
char adinput = 0;
int number;
//---------------------------------------------------------------------------------
// sbit Declaration
//---------------------------------------------------------------------------------
sbit at 0xB0 Steering_Switch;
sbit at 0xB1 Drive_Switch;
sbit at 0xB2 Enable_LCD;
sbit at 0xB3 Enable_HT;
sbit at 0xB4 Heading_Height;
sbit at 0xB5 Steering_GD;
sbit at 0xB6 Drive_GD;
sbit at 0xB7 Drive_Angle;
//---------------------------------------------------------------------------------
// Main Function
//---------------------------------------------------------------------------------
void main(void){
	Sys_Init();
	putchar(' ');
	XBR0_Init();
	Port_Init();
	SMB_Init();
	PCA_Init();
	ADC_Init();
	Drive_Init();
	printf("hello");
	while(1){
		set_variable();
		//voltage_update();
		if (Enable_LCD && (printcount > 8)){
			lcd_clear();
			lcd_print("Heading: %d\n", current_heading);
			lcd_print("Range:   %d\n", cmrange);
			lcd_print("Voltage: %d\n", adinput);
			printcount = 0;
		}
		if (Enable_HT && (printcount > 8)){
			printf("%d		%d		%d\r\n", current_heading,current_heading, adinput);
			printcount = 0;
			}
		if(Drive_Switch){
			  Range_Update(); //update the range
			  Drive_Motor(ranger_pd()); //
		}
		else
			Drive_Motor(PW_NEUT); //if ss is not flipped, put it in neutral
		if(Steering_Switch){
			Compass_Update();
			Steering_Control();
		}
  		else {
			SERVO_PW = PW_CENTER;
			PCA0CPL0 = (0xFFFF - SERVO_PW);
			PCA0CPH0 = (0xFFFF - SERVO_PW) >> 8;
   		}
	}
}
//---------------------------------------------------------------------------------
// Init Functions
//---------------------------------------------------------------------------------
void ADC_Init(void){
    REF0CN = 0x03;                     //Set Vref to use internal reference voltage (2.4 V)
    ADC1CN = 0x80;                     //Enable A/D converter (ADC1)
    ADC1CF |= 0x01;                    // Set A/D converter gain to 1
}

void SMB_Init(void){
	SMB0CR=0x93;
	ENSMB=1;
}

void Port_Init(){
	P0MDOUT = 0x0F; //set output pin for CEX0 in push-pull mode
	P3MDOUT &= ~0xFF; //set Port 3, pin 6 and P3.7 to open-drain mode (input)
	P3 |= 0xFF; //Write a logic high to P3.6 and P3.7
	P1MDIN &= ~0x20; //Set Port 1, Pin 5 to analog input
	P1MDOUT &= ~0x20; //Set Port 1, Pin 5 to open drain mode (input)
	P1 |= 0x20; //Set Port 1, Pin 5 to logic high
}

void XBR0_Init(){
	XBR0 = 0x25;	//configure crossbar with UART, SPI, SMBus, and CEX channels 
}

void PCA_Init(void){
         PCA0MD = 0x81;            	// SYSCLK/12, enable CF interrupts, suspend when idle
         PCA0CPM0 = 0xC2;			// 16 bit, enable compare, enable PWM
		 PCA0CPM1 = 0xC2;
		 PCA0CPM2 = 0xC2;			// 16 bit, enable compare, enable PWM
		 PCA0CPM3 = 0xC2;			// 16 bit, enable compare, enable PWM
         EIE1 |= 0x08;            	// enable PCA interrupts
         PCA0CN |= 0x40;          	// enable PCA
         EA = 1;                  	// enable all interrupts
}

void Drive_Init(void){
	//set initial value
	MOTOR_PW = PW_NEUT;
	PCA0CPL0 = 0xFFFF - MOTOR_PW; //set low byte of left fan CCM PW register
	PCA0CPH0 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL1 = 0xFFFF - MOTOR_PW; //set low byte of left fan CCM PW register
	PCA0CPH1 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL2 = 0xFFFF - MOTOR_PW; //set low byte of left fan CCM PW register
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL3 = 0xFFFF - MOTOR_PW; //set low byte of right fan CCM PW register
	PCA0CPH3 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	lcd_print("setting neutral\n");
	delay_time(10000000); //make sure the motor sits in neutral for a second
	lcd_clear();
	lcd_print("finished setting neutral\n");
	delay_time(1000000);
	lcd_clear();
}
//---------------------------------------------------------------------------------
// Ranger Functions
//---------------------------------------------------------------------------------
unsigned int ranger_pd(void){
	long int tmp_pw;
	curr_err = neutral_range - cmrange;
	tmp_pw = (long)ranger_kp*curr_err + (long)ranger_kd*(curr_err - ranger_error);
	if (tmp_pw > (long)PW_MAX)
		tmp_pw = PW_MAX;
	if (tmp_pw < (long)PW_MIN)
		tmp_pw = PW_MIN;
	ranger_error = curr_err;
	return (unsigned int)tmp_pw;
}

void Range_Update(void){
	if (new_range){
		new_range = 0; //reset the new_range flag
		i2c_read_data(ranger_addr, 2, Data, 2);  // read two bytes, starting at reg 2
		cmrange = (((unsigned int)Data[0] << 8) | Data[1]); //concatenate the two bytes.
		Data[0] = 0x51;
		i2c_write_data(ranger_addr,0,Data,1) ;  // write one byte of data to reg 0 at addr
	}
}
		
void Drive_Motor(unsigned int motorval){	
	MOTOR_PW = motorval; //set the motor_pw to whatever value was passed in
	PCA0CPL2 = 0xFFFF - MOTOR_PW; //set low byte
	PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
	PCA0CPL3 = 0xFFFF - MOTOR_PW; //set low byte
	PCA0CPH3 = (0xFFFF - MOTOR_PW) >> 8; //set high byte
}
//---------------------------------------------------------------------------------
// Steering Functions
//---------------------------------------------------------------------------------
void ReadCompass(void){ 	
	i2c_read_data(comp_addr,2,Data,2);
	current_heading =(((unsigned int)Data[0] << 8) | Data[1]); 	//combine the two values
}

void Compass_Update(void){
	if (new_heading){ // enough overflows for a new heading
		ReadCompass();
		if ((current_heading + compass_calibration) <= 0){
				current_heading = 3600 + (current_heading + compass_calibration);
				}
		else if((current_heading + compass_calibration) >= 3600){
				current_heading = 3600-(current_heading + compass_calibration);
				}
		else{					
			current_heading = (current_heading + compass_calibration);
		}
		new_heading = 0;
		}
}

void Steering_Control(void){
	long steering_temp;
	if(desired_heading > 1800){
	      desired_heading = desired_heading - 3600;
		  }

    steering_error = (current_heading-desired_heading);
	
	if(steering_error < 1800){
		steering_error = 0-steering_error;
		}
	else {
		steering_error = 3600-steering_error;
		}
	
	steering_temp = (long)steering_kp*steering_error + (long)steering_kd*(steering_error - steering_pre_error) + 2750;
	steering_pre_error = steering_error;
	SERVO_PW = steering_temp;
	

	if (SERVO_PW > PW_RIGHT)
		SERVO_PW = PW_RIGHT;
	else if (SERVO_PW < PW_LEFT)
		SERVO_PW = PW_LEFT;

			
	PCA0CPL0 = (0xFFFF - SERVO_PW);
	PCA0CPH0 = (0xFFFF - SERVO_PW) >> 8;
}

//---------------------------------------------------------------------------------
// Voltage Functions
//---------------------------------------------------------------------------------
unsigned char read_AD_input(unsigned char n){ //note: on gondola; voltage divider on Pin 1.5
	AMX1SL = n;                        /* Set P1.n as the analog input for ADC1 */
    ADC1CN = ADC1CN & ~0x20;           /* Clear the “Conversion Completed” flag */
    ADC1CN = ADC1CN | 0x10;            /* Initiate A/D conversion */
    while ((ADC1CN & 0x20) == 0x00);   /* Wait for conversion to complete */
    return ADC1;                       /* Return digital value in ADC1 register */
}

void voltage_update(void){
	float advolt;
	adinput = read_AD_input(5); //read the voltage on pin 1.5 and convert it to an unsigned char
	advolt = adinput;
	advolt = advolt/.236; //do some math, get a float out between 0-10(V)
	if (advolt < MIN_VOLT){
		while(1){
			Drive_Motor(PW_MIN);
			lcd_print("LOW VOLTAGE\n");
		}
	}
	adinput = (char)advolt;
}
//---------------------------------------------------------------------------------
// General Functions
//---------------------------------------------------------------------------------
void PCA_ISR (void) interrupt 9 {
         if (CF){
                 PCA0L = 0xFF & 28762;     // low byte of start count
                 PCA0H = 28762>>8;     // high byte of start count
                 CF = 0;           // Very important - clear interrupt flag
         }
         else PCA0CN &= 0xC0; // all other type 9 interrupts
		 
		 h_count++;
		 if(h_count>=2){
			new_heading=1; // 2 overflows is about 40 ms
			h_count = 0;
		 }
		 r_count++;
	     if(r_count>=4){
			new_range=1;  // 4 overflows is about 80 ms
			r_count = 0;
		 }
		 printcount++;	
}

int get_input(void){	
	char tempval = 0;
	char x;
	bit correct = 0;
	bit pause = 0;
	number = 0;
	while(correct == 0){
		Data[0] = 0;
		Data[1] = 0;
		Data[2] = 0;
		Data[3] = 0;
		Data[4] = 0;
		pause = 0;
		number = 0;
		lcd_print("Enter a <=4-digit number, confirm with *\n");
		for(x = 0;x<5;x++){
			tempval = read_keypad();
			tempval = -1;
			while(tempval == -1){
				tempval = read_keypad();
			}
			Data[x] = tempval;
			lcd_print("%c",Data[x]);
			if(tempval == '*'){
				break;
			}
			delay_time(100);
			while(tempval != -1){
			tempval = read_keypad();
			}
		}
		if (Data[1] == ('*'))
			number = (Data[0]-48);
		if (Data[2] == ('*'))
			number = ((Data[1]-48)+(Data[0]-48)*10);
		if (Data[3] == ('*'))
			number = ((Data[2]-48)+(Data[1]-48)*10+(Data[0]-48)*100);
		if (Data[4] == ('*'))
			number = ((Data[3]-48) + (Data[2]-48)*10+(Data[1]-48)*100+(Data[0]-48)*1000);
		
		lcd_clear();
		lcd_print("You entered: %d\n", number);
		delay_time(1000000);
		correct = 1;
		lcd_clear();
		
	}
	
	return number;
}

void set_variable(void){
	if (Heading_Height){
		lcd_clear();
		lcd_print("Enter desired heading\n");
		get_input();
		desired_heading = number;
		printf("hey");
		lcd_clear();
		lcd_print("Enter desired height\n");
		get_input();
		neutral_range = number;
	}
	if (Steering_GD){
		lcd_clear();
		lcd_print("Enter steering kp\n");
		get_input();
		steering_kp = number;
		lcd_clear();
		lcd_print("Enter steering kd\n");
		get_input();
		steering_kd = number;
	}
	if (Drive_GD){
		lcd_clear();
		lcd_print("Enter height kp\n");
		get_input();
		ranger_kp = number;
		lcd_clear();
		lcd_print("Enter height kd\n");
		get_input();
		ranger_kd = number;
	}
	if (Drive_Angle){
		int motor_angle = 2750;
		bit cor =0;
		lcd_clear();
		lcd_print("Use 1 and 3 to rotate drive motors\n");
		lcd_print("Press * to confirm\n");
		while (cor ==0){
			if(read_keypad() == '1')
				motor_angle = motor_angle - 1;
			if(read_keypad() == '3')
				motor_angle = motor_angle + 1;
			if(read_keypad() == '*')
				cor = 1;
			if(motor_angle < 2000)
				motor_angle = 2000;
			else if(motor_angle > 3500)
				motor_angle = 3500;
			PCA0CPL1 = 0xFFFF - motor_angle; //set low byte of left fan CCM PW register
			PCA0CPH1 = (0xFFFF - motor_angle) >> 8; //set high byte
		}
	}

}