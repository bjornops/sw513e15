class iRadio
{
public:
  virtual void broadcast(char *);
  virtual char *listen();
  virtual char *listenFor(unsigned long);
};

