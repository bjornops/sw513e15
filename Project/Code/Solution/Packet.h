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
    
    Packet(int);
};