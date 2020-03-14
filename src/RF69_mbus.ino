
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

//declarations for receive

unsigned char RSSI;

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

void printRxFrame()
{
	Serial.print("Rx: ");
	for (i = 0; i < rfm69._RxBufferLen; i++)
	{
		Serial.print(rfm69._RxBuffer[i], HEX);
		//Serial.print(' ');
	}
	Serial.print("* ");
	Serial.print(rfm69.convertRSSIToRSSIdBm(RSSI), 1);
	Serial.println("dBm");
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
	rfm69.setCalibrationTemp(164);
	//set sync words
	unsigned char SyncBytes[] = {0x54, 0x3D}; // lets start relaxed
	rfm69.setSyncWords(SyncBytes, sizeof(SyncBytes));
	rfm69.useSyncWords(true);

	rfm69.writeSPI(RFM69_REG_38_PAYLOADLENGTH, 0);	// unlimited
	rfm69.writeSPI(RFM69_REG_37_PACKETCONFIG1, 0x00); // fixed length, noCRC, no address filter
	rfm69.writeSPI(RFM69_REG_3C_FIFOTHRESH, MSGBLK1); //first mbus block
}

void loop()
{
	delay(10);

	if (rfm69.rxMBusMsg())
	{
		//if (rfm69.receiveSizedFrame(27)) {

		RSSI = rfm69.getLastRSSI();

		if (RSSI < 180)
		{ // only our test msg
			//printRxFrame();

			rfm69.decode3o6Block(rfm69._RxBuffer, rfm69._mbusmsg, rfm69._RxBufferLen);
			Serial.print("mbmsg: ");
			for (i = 0; i < rfm69._mbusmsg[0] + 1; i++)
			{
				char tempstr[3];
				sprintf(tempstr, "%02X", rfm69._mbusmsg[i]);
				Serial.print(tempstr);
			}
			Serial.print(":");
			Serial.print((rfm69._RxBufferLen * 2 / 3), HEX);

			Serial.print(":");
			Serial.print(rfm69.msgerr, HEX);
			Serial.println();
			
			uint16_t mtype = get_type(rfm69._mbusmsg);

			Serial.print("msgdec: ");
			Serial.print(get_vendor(rfm69._mbusmsg), HEX);
			Serial.print(";");
			Serial.print(get_serial(rfm69._mbusmsg), HEX);
			Serial.print(";");

			if (mtype == 0x8069)
			{
				Serial.print(get_temp1(rfm69._mbusmsg));
				Serial.print(";");
				Serial.print(get_temp2(rfm69._mbusmsg));
				Serial.print(";");
				Serial.print(get_actHKZ(rfm69._mbusmsg));
				Serial.print(";");
				Serial.println(get_prevHKZ(rfm69._mbusmsg));
			}
			else if ((mtype & 0xFF00) == 0x4300)
			{
				Serial.print(get_last(rfm69._mbusmsg));
				Serial.print(";");
				Serial.println(get_current(rfm69._mbusmsg));
			}
			else
			{
				Serial.println();
			};
		}
	}
	checkcmd();
}
