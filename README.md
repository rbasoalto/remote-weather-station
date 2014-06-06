# MSP430 Remote Temp/Humidity probe with DHT22 and nRF24L01

This is a test program / proof of concept of reading the [DHT22](http://www.lmgtfy.com/?q=DHT22) using Timer1A in the MSP430G2553 on the Launchpad (or whatever).

## Hardware

You'll need:
- MSP430 Launchpad with MSP430G2553 (or something else)
- DHT22 with required pullup resistor
- nRF24L01 module from your favorite chinese supplier

Build, program, and see the temperature and humidity on your favorite serial console program.

```
----------------+      +---------------+
MSP430G2553     |      |    nRF24L01   |
                |      |               |
  P1.5/UCB0CLK  |------| SCL           |
  P1.6/UCB0SOMI |------| MISO          |
  P1.7/UCB0SIMO |------| MOSI          |
           P2.0 |------| CE            |
           P2.1 |------| CSN           |
           P2.2 |------| IRQ           |
                |      +---------------+
                |
                |   ^ VCC
                |   |
                |   |  +---------+
                |  R1k |  DHT22  |
                |   |  |         |
    P2.3/TA1.0B |---+--| DATA    |
                |      +---------+
----------------+
```

# License

Licensed under the MIT License. See `LICENSE`.
