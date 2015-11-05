#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <RF24/RF24.h>

using namespace std;

void setupRadio();
void receiveLoop(unsigned int goal, unsigned int interval);

//Setup radio module on raspberry pins.
RF24 radio(22,0);

//Adress for data packages
const uint8_t rxAddr[6] = "00001";

int main(int argc, char** argv)
{
  unsigned int goalK = 10;
  unsigned int intervalK = 10;

  if (argc >= 2)
  {
    goalK = atoi(argv[1]);
  }

  if (argc >= 3)
  {
    intervalK = atoi(argv[2]);
  }

  setupRadio();
  receiveLoop(goalK*1000,intervalK*1000);

  return 0;
}

void setupRadio()
{
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setChannel(114);
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();
}

void receiveLoop(unsigned int goal, unsigned int interval)
{
  const char emptyString[32] = {0};
  char receivedString[33] = "00000000000000000000000000000000";
  char expectedString[33] = "10101010101010101010101010101010";

  unsigned int sessionErrors = 0;
  unsigned int sessionSuccesses = 0;
  double sessionErrorRate = 1.0;

  unsigned int totalErrors = 0;
  unsigned int totalSuccesses = 0;
  double totalErrorRate = 1.0;

  unsigned int sessionCount = 0;
  unsigned int intervalCount = 0;

  FILE *fp;
  fp = fopen("result.txt","w+");

  while(totalErrors+totalSuccesses < goal)
  {
    if (radio.available())
    {
      strcpy(receivedString,emptyString);
      //memcpy(receivedString,emptyString,32);
      //printf("\nreceivedString before receive: %s",receivedString);
      radio.read(&receivedString, sizeof(receivedString));
      //printf("\nreceivedString after receive: %s",receivedString);
      if(0 == strcmp(receivedString,expectedString))
      {
        totalSuccesses++;
        sessionSuccesses++;
      }
      else
      {
        totalErrors++;
        sessionErrors++;
      }
      sessionCount++;

      //print stuff
      if(sessionCount >= interval)
      {
        intervalCount++;
        if(sessionSuccesses > 0)
        {
          sessionErrorRate = (double)sessionErrors / sessionSuccesses;
          totalErrorRate = (double)totalErrors / totalSuccesses;
        }
        printf("Total packets received: %d * %dk:\n\tSession errors: %d,\tSession successes: %d,\tSession error-rate: %f%%\n\tTotal errors: %d,\tTotal successes: %d,\t\tTotal error-rate: %f%%\n", intervalCount,interval/1000, sessionErrors, sessionSuccesses, sessionErrorRate*100, totalErrors, totalSuccesses, totalErrorRate*100);
        fprintf(fp, "%d,%d\n",sessionErrors,sessionSuccesses);
        sessionCount = 0;
        sessionErrors = 0;
        sessionSuccesses = 0;
      }
    }
    fflush(stdout);
  }
  fclose(fp);
}

