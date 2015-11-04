#include <iostream>

#include "MoistureSensor.h" // Burde jo .h   men altså.. Det virker!
//#import "Packet.h"
//#import "PacketHandler.h"

int main(int argc, char *argv[])
{
    MoistureSensor sensor(1);
    
    std::cout << "Main is running! \\o/" <<  std::endl << "Sensor value: " <<  sensor.read();
    
    return 0;
}
