
/*
 RFM69 MBus

 Hints -------------------------------------------------------
 The commercial use of this firmware is not allowed.
 -------------------------------------------------------------

 History -----------------------------------------------------
 2016-06-27 begin programming
 2016-07-26 initial version
 2017-03-05 new functions und bugfixes
 -------------------------------------------------------------

 Bugs --------------------------------------------------------
 -------------------------------------------------------------

 Marks -------------------------------------------------------
 -------------------------------------------------------------

 Hardware ----------------------------------------------------
 Atmega 328P 16MHz at 3V3
 RFM69W or RFM69CW to receive MBus Date from heat meter
 -------------------------------------------------------------

 Pin usage ---------------------------------------------------
 APin  0   DPin 2   (D0/RX)   in  ... RxD
 APin  1   DPin 3   (D1/TX)   out ... TxD
 APin  2   DPin 4   (D2/INT0) in  ... DIO0
 APin  3   DPin 5   (D3/INT1) in  ... 
 APin  4   DPin 6   (D4)      out ... LED
 APin  5   DPin 11  (D5)      in  ... 
 APin  6   DPin 12  (D6)      in  ...
 APin  7   DPin 13  (D7)      in  ...
 APin  8   DPin 14  (B0)      in  ...
 APin  9   DPin 15  (B1)      in  ...
 APin 10   DPin 16  (B2)      out ... NSS
 APin 11   DPin 17  (B3/MOSI) out ... MOSI
 APin 12   DPin 18  (B4/MISO) in  ... MISO
 APin 13   DPin 19  (B5/SCK)  out ... SCK
 APin A0   DPin 23  (C0)      in  ... 
 APin A1   DPin 24  (C1)      in  ...
 APin A2   DPin 25  (C2)      in  ...
 APin A3   DPin 26  (C3)      in  ...
 APin A4   DPin 27  (C4/SDA)  i/o ...
 APin A5   DPin 28  (C5/SCL)  out ...
 -------------------------------------------------------------
 */

#include <SPI.h>
#include "RFM69mbus.h"
#include <stdio.h>
#include <stdlib.h>
#include "msgdecoder.h"

//declarations for pins
#define PinDIO0 2
#define PinLED LED_BUILTIN
//#define PinNSS 9
#define PinNSS 10

#define PACKET_OK 0
#define PACKET_CODING_ERROR 1
#define PACKET_CRC_ERROR 2

//instance RFM69
RFM69 rfm69;

//declarations for prg
unsigned char i;

int8_t PAind = 13;

void blink(unsigned char Times)
{
	for (i = 0; i < Times; i++)
	{
		digitalWrite(PinLED, 1);
		delay(75);
		digitalWrite(PinLED, 0);
		delay(75);
	}
}

void checkcmd()
{
	String cmdstr;
	unsigned char reg, regval;
	char *ptr;
	if (Serial.available() > 0)
	{
		cmdstr = Serial.readStringUntil("\n");
		//we assume register, value = 5 byte
		reg = strtoul(cmdstr.c_str(), &ptr, 16);
		regval = strtoul(cmdstr.substring(3).c_str(), &ptr, 16);

		rfm69.writeSPI(reg, regval);
		Serial.print("set ");
		Serial.print(reg, HEX);
		Serial.print(" to ");
		Serial.println(regval, HEX);
	}
}

void setup()
{
	//blink indicating reset
	pinMode(PinLED, OUTPUT);
	blink(5);
	Serial.begin(115200);
	Serial.println("RFM69 MBus Receiver");
	//init device RFM69
	if (!rfm69.initDevice(PinNSS, PinDIO0, CW, 868.95, GFSK, 100000, 40000, 5, PAind))
		Serial.println("error initializing device");
}

void loop()
{
	delay(10);

	//if (rfm69.rxMBusMsg())
	if (rfm69.receiveSizedFrame(FixPktSize))
	{
		printmsg(rfm69);
	}

	checkcmd();
}
