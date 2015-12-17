#include "NRF24Radio.h"

//Sætter hardware op.
NRF24Radio::NRF24Radio()
{
    _radio = new RF24(_cePin, _csPin);
    _radio->begin();
    _radio->setAutoAck(false);
    _radio->setDataRate(RF24_250KBPS);
    _radio->setPALevel(RF24_PA_MIN);
    _radio->setCRCLength(RF24_CRC_DISABLED);
    _radio->setChannel(_channel);
    _radio->openReadingPipe(_readingPipe, _rxAddr);
    _radio->startListening();
}

// Sender pakke ud som string.
void NRF24Radio::broadcast(char *packetAsString)
{
    memset(lastMessage, 0, 16*sizeof(char));
    memcpy(lastMessage, packetAsString, 16*sizeof(char));

    _radio->stopListening();
    _radio->write(&lastMessage, 16*sizeof(char));
}

// Lyt indtil en pakke modtages, eller tiden er gået
char *NRF24Radio::listenFor(unsigned long ms)
{
    _radio->startListening();
    unsigned long firstMs = millis();

    while(true)
    {
        if(_radio->available()) // Læs og returner data
        {
            memset(lastMessage, 0, 32);
            _radio->read(&lastMessage, 32*sizeof(char));

            return lastMessage;
        }

        // Er tiden gået?
        unsigned long thisMs = millis();
        if(thisMs-firstMs > ms)
        {
            return {defaultMessage};
        }
    }
}

// Lyt indtil en pakke er klar
char *NRF24Radio::listen(void)
{
    _radio->startListening();

    while(true)
    {
        //er der data?
        if (_radio->available()) // Læs og returner data
        {
            memset(lastMessage, 0, 32);
            _radio->read(&lastMessage, 32*sizeof(char));
            return lastMessage;
        }

        //er der modtaget signal hos noden?
        if (signalReceivedR)
        {
            printf("Listening interrupted by signal...\n");
            fflush(stdout);
            signalReceivedR = 0;
            return {defaultMessage};
        }
    }
}

