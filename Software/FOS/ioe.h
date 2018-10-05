#ifndef ioe
#define ioe

void UART_init(unsigned char, unsigned char);
unsigned char UART_getChar();
void printc(unsigned char data);

void printm(char* output);
void scanm(unsigned char* buffer);
void br();
void screenClr();

#endif