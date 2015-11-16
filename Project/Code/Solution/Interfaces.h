class iRadio
{
public:
  virtual void broadcast(char *);
  virtual char *listen();
  virtual char *listenFor(unsigned long);
};

class iSensor
{
public:
  virtual int read() = 0;
};
