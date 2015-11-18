#ifndef IRADIO_H
#define IRADIO_H

class iRadio
{
public:
  virtual void broadcast(char *) = 0;
  virtual char *listen() = 0;
  virtual char *listenFor(unsigned long) = 0;
};

#endif
