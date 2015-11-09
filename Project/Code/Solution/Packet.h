enum PacketType : uint8_t {
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
	uint16_t addresser;
	uint16_t addressee;
	uint16_t origin;
	uint16_t sensor1;
	uint16_t sensor2;
	uint16_t sensor3;
	uint16_t checksum;
	PacketType packetType;
	Packet(string);
	Packet(PacketType, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
	string encode();

private:
	void decode(string);
	uint16_t getChecksum();
};