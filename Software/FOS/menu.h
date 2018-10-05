#ifndef menus
#define menus

void delay_ms(int n);	
void delay_us(int n);
void header();

unsigned char mainMenu();	

void calibrationMenu();
	
void diagnosticMenu();

void powerModes();	

/*

void ledModes(unsigned int mode);		

void ledBlinking();

void ledPWM();

*/

#endif
