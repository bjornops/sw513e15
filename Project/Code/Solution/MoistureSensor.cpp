#import "MoistureSensor.h"

MoistureSensor::MoistureSensor(int aPin) {
    myPin = aPin;
}

// Implementering af read ved moisture sensor
int MoistureSensor::read() {
    int val =  1337; // analogRead(myPin);
    return val;
}