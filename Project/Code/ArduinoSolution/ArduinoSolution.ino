#include <EEPROM.h>
#include "Arduino.h"
#include "printf.h"

#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include "Radio.h"
#include "MoistureSensor.h"
#include "Packet.h"
#include "Node.h"

void setup()
{
    Serial.begin(57600);
    printf_begin();
    
    // Skal der sendes pair request ved startup?
    bool sendPairRequest = false;
    /*
    pinMode(2, INPUT);
    int val = digitalRead(2);
    if(val == HIGH)
    {
        Serial.println("Sender pair request!");
    }
    else
    {
        Serial.println("Ingen pair request!");
    }
    */
    // Lav sensor og radio
    MoistureSensor sensor(1);
    NRF24Radio radio(7, 8);
    
    // Initialiser og start node
    Node::initializeNode(&sensor, &radio);
    Node::begin(sendPairRequest);
}

void loop()
{ 
}


int address = 0;
byte value;
void dumpEEPROM()
{
    value = EEPROM.read(address);
  
    Serial.print(address);
    Serial.print("\t");
    Serial.print(value, DEC);
    Serial.println();
  
    /***
      Advance to the next address, when at the end restart at the beginning.
  
      Larger AVR processors have larger EEPROM sizes, E.g:
      - Arduno Duemilanove: 512b EEPROM storage.
      - Arduino Uno:        1kb EEPROM storage.
      - Arduino Mega:       4kb EEPROM storage.
  
      Rather than hard-coding the length, you should use the pre-provided length function.
      This will make your code portable to all AVR processors.
    ***/
    address = address + 1;
    if (address == EEPROM.length()) {
      address = 0;
    }
  
    /***
      As the EEPROM sizes are powers of two, wrapping (preventing overflow) of an
      EEPROM address is also doable by a bitwise and of the length - 1.
  
      ++address &= EEPROM.length() - 1;
    ***/
  
    delay(10);
}
