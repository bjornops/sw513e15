#include "MoistureSensor.h"

MoistureSensor::MoistureSensor(int aPin) 
{
    myPin = aPin;
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
<<<<<<< Updated upstream
int MoistureSensor::read() {
    int val =  1337; // analogRead(myPin);
    return val;
}
=======
int MoistureSensor::read() 
{
    // Read three values and return the middle value
    int val1 = 1337; // analogRead(myPin);
    int val2 = 1338; // analogRead(myPin);
    int val3 = 1339; // analogRead(myPin);
    
    return middle(val1, val2, val3);
}
>>>>>>> Stashed changes
