#include "tm.h"

const unsigned char num[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

#ifdef TM1638_USE_USCI_B
#define TM_UCBR0    UCB0BR0
#define TM_UCBR1    UCB0BR1
#define TM_UCCTL0   UCB0CTL0
#define TM_UCCTL1   UCB0CTL1
#define TM_UCRXBUF  UCB0RXBUF
#define TM_UCRXIE   UCB0RXIE
#define TM_UCRXIFG  UCB0RXIFG
#define TM_UCSTAT   UCB0STAT
#define TM_UCTXBUF  UCB0TXBUF
#define TM_UCTXIE   UCB0TXIE
#define TM_UCTXIFG  UCB0TXIFG
#else
#define TM_UCBR0    UCA0BR0
#define TM_UCBR1    UCA0BR1
#define TM_UCCTL0   UCA0CTL0
#define TM_UCCTL1   UCA0CTL1
#define TM_UCRXBUF  UCA0RXBUF
#define TM_UCRXIE   UCA0RXIE
#define TM_UCRXIFG  UCA0RXIFG
#define TM_UCSTAT   UCA0STAT
#define TM_UCTXBUF  UCA0TXBUF
#define TM_UCTXIE   UCA0TXIE
#define TM_UCTXIFG  UCA0TXIFG
#endif

inline void tm_send(unsigned char data) {
    __delay_cycles(100);
    TM_UCTXBUF = data;
    while(!(TM_UCSTAT & UCBUSY)); //busywaiting!
    __delay_cycles(500);
}

inline unsigned char tm_recv() {
    tm_send(0xFF);
    return TM_UCRXBUF;
}

static void tm_spi_init() {
  TM_UCCTL1 |= UCSWRST;
  
  TM_UCCTL0 = UCCKPH | UCMST | UCSYNC;
  TM_UCCTL1 = UCSSEL_2 | UCSWRST; // Still in reset while configuring
  
  IE2 &= ~(TM_UCTXIE | TM_UCRXIE);
  IFG2 &= ~(TM_UCTXIFG | TM_UCRXIFG);
  
  TM_PDIR |= TM_STB;
  TM_PSEL |= TM_MOSI | TM_CLK | TM_MISO;
  TM_PSEL2 |= TM_MOSI | TM_CLK | TM_MISO;
  TM_POUT |= TM_STB;
  
  TM_UCBR0 = 32; // SMCLK/16 = 500 KHz (if faster, reads are erratic)
  TM_UCBR1 = 0;
  
  TM_UCSTAT = 0;
  
  TM_UCCTL1 &= ~UCSWRST; // OK Enable USCI
}

static void tm_clear() {
    unsigned char i;
    TM_POUT &= ~TM_STB;
    tm_send(0x40);
    tm_send(0xC0);
    for(i = 0; i < 16; i++) {
        tm_send(0x00);
    }
    TM_POUT |= TM_STB;
}

void tm_init(unsigned char brightness) {
    tm_spi_init();
    tm_cmd(0x40);
    tm_cmd(0x80 | (brightness&0x0F));
    tm_clear();
}

void tm_cmd(unsigned char data) {
    TM_POUT &= ~TM_STB;
    tm_send(data);
    TM_POUT |= TM_STB;
}

void tm_data(unsigned char data, unsigned char addr) {
    tm_cmd(0x44);
    TM_POUT &= ~TM_STB;
    tm_send(0xC0 | addr);
    tm_send(data);
    TM_POUT |= TM_STB;
}

unsigned char tm_getButtons() {
    unsigned char i, temp = 0;
    TM_POUT &= ~TM_STB;
    tm_send(0x42);
    for(i=0; i < 4; i++) {
        temp |= tm_recv() << i;
    }
    TM_POUT |= TM_STB;
    return temp;
}

void tm_setLed(unsigned char color, unsigned char idx) {
    tm_data(color, (idx<<1)+1);
}

void tm_setDigit(unsigned char digit, unsigned char idx) {
    tm_data(num[digit], (idx<<1));
}

void tm_setDigitWithDot(unsigned char digit, unsigned char idx) {
    tm_data(num[digit]|0x80u, (idx<<1));
}

void tm_testDots() {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        tm_data(0x80u, i<<1);
    }
}

void tm_testDigits() {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        tm_setDigit(i, i);
    }
}

void tm_testLeds() {
    unsigned char i;
    for(i = 0; i < 4; i++) {
        tm_setLed(TM_RED, (i<<1)+1);
        tm_setLed(TM_GREEN, (i<<1));
    }
}

void tm_allOn() {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        tm_setLed(TM_RED | TM_GREEN, i);
        tm_data(0xFFu, i<<1);
    }
}