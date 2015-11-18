#ifndef IRADIO_H
#define IRADIO_H

class iRadio
{
public:
  virtual void broadcast(char *);
  virtual char *listen();
  virtual char *listenFor(unsigned long);
};

#endif
