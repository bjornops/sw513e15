#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

int turn;
const byte rxAddr[6] = "00001";

void setup()
{
  turn = 0;
  
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  
  radio.stopListening();
}

void loop()
{
  if(turn == 0)
  {
    const char text[] = "ON";
    radio.write(&text, sizeof(text));
    
    turn = 1;
  }
  else if(turn == 1)
  {
    const char text[] = "OFF";
    radio.write(&text, sizeof(text));
    
    turn = 0;
  }
  
  delay(1000);
}
