#include <iostream>
using namespace std;

// To execute C++, please define "int main()"
int main(int argc, char *argv[])
{
  cout << "Lol";
  return 0;
}

enum PacketType
{
  Acknowledgement,
  Request,
  Data,
  PairRequest,
  PairRequestAcknowledgement
};

class Packet
{
  public:
  PacketType type;
  Packet(int sensorData)
  {
    
  }
};

class iRadio
{
  public:
  virtual void broadcast(Packet packet);
  virtual void waitForAccept();
  virtual void broadcastRequest();
};

class iSensor
{
  public:
  virtual int read() = 0;
};



class PacketHandler
{
  
  private:
    static iRadio  *radio;
    static iSensor *sensor;
    static bool waitForAcknowledgement;
    static bool readyToForward;
  
  
  
  static void determineAction(Packet packet)
  {
    switch(packet.type)
    {
      case Acknowledgement : 
      {
          if (waitForAcknowledgement)
          {
            waitForAcknowledgement = false;
            readyToForward         = true;
          }
      }
      break;
        
      case Request :
      {
          if(!waitForAcknowledgement)
          {
            readPackSend();
            waitForAcknowledgement = true;
          }
          requestRecieved();
      }
      break;
        
      case Data :
      {
          if (readyToForward)
          {
            forwardSignal();
          }
      }
      break;
        
      case PairRequest :
        // Wanna be a pair..
      break;
        
      case PairRequestAcknowledgement :
        // .. OF SOCKS!
      break;
        
      default:
          cout << "FUCK";
      break;
    }
  }
  
    // ACTIONS //
  static void readPackSend()
  {
    int sensorData = sensor->read();     //Read from sensor 
    Packet packet = Packet(sensorData); //Pack data
    radio->broadcast(packet);            //Send data
    
    radio->waitForAccept();            //Wait for accept here?
    
  }
  
  static void sendRequest()
  {
    radio->broadcastRequest();
  }
  
  static void forwardSignal(Packet forwardData)
  {
    radio->broadcast(forwardData);
  }
  
  static void requestRecieved()
  {
    
  }
  static void forwardSignal()
  {
    
  }
};