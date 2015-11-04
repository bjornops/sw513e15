enum PacketType
{
  Acknowledgement,
  Request,
  Data,
  PairRequest,
  PairRequestAcknowledgement,
  ClearSignal
};

class Packet
{
public:
    PacketType type;
    
    Packet(int);
};