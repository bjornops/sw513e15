#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <RF24/RF24.h>

using namespace std;

//Setup radio module on raspberry pins.
RF24 radio(22,0);

//Adress for data packages
const uint8_t rxAddr[6] = "00001";

int main(int argc, char** argv)
{
  //setup radio module
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setChannel(114);
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();

  //radio.printDetails();
  printf("Entering recieve-loop, ctrl+C to stop.");
  fflush(stdout);

  char emptyString[32] = {0};
  char receivedString[32] = {0};
  char expectedString[32] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};

  int errors = 0;
  int successes = 0;
  float errorRate = 0.0;


  while(true)
  {
    if (radio.available())
    {
      memcpy(receivedString,emptyString,32);
      radio.read(&receivedString, sizeof(receivedString));
      if(0 == strcmp(receivedString,expectedString))
      {
        successes++;
      }
      else
      {
        errors++;
      }
    }
    if(successes > 0)
    {
      errorRate = errors / successes;
    }
    printf("\nErrors: %d, Successes: %d, Error-rate :%f", errors, successes, errorRate );
    fflush(stdout);
  }
}



