#include "iSensor.h"

#ifndef MOISTURESENSOR_H
#define MOISTURESENSOR_H

class MoistureSensor: public iSensor
{
private:
    int myPin;
    
public:
    MoistureSensor(int);
    int read();  
};

#endif
