#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#include "Packet.h"
#include "Radio.h"
#include "MoistureSensor.h"
#include "Node.h"

int main(int argc, char *argv[])
{
    MoistureSensor sensor(1);
    NRF24Radio radio(8, 7);
    
    Node::initializeNode(&sensor, &radio);
    
    return 0;
}
