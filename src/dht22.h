#ifndef __DHT_22_H__
#define __DHT_22_H__

#include <stdint.h>

// Change only the following 3 settings:
#define DHT22_PORT    2
#define DHT22_BIT     BIT3
// Capture/Compare input 0 is A, 1 is B
#define DHT22_CCI_TYPE 1

#if DHT22_PORT == 1
#define DHT22_PORT_OUT  (P1OUT)
#define DHT22_PORT_DIR  (P1DIR)
#define DHT22_PORT_IFG  (P1IFG)
#define DHT22_PORT_IES  (P1IES)
#define DHT22_PORT_IE   (P1IE)
#define DHT22_PORT_SEL  (P1SEL)
#define DHT22_PORT_SEL2 (P1SEL2)
#define DHT22_PORT_REN  (P1REN)
#elif DHT22_PORT == 2
#define DHT22_PORT_OUT  (P2OUT)
#define DHT22_PORT_DIR  (P2DIR)
#define DHT22_PORT_IFG  (P2IFG)
#define DHT22_PORT_IES  (P2IES)
#define DHT22_PORT_IE   (P2IE)
#define DHT22_PORT_SEL  (P2SEL)
#define DHT22_PORT_SEL2 (P2SEL2)
#define DHT22_PORT_REN  (P2REN)
#endif

uint8_t* dht_get_data();
uint8_t dht_is_ready();
void dht_start_read();
int dht_get_temp();
int dht_get_rh();

#endif
