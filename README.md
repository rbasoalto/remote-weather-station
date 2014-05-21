# MSP430 Remote Temp/Humidity probe with DHT22 and nRF24L01

This is a test program / proof of concept of reading the [DHT22](http://www.lmgtfy.com/?q=DHT22) using Timer1A in the MSP430G2553 on the Launchpad (or whatever).

## Hardware

You'll need:
- MSP430 Launchpad with MSP430G2553 (or something else)
- DHT22 with required pullup resistor
- nRF24L01 module from your favorite chinese supplier

Connect the data pin of the DHT22 to P2.0 on the Launchpad (that's pin 8 on J1).

Connect the nRF module (guess...)

Build, program, and see the temperature and humidity on your favorite serial console program.

# License

Licensed under the MIT License. See `LICENSE`.
