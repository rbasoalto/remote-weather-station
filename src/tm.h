#ifndef __TM_H__
#define __TM_H__

#include <msp430.h>

#define TM_RED 1
#define TM_GREEN 2

#define TM_MOSI BIT2
#define TM_MISO BIT1
#define TM_CLK BIT4
#define TM_STB BIT0

#define TM_POUT P1OUT
#define TM_PDIR P1DIR
#define TM_PIN P1IN
#define TM_PSEL P1SEL
#define TM_PSEL2 P1SEL2
#define TM_PREN P1REN

void tm_init(unsigned char brightness);
void tm_cmd(unsigned char data);
void tm_data(unsigned char data, unsigned char addr);
void tm_send(unsigned char data);
unsigned char tm_recv();
unsigned char tm_getButtons();
void tm_setLed(unsigned char color, unsigned char idx);
void tm_setDigit(unsigned char digit, unsigned char idx);
void tm_setDigitWithDot(unsigned char digit, unsigned char idx);
void tm_testLeds();
void tm_testDigits();
void tm_testDots();
void tm_allOn();

#endif