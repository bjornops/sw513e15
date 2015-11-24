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
    Serial.begin(9600);
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
