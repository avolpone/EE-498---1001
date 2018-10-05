/*
* FOS.c
*
* Created: 1/29/2018 1:03:00 PM
* Author : Aaron Volpone
*/ 

// ========================================
// PARAMETER DEFINITIONS

// ----------------------------------------
// MODIFIABLE DEFINITIONS 
// - - - - -

// VOLTAGE ADJUSTMENTS
#define V_ADJUST	-0.11
#define ADC_ADJUST	-0.00

// ANGLE CALIBRATIONS
#define VOLT_PER_DEGREE		0.050
#define DEGREE_RANGE		1
// - - - - -

// ----------------------------------------
// STATIC DEFINITIONS
// - - - - -
// PHYSICAL
#define F_CPU	8000000		
//#define BAUD	38400		
#define BAUD	76800
#define VCC		5.05
#define ADC_NUM 6

// LOGIC
#define ZERO	0
#define ONE		1
#define TWO		2

#define TRUE	1
#define FALSE	0

#define ON		1
#define OFF		0

#define NORTH		10
#define NORTHEAST	11
#define EAST		20
#define SOUTHEAST	22
#define SOUTH		30
#define SOUTHWEST	33
#define WEST		40
#define NORTHWEST	44

#define SIGNAL_NA	0
#define SIGNAL_L	1
#define SIGNAL_M	2
#define SIGNAL_H	3
// - - - - -
// END ====================================

// ========================================
// LIBRARY DECLARATIONS
#include <stdlib.h>		
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "menu.h"
#include "ioe.h"
// END ====================================


// ========================================
// GLOBAL VARIABLES

// ----------------------------------------
// ADC VARIABLES
// - - - - -
unsigned int ADC_en[6] = {ON, ON, ON, ON, ON, ON};
float	  	 AD_global_offset = 0.0;
unsigned int AD_global_offset_sign;
float		 AD_offset[ADC_NUM] = {-3.63, 0, -3.68, -3.67, -3.67, 0};
unsigned int direction = 00;
unsigned int intensity = 0;
//- - - - -

// ----------------------------------------
// MISC VARIABLES
// - - - - -
unsigned int			op_mode = 0;
volatile unsigned char command;
unsigned int			menu = 0;
unsigned int			terminate = 0;

float	voltage[5];
int		ADC_en_count = 0;

float	voltage_deg;
char	stringValue[20];

float	voltageMin = 5.55;
int		channelMin;
char	voltageStrMin[20];
char	channelStrMin[8];

float	voltageAvg = 0;
char	voltageStrAvg[20];

float	voltageMax = 0;
int		channelMax;
char	voltageStrMax[20];
char	channelStrMax[8];
char	voltageStr[5][20];
char	channelStr[5][8];

int pollingSpeed = 1000;
// - - - - -
// END ====================================

// ========================================
// FUNCTION DECLARATIONS
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void ADC_init();
float ADC_conv(unsigned int, unsigned int);
void internal_Volt();
void fiberDisplay(float[5], float, int, float, float, int);
// END ====================================

// ========================================
// MAIN DRIVER
int main( void )
{

	// clock prescaler 256 --> 8MHz / 256 = 31250 = 31.25 KHz
	CLKPR = (1 << CLKPCE) | (1 << CLKPS3); 

	cli();					// disable global interrupts

	_delay_ms(700);			

	screenClr();			// clear terminal screen

	br(); br();				

	UART_init(ONE, TRUE);	// initialize UART 

	_delay_ms(700);			

	br(); br();

	ADC_init();				// initialize ADC

	_delay_ms(500);	

	header();				// UNLV project header

	_delay_ms(200);

	// start message before main screen
	printm("Starting");
	_delay_ms(60);
	printc('.');
	_delay_ms(80);
	printc('.');
	_delay_ms(100);
	printc('.');

	_delay_ms(500);

	// ----------------------------------------
	// LOOP FOREVER
	while(1) 
	{

		// ----------------------------------------
		// DEFAUlT INFO TABLE
		// - - - - -
		if(op_mode == 0)
		{
			screenClr();	// clear terminal screen
			
			header();		// UNLV project header

			voltage[0] = ADC_conv(0, FALSE);			// ADC0 calc, return voltage - don't print
			dtostrf(voltage[0], 4, 3, voltageStr[0]);	// float to char conversion

			voltage[1] = ADC_conv(1, FALSE);			// ADC1 calc, return voltage - don't print
			dtostrf(voltage[1], 4, 3, voltageStr[1]);	// float to char conversion

			voltage[2] = ADC_conv(2, FALSE);			// ADC2 calc, return voltage - don't print
			dtostrf(voltage[2], 4, 3, voltageStr[2]);	// float to char conversion

			voltage[3] = ADC_conv(3, FALSE);			// ADC3 calc, return voltage - don't print
			dtostrf(voltage[3], 4, 3, voltageStr[3]);	// float to char conversion

			voltage[4] = ADC_conv(4, FALSE);			// ADC4 calc, return voltage - don't print
			dtostrf(voltage[4], 4, 3, voltageStr[4]);	// float to char conversion

			voltage[5] = ADC_conv(5, FALSE);			// ADC5 calc, return voltage - don't print
			dtostrf(voltage[5], 4, 3, voltageStr[5]);	// float to char conversion

			printm("---------------------------------------------");

			br(); br();

			// - - - - -
			// FIBER CORE DIAGRAM 
			// - - - - -
			fiberDisplay(voltage, voltageMax, 
						 channelMax, voltageAvg,
					     voltageMin, channelMin);

			// - - - - -
			// RESET ADC VARIABLES
			// - - - - -
			voltageMax	 = 0;
			channelMax	 = 0;

			ADC_en_count = 0;
			voltageAvg 	 = 0;

			voltageMin	 = 5.55;
			channelMin	 = 0;

			br(); br();

			printm("ADC - - - - - - - - - - - - - - - - - - - - -");
			br();

			// ----------------------------------------
			// PRINT ADC HEADERS w/ VOLTAGE
			// - - - - -
			printm("A0: "); 
			if(voltage[0] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[0]); printm("V   "); }	// otherwise, display ADC voltage

			printm("A1: ");
			if(voltage[1] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[1]); printm("V   "); }	// otherwise, display ADC voltage

			printm("A2: ");
			if(voltage[2] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[2]); printm("V   "); }	// otherwise, display ADC voltage

			br();
			printm("A3: ");
			if(voltage[3] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[3]); printm("V   "); }	// otherwise, display ADC voltage

			printm("A4: ");
			if(voltage[4] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[4]); printm("V   "); }	// otherwise, display ADC voltage

			printm("A5: ");
			if(voltage[5] == -1.0)
				printm("OFF      ");						// print OFF if ADC disabled
			else
				{ printm(voltageStr[5]); printm("V   "); }	// otherwise, display ADC voltage
			
			br();

			// calculate the voltage avg
			for(int i = 0; i <= 5; i++)
			{
				voltageAvg += voltage[i];
				
				// find voltage max
				if(voltage[i] > voltageMax)
				{
					voltageMax = voltage[i];
					channelMax = i;
				}

				// find voltage min
				if(voltage[i] < voltageMin)
				{
					voltageMin = voltage[i];
					channelMin = i;
				}
			}

			br();
			printm("A_MAX_CHAN: ");

			
			dtostrf(voltageMax, 4, 3, voltageStrMax);	// float to char conversion
			itoa(channelMax, channelStrMax, 10);		// integer to char conversion
			printm(channelStrMax);						// print channel number for max voltage

			printm("     A_MAX_VOLT: ");

			printm(voltageStrMax);						// print max voltage found

			printm("V");

			// ------

			for(int j = 0; j <= 5; j++)
			{
				if(ADC_en[j] == ON)
					ADC_en_count++;
			}
				
			voltageAvg /= 6;
			//voltageAvg /= ADC_en_count;

			if(voltageAvg == -1.0)
				voltageAvg = 0.0;

			br();
			printm("                  A_AVG_VOLT: ");

			dtostrf(voltageAvg, 4, 3, voltageStrAvg);
			//itoa(voltageAvg, voltageStrAvg, 10);
			printm(voltageStrAvg);

			printm("V");

			// ------

			br();
			printm("A_MIN_CHAN: ");

			if(voltageMin == -1.0)
				voltageMin = 0.0;

			dtostrf(voltageMin, 4, 3, voltageStrMin);
			itoa(channelMin, channelStrMin, 10);
			printm(channelStrMin);

			printm("     A_MIN_VOLT: ");

			printm(voltageStrMin);

			printm("V");

			


			br(); br();
			printm("DIAG  - - - - - - - - - - - - - - - - - - - -");
			br();

			printm("INTERNAL_V: ");
			internal_Volt();
			
			delay_ms(pollingSpeed);
			
		}

		else if(op_mode == 1)
		{

			_delay_ms(2);

			voltage[0] = ADC_conv(0, FALSE);			// ADC0 calc, return voltage - don't print
			dtostrf(voltage[0], 4, 3, voltageStr[0]);	// float to char conversion

			voltage[1] = ADC_conv(1, FALSE);			// ADC1 calc, return voltage - don't print
			dtostrf(voltage[1], 4, 3, voltageStr[1]);	// float to char conversion

			voltage[2] = ADC_conv(2, FALSE);			// ADC2 calc, return voltage - don't print
			dtostrf(voltage[2], 4, 3, voltageStr[2]);	// float to char conversion

			voltage[3] = ADC_conv(3, FALSE);			// ADC3 calc, return voltage - don't print
			dtostrf(voltage[3], 4, 3, voltageStr[3]);	// float to char conversion

			voltage[4] = ADC_conv(4, FALSE);			// ADC4 calc, return voltage - don't print
			dtostrf(voltage[4], 4, 3, voltageStr[4]);	// float to char conversion

			voltage[5] = ADC_conv(5, FALSE);			// ADC5 calc, return voltage - don't print
			dtostrf(voltage[5], 4, 3, voltageStr[5]);	// float to char conversion

			voltage_deg = (voltageMax / VOLT_PER_DEGREE);

			if(voltage_deg >= 60.0)
			voltage_deg = 60;
			
			dtostrf(voltage_deg, 2, 1, stringValue);
			printm(stringValue);				// print the voltage
			printc(176);
			printm(" at max voltage");				// suffix

			if(voltageMax <= 0.2)
			intensity = SIGNAL_NA;
			else if(voltageMax > 0.200 && voltageMax <= 0.5)
			intensity = SIGNAL_L;
			else if(voltageMax > 0.5 && voltageMax <= 2.0)
			intensity = SIGNAL_M;
			else if(voltageMax > 2.0)
			intensity = SIGNAL_H;

			if(channelMax == 0)
			direction = EAST;
			else if(channelMax == 1)
			direction = SOUTHEAST;
			else if(channelMax == 2)
			direction = SOUTHWEST;
			else if(channelMax == 3)
			direction = WEST;
			else if(channelMax == 4)
			direction = NORTHWEST;
			else if(channelMax == 5)
			direction = NORTHEAST;
		}
		
		else if(op_mode == 2)
		{
			printm(" ");
			ADC_conv(0, TRUE); printm(" | ");
			ADC_conv(1, TRUE); printm(" | ");
			ADC_conv(2, TRUE); printm(" | ");
			ADC_conv(3, TRUE); printm(" | ");
			ADC_conv(4, TRUE); printm(" | ");
			ADC_conv(5, TRUE);
			
			br();
			
			_delay_ms(1);
		}

		sei();

		_delay_ms(20);

		cli();
		
		
	}

}
// END ====================================

// ========================================
// INTERRUPT VECTOR
ISR(USART_RX_vect)
{

	cli();

	screenClr();
	header();
	
	if(menu == FALSE)
	{
		_delay_ms(1);
		menu = TRUE;
		terminate = TRUE;
		_delay_ms(1);
	}

	else if(menu == TRUE)
	{
		UART_getChar();

		do
		{
			command = mainMenu();
			
			switch (command)
			{
				case '0':
				op_mode = 0;
				screenClr();
				header();
				menu	  = FALSE;
				terminate = TRUE;
				break;

				case '1':
				op_mode = 1;
				screenClr();
				header();
				menu	  = FALSE;
				terminate = TRUE;
				break;

				case '2':
				op_mode = 2;
				screenClr();
				header();
				menu	  = FALSE;
				terminate = TRUE;
				break;

				case 'a':
				calibrationMenu();
				break;
				
				case 'd':
				diagnosticMenu();
				break;

				case 's':
				powerModes();
				break;

				case 'l':
				//ledModes();
				case 'r':
				wdt_init();
				case 'x':
				screenClr();
				header();
				menu	  = FALSE;
				terminate = TRUE;
				break;

				case '-':
				pollingSpeed -= 100;

				if(pollingSpeed == 0)
					pollingSpeed = 100;

				screenClr();
				header();
				break;

				case '=':
				pollingSpeed += 100;

				if(pollingSpeed > 3000)
					pollingSpeed = 3000;

				screenClr();
				header();
				break;

				default:
				screenClr();
				header();
				_delay_ms(1);
				break;
			}
		} while (terminate == FALSE);
	
		_delay_ms(100);
	} 

	terminate = FALSE;

	_delay_ms(100);

}
// END ====================================

// - - - - -



void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();

	return;
}

void ADC_init()
{
	DDRC = 0x0;			// PORTC input

	DIDR0 = 0x1;		// disable digital input on ADC0 pin

	// initialize ADC
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | ( 1 <<ADPS1) | (1 << ADPS0);
	//ADCSRA |=
	// prescaler = 128
	ADCSRB = 0x0;		// free running mode

	ADMUX |= (1 << REFS0);

	printm("ADC initialized...");
	br();
	br();
}

float ADC_conv(unsigned int channel, unsigned int print)
{
	// ----------------------------------------
	// VARIABLE DECLARATION
	char		 channelChar[8];
	char		 stringValue[20];
	unsigned int ADCV = 0;
	float		 voltage = 0;
	// ----------------------------------------

	// ========================================
	// IMPLEMENATION
	// ----------------------------------------

	_delay_ms(1);

	// conversion
	if(ADC_en[channel] == 1)
	{
		ADMUX = 0100000 + channel;
		
		ADCSRA |= (1 << ADEN) | (1 << ADSC);
		
		_delay_ms(1);
		
		// wait for conversion
		while( (ADCSRA & (1 << ADIF) ) == 0 );
		
		ADCV = ADC & 0x03FF;				// grab only the 10 bits		// add offset to recorded voltage
		
		voltage = (ADCV * VCC / 0x3FF);		// convert int to float
		
		voltage += V_ADJUST;
		voltage += ADC_ADJUST;

		voltage += AD_global_offset;
		voltage += AD_offset[channel];

		if(voltage < 0)
		voltage = 0;
		
		dtostrf(voltage, 4, 3, stringValue);
		
		itoa(channel, channelChar, 10);
		
		if(print == TRUE)
		{
			printm("A");
			printm(channelChar);
			printm(" ");
			printm(stringValue);				// print the voltage
			printm(" V");				// suffix
		}
		
		return voltage;
	}
	
	else if(ADC_en[channel] == 0)
	{
		ADCSRA |= (0 << ADEN);

		return -1.0;
	}
	else
	return 0.0;
}

void internal_Volt()
{
	// ----------------------------------------
	// VARIABLE DECLARATION
	// char		 channelChar[8];
	char		 stringValue[20];
	double		 voltage = 0;
	// ----------------------------------------

	// ========================================
	// IMPLEMENATION
	// ----------------------------------------

	ADMUX = (0 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);

	_delay_ms(1);

	ADCSRA |= (1 << ADEN) | (1 << ADSC);

	_delay_ms(1);

	while( (ADCSRA & (1 << ADIF) ) == 0 );      // Detect end-of-conversion

	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
	uint8_t high = ADCH; // unlocks both

	long result = (high << 8) | low;

	result = 1125300 / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	
	voltage = result;
	voltage /= 1000;
	voltage += V_ADJUST;

	dtostrf(voltage, 2, 3, stringValue);
	printm(stringValue);				// print the voltage
	printm("V");				// suffix

	//return voltage;
}

void fiberDisplay(float voltage[5], float voltageMax, int channelMax, float voltageAvg, float voltageMin, int channelMin)
{
	float voltage_deg;
	char stringValue[20];

	printm("               ");

	if(voltage[4] == -1.0)
	printm("( - )");
	else if(voltage[4] < 0.5 && voltage[4] >= 0.0)
	printm("(   )");
	else if(voltage[4] >= 0.5 && voltage[4] < 2.0)
	printm("( * )");
	else if(voltage[4] >= 2.0)
	printm("( X )");

	printm("   ");

	if(voltage[5] == -1.0)
	printm("( - )");
	else if(voltage[5] < 0.5 && voltage[5] >= 0.0)
	printm("(   )");
	else if(voltage[5] >= 0.5 && voltage[5] < 2.0)
	printm("( * )");
	else if(voltage[5] >= 2.0)
	printm("( X )");

	br(); br();
	printm("           ");

	if(voltage[3] == -1.0)
	printm("( - )");
	else if(voltage[3] < 0.5 && voltage[3] >= 0.0)
	printm("(   )");
	else if(voltage[3] >= 0.5 && voltage[3] < 2.0)
	printm("( * )");
	else if(voltage[3] >= 2.0)
	printm("( X )");

	printm("   ( L )   ");
	
	if(voltage[0] == -1.0)
	printm("( - )");
	else if(voltage[0] < 0.5 && voltage[0] >= 0.0)
	printm("(   )");
	else if(voltage[0] >= 0.5 && voltage[0] < 2.0)
	printm("( * )");
	else if(voltage[0] >= 2.0)
	printm("( X )");

	br(); br();
	printm("               ");

	if(voltage[2] == -1.0)
	printm("( - )");
	else if(voltage[2] < 0.5 && voltage[2] >= 0.0)
	printm("(   )");
	else if(voltage[2] >= 0.5 && voltage[2] < 2.0)
	printm("( * )");
	else if(voltage[2] >= 2.0)
	printm("( X )");

	printm("   ");

	if(voltage[1] == -1.0)
	printm("( - )");
	else if(voltage[1] < 0.5 && voltage[1] >= 0.0)
	printm("(   )");
	else if(voltage[1] >= 0.5 && voltage[1] < 2.0)
	printm("( * )");
	else if(voltage[1] >= 2.0)
	printm("( X )");

	br(); br();
	printm("ANGLE-OF-ATTACK: ");

	voltage_deg = (voltageMax / VOLT_PER_DEGREE);

	if(voltage_deg >= 60.0)
		voltage_deg = 60;
	
	dtostrf(voltage_deg, 2, 1, stringValue);
	printm(stringValue);				// print the voltage
	printc(176);
	printm(" at max voltage");				// suffix

	if(voltageMax <= 0.2)
		intensity = SIGNAL_NA;
	else if(voltageMax > 0.200 && voltageMax <= 0.5)
		intensity = SIGNAL_L;
	else if(voltageMax > 0.5 && voltageMax <= 2.0)
		intensity = SIGNAL_M;
	else if(voltageMax > 2.0)
		intensity = SIGNAL_H;

	if(channelMax == 0)
		direction = EAST;
	else if(channelMax == 1)
		direction = SOUTHEAST;
	else if(channelMax == 2)
		direction = SOUTHWEST;
	else if(channelMax == 3)
		direction = WEST;
	else if(channelMax == 4)
		direction = NORTHWEST;
	else if(channelMax == 5)
		direction = NORTHEAST;


	/*
	if(voltage[0] && voltage[1] && voltage[2] &&
	voltage[3] && voltage[4] && voltage[5] <= 0 )
	{
		intensity = SIGNAL_NA;
	}
	

	else if( (voltage[0] || voltage[1] || voltage[2] ||
	voltage[3] || voltage[4] || voltage[5]) < 0.5 &&
	(voltage[0] || voltage[1] || voltage[2] ||
	voltage[3] || voltage[4] || voltage[5]) > 0 )
	{
		intensity = SIGNAL_L;
	}
	

	else if( (voltage[0] || voltage[1] || voltage[2] ||
	voltage[3] || voltage[4] || voltage[5]) >= 0.5 &&
	(voltage[0] || voltage[1] || voltage[2] ||
	voltage[3] || voltage[4] || voltage[5]) < 2.0 )
	{
		intensity = SIGNAL_M;
	}
	
	
	else if( (voltage[0] || voltage[1] || voltage[2] ||
	voltage[3] || voltage[4] || voltage[5]) >= 2.0)
	{
		intensity = SIGNAL_H;
	}
	

	// SINGLE PD CHECK
	if(voltage[0] >
	(voltage[1] && voltage[2] && voltage[3] &&
	voltage[4] && voltage[5]) )
	{
		direction = EAST;
	}

	else if(voltage[1] >
	(voltage[0] && voltage[2] && voltage[3] &&
	voltage[4] && voltage[5]) )
	{
		direction = SOUTHEAST;
	}

	else if(voltage[2] >
	(voltage[0] && voltage[1] && voltage[3] &&
	voltage[4] && voltage[5]) )
	{
		direction = SOUTHWEST;
	}

	else if(voltage[3] >
	(voltage[0] && voltage[1] && voltage[2] &&
	voltage[4] && voltage[5]) )
	{
		direction = WEST;
	}

	else if(voltage[4] >
	(voltage[0] && voltage[1] && voltage[2] &&
	voltage[3] && voltage[5]) )
	{
		direction = NORTHWEST;
	}

	else if(voltage[5] >
	(voltage[0] && voltage[1] && voltage[2] &&
	voltage[3] && voltage[4]) )
	{
		direction = NORTHEAST;
	}
	else
	direction = 00;

	// MULTI PD CHECK

	if( (voltage[0] && voltage[1] && voltage[5]) >
	(voltage[2] && voltage[3] && voltage[4]) )
	{
		direction = EAST;
	}

	else if( (voltage[0] && voltage[1]) >
	(voltage[2] && voltage[3] && voltage[4] && voltage[5]) )
	{
		direction = SOUTHEAST;
	}

	else if( (voltage[0] && voltage[1] && voltage[2]) >
	(voltage[3] && voltage[4] && voltage[5]) )
	{
		direction = SOUTHEAST;
	}

	else if( (voltage[1] && voltage[2]) >
	(voltage[0] && voltage[3] && voltage[4] && voltage[5]) )
	{
		direction = SOUTH;
	}

	else if( (voltage[2] && voltage[3]) >
	(voltage[0] && voltage[1] && voltage[4] && voltage[5]) )
	{
		direction = SOUTHWEST;
	}

	else if( (voltage[1] && voltage[2] && voltage[3]) >
	(voltage[0] && voltage[4] && voltage[5]) )
	{
		direction = SOUTHWEST;
	}

	else if( (voltage[2] && voltage[3] && voltage[4]) >
	(voltage[0] && voltage[1] && voltage[5]) )
	{
		direction = WEST;
	}

	else if( (voltage[3] && voltage[4]) >
	(voltage[0] && voltage[1] && voltage[2] && voltage[5]) )
	{
		direction = NORTHWEST;
	}

	else if( (voltage[3] && voltage[4] && voltage[5]) >
	(voltage[0] && voltage[1] && voltage[2]) )
	{
		direction = NORTHWEST;
	}

	else if( (voltage[4] && voltage[5]) >
	(voltage[0] && voltage[1] && voltage[2] && voltage[3]) )
	{
		direction = NORTH;
	}

	else if( (voltage[0] && voltage[5]) >
	(voltage[1] && voltage[2] && voltage[3] && voltage[4]) )
	{
		direction = NORTHEAST;
	}
	*/

	br(); br();
	printm("SIGNAL DIRECTION: ");

	switch (direction)
	{
		case NORTH:
		printm("NORTH");
		break;
		
		case NORTHEAST:
		printm("NORTHEAST");
		break;

		case EAST:
		printm("EAST");
		break;

		case SOUTHEAST:
		printm("SOUTHEAST");
		break;

		case SOUTH:
		printm("SOUTH");
		break;

		case SOUTHWEST:
		printm("SOUTHWEST");
		break;

		case WEST:
		printm("WEST");
		break;

		case NORTHWEST:
		printm("NORTHWEST");
		break;

		default:
		printm("INDETERMINANT");
		break;
	}

	printm("; ");

	switch (intensity)
	{
		case SIGNAL_L:
		printm("LIGHT");
		break;

		case SIGNAL_M:
		printm("MODERATE");
		break;

		case SIGNAL_H:
		printm("HEAVY");
		break;

		default:
		printm("N/A");
		break;
	}

}