#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include "dht22.h"
#include "msprf24.h"
#include "tm.h"


#ifdef UART_DEBUG
#define DEBUGMSG(x) (uart_send(x))
char txbuf[256];
inline void uart_setup() {
  UCA0CTL0 = 0;
  UCA0CTL1 |= UCSWRST; // Put USCI in reset mode
  UCA0CTL1 = UCSSEL_2 | UCSWRST; // Use SMCLK, still reset
  UCA0MCTL = UCBRF_0 | UCBRS_6;
  UCA0BR0 = 131; // 9600 bauds
  UCA0BR1 = 6;
  UCA0CTL1 &= ~UCSWRST; // Normal mode
	P1SEL2 |= (BIT1 + BIT2); // Set pins for USCI
	P1SEL |= (BIT1 + BIT2);
}
void uart_send(int len) {
  int i;
  for(i = 0; i < len; i++) {
    while (UCA0STAT & UCBUSY);
    UCA0TXBUF = txbuf[i];
  }
}
#else
#define DEBUGMSG(x) ((void)0)
inline void uart_setup() {}
#endif

void radio_setup() {
  uint8_t addr[] = {0xDE, 0xAD, 0x00, 0xBE, 0xEF};
  rf_crc = RF24_EN_CRC | RF24_CRCO;
  rf_addr_width = 5;
  rf_speed_power = RF24_SPEED_250KBPS | RF24_POWER_0DBM;
  rf_channel = 120;
  // Initialize and such
  msprf24_init();
  msprf24_set_pipe_packetsize(0, 0);
  msprf24_open_pipe(0, 1);
  // Wake up from deep sleep (if...)
  msprf24_standby();
  // Set TX and RX addresses (AutoACKs are sent to the TX address)
  w_tx_addr(addr);
  w_rx_addr(0, addr);
}

void verylongdelay() {
  TA0CCR0 = 40960; // About 10sec
  TA0CCTL0 |= CCIE;
  TA0CTL = TACLR;
  TA0CTL = TASSEL_1 | ID_3 | MC_1; // ACLK/8 (4096 hz), up to CCR0
  LPM3;
}

void __attribute__((interrupt (TIMER0_A0_VECTOR)))
verylongdelayisr() {
  TA0CTL = TACLR;
  LPM4_EXIT;
}

int main() {
  uint8_t buf[32];
  uint8_t is_first_tx = 1;

  WDTCTL = WDTPW | WDTHOLD;
  DCOCTL = 0;
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
  BCSCTL3 |= LFXT1S_2;

  P1DIR |= BIT0;
  P1OUT &= ~BIT0;

  _BIS_SR(GIE);
  uart_setup();
  DEBUGMSG(sprintf(txbuf, "Hello! Starting up...\r\n"));
  radio_setup();
  DEBUGMSG(sprintf(txbuf, "Radio is ready...\r\n"));
#ifdef WITH_TM_1638
  tm_init(0x08);
#endif
  while(1) {
    if (is_first_tx) {
      is_first_tx = 0;
    } else {
      msprf24_powerdown();
      P1OUT &= ~BIT0;
      verylongdelay();
    }
    DEBUGMSG(sprintf(txbuf, "Starting read\r\n"));
    dht_start_read();
    int t = dht_get_temp();
    int h = dht_get_rh();
    DEBUGMSG(sprintf(txbuf, "%3d.%1d C; %3d.%1d %%RH\r\n", t/10, t%10, h/10, h%10));
#ifdef WITH_TM_1638
    int i;
    tm_setDigitWithDot(t%10, 3);
    t/=10;
    for (i=2; i >= 0; i--) {
      tm_setDigit(t%10, i);
      t/=10;
    }
    tm_setDigitWithDot(h%10, 7);
    h/=10;
    for (i=6; i >= 4; i--) {
      tm_setDigit(h%10, i);
      h/=10;
    }
#endif
    memcpy(buf, dht_get_data(), 5);
    msprf24_standby();
    w_tx_payload(5, buf);
    msprf24_activate_tx();
    DEBUGMSG(sprintf(txbuf, "Message TX started\r\n"));
    LPM4;
    DEBUGMSG(sprintf(txbuf, "Interrupt received!\r\n"));
    if (rf_irq & RF24_IRQ_FLAGGED) {
      rf_irq &= ~RF24_IRQ_FLAGGED;
      msprf24_get_irq_reason();
      if (rf_irq & RF24_IRQ_TX) {
        P1OUT &= ~BIT0;
        DEBUGMSG(sprintf(txbuf, "TX Success!\r\n"));
      }
      if(rf_irq & RF24_IRQ_TXFAILED) {
        P1OUT |= BIT0;
        DEBUGMSG(sprintf(txbuf, "TX Failed!\r\n"));
      }
      msprf24_irq_clear(RF24_IRQ_MASK);
    }
  }

  return 0;
}
