#include "MoistureSensor.h"
#include "Arduino.h"

MoistureSensor::MoistureSensor(int aPin)
{
    switch(aPin)
    {
        case 0:
        pinMode(A0, INPUT);
        break;
        
        case 1:
        pinMode(A1, INPUT);
        break;
        
        case 2:
        pinMode(A2, INPUT);
        break;
        
        case 3:
        pinMode(A3, INPUT);
        break;
        
        case 4:
        pinMode(A4, INPUT);
        break;
        
        case 5:
        pinMode(A5, INPUT);
        break;
    }
    _myPin = aPin;
}

int middle(int v1, int v2, int v3)
{
    if(v1 <= v2 && v2 <= v3)
    {
        return v2;
    }
    if(v2 <= v1 && v1 <= v3)
    {
        return v1;
    }
    if(v1 <= v3 && v3 <= v2)
    {
        return v3;
    }

    return -1;
}

// Implementering af read ved moisture sensor
int MoistureSensor::read()
{
    // Read three values and return the middle value
    int val1 = analogRead(_myPin);
    int val2 = analogRead(_myPin);
    int val3 = analogRead(_myPin);

    return middle(val1, val2, val3);
}
