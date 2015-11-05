#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(53,48); // Mega
RF24 radio(7, 8); // Uno
const byte rxAddr[6] = "00001";

void setup()
{
  Serial.begin(9600);
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  
  radio.setPALevel(RF24_PA_MIN);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setChannel(114);
  radio.openWritingPipe(rxAddr);
  
  radio.stopListening();
}

void loop()
{
  const char text[] = "10101010101010101010101010101010"; 
  // "10101010101010101010101010101010"; 
  // "11111111111111111111111111111111";
  // "01010101010101010101010101010101";
  radio.write(&text, sizeof(text));
  delay(2); // 2 default
}
