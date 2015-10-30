class iSensor {
public:
    virtual double read() = 0;
};

// Moisture sensor, nedarver fra iSensor (Implementer metoder)
class MoistureSensor: public iSensor {
  int myPin;
public:
    double read();
    MoistureSensor(int);
};

MoistureSensor::MoistureSensor(int aPin) {
    myPin = aPin;
}

// Implementering af read ved moisture sensor
double MoistureSensor::read() {
    int val =  analogRead(myPin);
    return val;
}

MoistureSensor sens(1);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(sens.read());
}


