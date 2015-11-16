class MoistureSensor: public iSensor
{
private:
    int myPin;
    
public:
    MoistureSensor(int);
    int read();  
};
