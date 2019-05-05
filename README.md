TWI librarfy is from TinyWireS ( https://github.com/nadavmatalon/TinyWireS ).
Makefile is from VUSB ( https://github.com/obdev/v-usb ).

Attiny44 was connected to arduino with TWI and arduino was conneced to Linux laptop with USB.
Only one attiny was tested at same time.

To flash attiny connect usbasp programmer and type 'make flash'.
To flash arduino use arduino IDE. Tested with "Blue Pill" (stm32f103c8t6) and "Arduino pro micro" (Atmega32U4).
To run Linux app type 'lua prog.lua'. It needs moonfltk ( https://github.com/stetre/moonfltk ).

It allows read and write predefined values, currently:

0) some 16 bit variable
1) ADC (ADC is running as differential with 20x amplification on PA0 and PA1)
2) some 16 bit variable
3) OCR0A (PWM is running as inverting Fast PWM on PA7)
4) some 32 bit variable

Variables have predefined length that must meet. 

TWI is sensitive to noise often with fatal (hang) effect.
