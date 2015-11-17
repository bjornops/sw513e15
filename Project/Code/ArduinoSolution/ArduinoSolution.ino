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
    
    // Lav sensor og radio
    MoistureSensor sensor(1);
    NRF24Radio radio(7, 8);
    
    // Initialiser og start node
    Node::initializeNode(&sensor, &radio);
    Node::begin();
}

void loop()
{ 
}
