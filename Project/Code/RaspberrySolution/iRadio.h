#ifndef IRADIO_H
#define IRADIO_H
#include <signal.h>

class iRadio
{
public:
  volatile sig_atomic_t signalReceivedR;
  virtual void broadcast(char *) = 0;
  virtual char *listen() = 0;
  virtual char *listenFor(unsigned long) = 0;
};

#endif
