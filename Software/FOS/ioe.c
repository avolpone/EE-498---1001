#define F_CPU 8000000
#include <avr/io.h>
#include <stdlib.h>

void UART_init(unsigned char mode, unsigned char interrupted)
{	
	DDRD |= (1<<PORTD1);	
	DDRD &= ~(1<<PORTD0);		

	#define BAUD 76800

	#include <util/setbaud.h>

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	switch (mode){
		case 1:			
			UCSR0A |= (1<<U2X0);
			break;

		case 2:		
			UCSR0C |= (1<<UMSEL00);
			break;

		default:		
			UCSR0A &= ~(1<<U2X0);
			break;
	}

	if (interrupted)
		UCSR0B |= (1<<RXCIE0);	

	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);		
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);	

	screenClr();

		printm("UART initialized...");
		br(); br();
		printm("Interrupts initialized...");
		
}

unsigned char UART_getChar()
{
	while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

void printc(unsigned char data)
{
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void printm(char *output)
{
    char i = 0;
    while(*(output + i) != '\0'){
        printc(*(output + i));
        i++;
    }
}

void scanm(unsigned char* buffer)
	{
		int i = 0;
		while(buffer[i] != '\n'){
			buffer[i] = UART_getChar();
			if (buffer[i]=='\r')
				break;
			printc(buffer[i]);
			i++;
		}
		buffer[i] = '\0';
	}

void br()
{

		// default line feed
		printm("\n"); // line feed

		// windows required character
		printm("\r"); // only for windows
}

/*
void br(unsigned int amount)
{
	if(amount == NULL)
		amount = 0;

	for(int i = 0; i < amount; i++)
	{
		// default line feed
		printm("\n"); // line feed

		// windows required character
		printm("\r"); // only for windows
	}
}
*/

void screenClr()
{
	printc('A');

	printc(27);
	printc('[');
	printc('2');
	printc('J');

	printc(27);
	printc('[');
	printc('H');
}