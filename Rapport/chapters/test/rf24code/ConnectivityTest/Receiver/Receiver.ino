#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte rxAddr[6] = "00001";
int receivedPackets;

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  
  receivedPackets = 0;
  
  while (!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.openReadingPipe(0, rxAddr);

  radio.startListening();
}

void loop()
{
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    receivedPackets += 1;
    
    String s = String("Packets: ");
    String s2 = s + receivedPackets;
    Serial.println(s2);
  }
}
