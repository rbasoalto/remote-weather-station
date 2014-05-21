#include <msp430.h>
#include "dht22.h"

enum {
  DHT_UNINIT,
  DHT_READY,
  DHT_TRIGGERING,
  DHT_WAITING_ACK,
  DHT_ACK_LOW,
  DHT_ACK_HIGH,
  DHT_IN_BIT_LOW,
  DHT_IN_BIT_HIGH,
} dht_current_state = DHT_UNINIT;

union {
  struct {
    uint8_t hh;
    uint8_t hl;
    uint8_t th;
    uint8_t tl;
    uint8_t crc;
  } val;
  uint8_t bytes[5];
} dht_data;

uint8_t dht_data_byte, dht_data_bit;

uint8_t
dht_is_ready() {
  return dht_current_state == DHT_READY;
}

uint8_t*
dht_get_data() {
  while (dht_current_state != DHT_READY);
  return dht_data.bytes;
}

int
dht_get_temp() {
  uint16_t temp_temp;
  while (dht_current_state != DHT_READY);
  temp_temp = (((dht_data.val.th&0x7f)<<8)+dht_data.val.tl);
  return ((-1)*((dht_data.val.th&0x80)>>7)+temp_temp);
}

int
dht_get_rh() {
  uint16_t temp_rh;
  while (dht_current_state != DHT_READY);
  temp_rh = (dht_data.val.hh<<8)+dht_data.val.hl;
  return temp_rh;
}

void
dht_start_read() {
  // First, low pulse of 1ms
  DHT22_PORT_OUT &= ~DHT22_BIT;
  DHT22_PORT_SEL &= ~DHT22_BIT;
  DHT22_PORT_DIR |= DHT22_BIT;

  TA1CCTL0 &= ~CCIFG;
  TA1CCR0 = 16000u;
  TA1CCTL0 = CCIE;
  TA1CTL = TACLR;
  TA1CTL = TASSEL_2 | ID_0 | MC_1;

  dht_current_state = DHT_TRIGGERING;
}

void __attribute__((interrupt (TIMER1_A0_VECTOR)))
timer1_a0_isr() {
  TA1CCTL0 &= ~CCIFG;
  // This handles only TA1CCR0 interrupts
  switch (dht_current_state) {
  case DHT_UNINIT:
  case DHT_READY:
    break; // Shouldn't be here
  case DHT_TRIGGERING:
    // 1ms has passed since setting the pin low
    // Let P2.0 go high and set Compare input on T1
    DHT22_PORT_DIR &= ~DHT22_BIT; // input
    DHT22_PORT_SEL |= DHT22_BIT; // Timer1_A3.CCI0A input
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_WAITING_ACK;
    break;
  case DHT_WAITING_ACK:
    // I don't care about timings here...
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_ACK_LOW;
    break;
  case DHT_ACK_LOW:
    // I don't care about timings here either...
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_ACK_HIGH;
    dht_data_byte = dht_data_bit = 0;
    break;
  case DHT_ACK_HIGH:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_IN_BIT_LOW;
    break;
  case DHT_IN_BIT_LOW:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_IN_BIT_HIGH;
    break;
  case DHT_IN_BIT_HIGH:
    // OK now I need to measure the time since last time
    dht_data.bytes[dht_data_byte] <<= 1;
    if (TA1CCR0 > 750) {
      // Long pulse: 1
      dht_data.bytes[dht_data_byte] |= 1;
    }
    if (++dht_data_bit >= 8) {
      dht_data_bit = 0;
      dht_data_byte++;
    }
    if (dht_data_byte >= 5) {
      // I'm done, bye
      TA1CTL = TACLR;
      dht_current_state = DHT_READY;
      LPM4_EXIT; // Exit any low-power mode after having a good reading
    } else {
      TA1CTL = TACLR;
      TA1CTL = TASSEL_2 | ID_0 | MC_2;
      TA1CCTL0 = CM_1 | (CCIS_1*DHT22_CCI_TYPE) | CAP | CCIE; // Capture on rising edge
      dht_current_state = DHT_IN_BIT_LOW;
    }
    break;
  }
}
