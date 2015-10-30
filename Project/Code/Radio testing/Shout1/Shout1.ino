#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int count = 0;

void setup()
{
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  
  radio.stopListening();
}

void loop()
{
  if(count < 10000)
  {
  const char text[] = "11111111111111111111111111111111";
  radio.write(&text, sizeof(text));
  
  delay(2);
  count++;
  }
}
