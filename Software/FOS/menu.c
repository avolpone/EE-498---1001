#define F_CPU	8000000		// 8MHz
#define BAUD	38400			// 9600 baud rate
#define VCC		5.05

#define V_CALIBRATION -0.11

#define ADC_NUM 6

#define ZERO	0
#define ONE		1
#define TWO		2

#define TRUE	1
#define FALSE	0

#define ON		1
#define OFF		0

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "ioe.h"
#include "menu.h"

void delay_ms(int n) {		
	 while(n--) {
		 _delay_ms(1);
	 }
 }

void delay_us(int n) {
	  while(n--) {
		  _delay_us(1);
	  }
}

//extern 
extern unsigned int op_mode;

void header()
{
	br();
	printm("=============================================");
	br();
	printm("EE498 - 1001 | Fiber Optic Sensor for CubeSat");
	br();
	printm("=============================================");
	br();
	printm("by: Jiayi Ren, Aaron Volpone");
	br(); br();
}

unsigned char mainMenu()	
{
	unsigned char mm;	

	br();

	printm("PHOTODIODE ARRAY: ");							 br();
	printm("               ( 4 )   ( 5 )                 "); br();
	printm("                                             "); br();
	printm("           ( 3 )   ( * )   ( 0 )             "); br();
	printm("                                             "); br();
	printm("               ( 2 )   ( 1 )                 "); br(); 
	
	br();

	printm("OPERATING MODE: ");
	
	br();
	if(op_mode == 0) printm("ENABLED --> ");
	else		  printm("            ");
	printm("[0] Overview mode");

	br();
	if(op_mode == 1) printm("ENABLED --> ");
	else		  printm("            ");
	printm("[1] Testbench mode");

	br();
	if(op_mode == 2) printm("ENABLED --> ");
	else		  printm("            ");
	printm("[2] ADC debug mode");

	br(); br(); 
	printm("- - - - - - - - - - - - - - - - - - - - - - -");

	br();
	printm("Options:");

	br();
	printm("[a] ADC configuration options");

	br();
	printm("[d] Diagnostic options");

	br();
	printm("[p] Power options");

	br();
	printm("[r] Software reset");

	br(); br();
	printm("[x] Exit to operation mode screen. . .");

	br();
	br();
	printm("> ");

	mm = UART_getChar();	

	printc(mm);

	return mm;
}

extern unsigned int ADC_en[ADC_NUM];

extern float		AD_global_offset;
extern unsigned int AD_global_offset_sign;
extern float AD_offset[ADC_NUM];


void calibrationMenu()
{
	unsigned char selection;
	unsigned int leave;
	unsigned char *global_offset = (unsigned char *) malloc(sizeof(char)*8);
	unsigned char *channel_offset = (unsigned char *) malloc(sizeof(char)*8);
	int ADC_offset_voltage;
	unsigned char ADC_pick;
	int ADC_pick_int;
	
	br();
	printm("\n\rSelect a global offset option: ");
	
	br();
	printm("[1] Global ADC voltage offset");
	
	br();
	printm("[2] ADC channel offset offset");

	br();
	printm("[3] Enable/disable ADC channels");
	
	br();
	printm("[b] Back to main menu");
	
	br();
	br();
	printm("> ");
	
	do
	{
		selection = UART_getChar();
	
		switch (selection){
			case '1':
			printc(selection);
			br();
			br();
			printm("Enter a global voltage offset: (in mV)");
			br();
			printm("> ");

			scanm(global_offset);
			float global_offset_float = atof(global_offset);

			global_offset_float /= 1000;

			AD_global_offset = global_offset_float;
			
			leave = 1;
			br();
			screenClr();
			header();
			break;

			case '2':
			printc(selection);
			br();
			br();
			printm("Enter a channel to change: (0 to 5)");
			br();
			printm("> ");

			ADC_pick = UART_getChar();
			printc(ADC_pick);
			
			ADC_pick_int = ADC_pick - '0';

			br(); br();
			printm("Enter the channel voltage offset: (in mV)");
			br();
			printm("> ");

			scanm(channel_offset);
			float ADC_offset_voltage_float = atof(channel_offset);

			ADC_offset_voltage_float /= 1000;

			AD_offset[ADC_pick_int] = ADC_offset_voltage_float;

			leave = 1;
			br();
			screenClr();
			header();
			break;

			case '3':
			printc(selection);
			br();
			br();
			printm("Enter a channel to toggle ON or OFF: (0 to 5)");
			br();
			printm("> ");

			ADC_pick = UART_getChar();
			printc(ADC_pick);
			
			ADC_pick_int = ADC_pick - '0';

			if(ADC_en[ADC_pick_int] == ON)
				ADC_en[ADC_pick_int] = OFF;
			else if(ADC_en[ADC_pick_int] == OFF)
				ADC_en[ADC_pick_int] = ON;

			leave = 1;

			br();
			screenClr();
			header();
			break;

			case 'b':
			_delay_ms(1);
			printc(selection);
			leave = 1;
			br();
			screenClr();
			header();
			_delay_ms(1);
			break;
			default:
			leave = 0;
			_delay_ms(1);
			break;
		}
	} while(!leave);
}

void diagnosticMenu()
{
	unsigned char selection;
	unsigned int leave;
	unsigned char *global_offset = (unsigned char *) malloc(sizeof(char)*8);
	
	br();
	printm("\n\rSelect a global offset option: ");
	
	br();
	printm("[1] Global ADC voltage offset");
	
	br();
	printm("[2] ADC channel offset offset");
	
	br();
	printm("[b] Back to main menu");
	
	br();
	br();
	printm("> ");
	
	do
	{
		selection = UART_getChar();
	
		switch (selection){
			case '1':
			printc(selection);
				br();
				br();
				printm("Enter a global voltage offset: (in mV)");
				br();
				printm("> ");

				
				scanm(global_offset);
				float global_offset_float = atof(global_offset);

				global_offset_float /= 1000;

				AD_global_offset = global_offset_float;
			
			/*
			printm("\n\rPress hardware button to wake");
			_delay_ms(10);
			set_sleep_mode(SLEEP_MODE_PWR_SAVE);
			sleep_mode();
			_delay_ms(1);
			*/
			leave = 1;
			break;
			case '2':
			printc(selection);
			printm("\n\rPress hardware button to wake");
			_delay_ms(10);
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_mode();
			_delay_ms(1);
			break;
			case 'b':
			_delay_ms(1);
			printc(selection);
			leave = 1;
			br();
			_delay_ms(1);
			break;
			default:
			leave = 0;
			_delay_ms(1);
			break;
		}
	} while(!leave);
}

void powerModes()	
{
	unsigned char selection;	
	unsigned int leave;

	printc('\n');
	printc('\r');
	printm("\n\rSelect a sleep option: ");	
	
	printc('\n');
	printc('\r');
	printm("[1] Sleep");

	printc('\n');
	printc('\r');
	printm("[2] Power down");

	printc('\n');
	printc('\r');
	printm("[b] Back to main menu");

	printc('\n');
	printc('\r');
	printc('\n');
	printc('\r');
	printm("> ");

	do
	{
		selection = UART_getChar();

		switch (selection){
			case '1':
			printc(selection);
			printm("\n\rPress hardware button to wake");
			_delay_ms(10);
			set_sleep_mode(SLEEP_MODE_PWR_SAVE);	
			sleep_mode();	
			_delay_ms(1);
			break;
			case '2':
			printc(selection);
			printm("\n\rPress hardware button to wake");
			_delay_ms(10);
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);	
			sleep_mode();
			_delay_ms(1);
			break;
			case 'b':
			_delay_ms(1);
			printc(selection);
			leave = 1;
			_delay_ms(1);
			break;
			default:
			leave = 0;
			_delay_ms(1);
			break;
		}
	} while(!leave);
}

/*

void ledModes(unsigned int mode)
{
	unsigned char selection;		

	printc('\n');
	printc('\r');
	printm("\n\rSelect an LED option: ");	
	
	printc('\n');
	printc('\r');
	printm("[1] Toggle LED");

	printc('\n');
	printc('\r');
	printm("[2] Blinking options");

	printc('\n');
	printc('\r');
	printm("[3] PWM options");

	printc('\n');
	printc('\r');
	printm("[b] Back to main menu");

	printc('\n');
	printc('\r');
	printc('\n');
	printc('\r');
	printm("> ");
	
	selection = UART_getChar();		
	switch (selection){
		case '1':
			printc(selection);

			PORTB ^= (1 << PB0);
			OCR1A ^= (255 * 100) / 100;
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED toggled");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			_delay_ms(2000);
			break;
		case '2':
			ledBlinking();
			break;
		case '3':
			ledPWM();
			break;
		case 'b':
			_delay_ms(1);
			printc(selection);
			_delay_ms(1);
			mainMenu(mode);	
			break;
		default:
			_delay_ms(1);
			break;
	}
}

void ledBlinking()
{
	unsigned int leave;
	unsigned char selection;
	unsigned char *onTime = (unsigned char *) malloc(sizeof(char)*8);		
	unsigned char *offTime = (unsigned char *) malloc(sizeof(char)*8);		

	printc('\n');
	printc('\r');
	printm("\n\rSelect a blinking option: ");	
	
	printc('\n');
	printc('\r');
	printm("[1] Keep LED on");

	printc('\n');
	printc('\r');
	printm("[2] Blink once");

	printc('\n');
	printc('\r');
	printm("[3] Periodic blinking");

	printc('\n');
	printc('\r');
	printm("[b] Back to main menu");

	printc('\n');
	printc('\r');
	printc('\n');
	printc('\r');
	printm("> ");

	do
	{
		selection = UART_getChar();

		switch (selection){
			case '1':
			printc(selection);

			PORTB = (1 << PB0);
			OCR1A = (255 * 100) / 100;
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED turned on");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			_delay_ms(2000);

			ledModes();
			break;
			case '2':
			printc(selection);

			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED blinking once");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			PORTB = (0 << PB0);
			_delay_ms(500);
			PORTB = (1 << PB0);
			OCR1A = (255 * 100) / 100;
			_delay_ms(500);
			PORTB = (0 << PB0);
			OCR1A = (255 * 0) / 100;

			_delay_ms(1000);

			ledModes();
			break;
			case '3':
			PORTB = (0 << PB0);
			OCR1A = (255 * 0) / 100;

			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("Enter time between turning LED on and off (in ms): ");
			printc('\n');
			printc('\r');
			printm("[ms] > ");
			scanm(offTime);

			printc('\n');
			printc('\r');			
			printm("Enter time between to keep LED on per cycle (in ms): ");
			printc('\n');
			printc('\r');
			printm("[s] >");
			scanm(onTime);

				while (!(UCSR0A & (1<<RXC0)))
				{
					PORTB = (1<<PB0);	//LED on
					OCR1A = (255 * 100) / 100;
					delay_ms(atoi((const char*)onTime));		
					PORTB = (0<<PB0);	//LED off
					OCR1A = (255 * 0) / 100;
					delay_ms(atoi((const char*)offTime));		
				}

			free(onTime);	
			free(offTime);	
			
			ledModes();
			leave = 1;
			break;
			case 'b':
			_delay_ms(1);
			printc(selection);
			leave = 1;
			_delay_ms(1);
			break;
			default:
			leave = 0;
			_delay_ms(1);
			break;
		}
	} while(!leave);
}

void ledPWM()
{
	unsigned int leave;
	unsigned char selection;
	unsigned char *percent = (unsigned char *) malloc(sizeof(char)*8);

	printc('\n');
	printc('\r');
	printm("\n\rSelect a PWM option: ");
	
	printc('\n');
	printc('\r');
	printm("[1] 10% brightness");

	printc('\n');
	printc('\r');
	printm("[2] 50% brightness");

	printc('\n');
	printc('\r');
	printm("[3] 90% brightness");

	printc('\n');
	printc('\r');
	printm("[4] Custom brightness");

	printc('\n');
	printc('\r');
	printm("[b] Back to main menu");

	printc('\n');
	printc('\r');
	printc('\n');
	printc('\r');
	printm("> ");

	do
	{
		selection = UART_getChar();

		switch (selection){
			case '1':
			printc(selection);

			OCR1A = (255 * 10) / 100;
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED at 10% brightness");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			_delay_ms(2000);

			ledModes();
			break;
			case '2':
			printc(selection);

			OCR1A = (255 * 50) / 100;
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED at 50% brightness");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			_delay_ms(2000);

			ledModes();

			ledModes();
			break;
			case '3':
			printc(selection);

			OCR1A = (255 * 90) / 100;
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("\tLED at 90% brightness");
			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');

			_delay_ms(2000);
			
			ledModes();
			leave = 1;
			break;
			case '4':
			PORTB = (0 << PB0);
			OCR1A = (255 * 0) / 100;

			printc('\n');
			printc('\r');
			printc('\n');
			printc('\r');
			printm("Enter a custom percentage (from 0 to 100): ");
			printc('\n');
			printc('\r');
			printm("[%] > ");
			scanm(percent);

			OCR1A = (atoi((const char*)percent) * 255) / 100;

			ledModes();
			leave = 1;
			break;
			case 'b':
			_delay_ms(1);
			printc(selection);
			leave = 1;
			_delay_ms(1);
			break;
			default:
			leave = 0;
			_delay_ms(1);
			break;
		}
	} while(!leave);
}

*/