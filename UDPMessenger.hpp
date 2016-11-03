#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define BIT_(x) 	1 << x
#define UDP_MAX_SIZE 	65507
#define CHUNK_HDR_SIZE	3

//Chunk protocol flags
#define F_INIT_CHUNKING	BIT_(0)
#define F_CHUNK_NUM 	BIT_(1)
#define F_END_CHUNKING	BIT_(2)

/**
 * @brief UDP messenger class for simple UDP communications. Doesn't do
 * anything fancy.
 *
 * UDPMessenger uses a protocol designed to chunk messages into smaller
 * sizes based on the user's judgement of how small each packet should be.
 * This protocol is NOT particularly resistant to packet loss. It is 
 * designed to be fast.
 *
 * @verbatim
 * -------------------------------
 * | Flags | Reserved | Data ... |
 * -------------------------------
 * B:  1        2       1-65504
 *
 * Flags:
 * F_INIT_CHUNKING	-Begin chunking, chunk total in reserved, data follows
 * F_CHUNK_NUM		-Chunk number in reserved, data follows
 * F_END_CHUNKING	-End chunking, final chunk in data
 * @endverbatim
 */
class UDPMessenger
{
public:
	UDPMessenger(const char* addr, const uint16_t port);
	~UDPMessenger();
	
	//Functions for sending
	void setDest(const char* dest, const uint16_t port);
	void sendPacket(const char* dest, const uint16_t port, const char* msg, const uint16_t msglen);
	void sendPacket(const char* msg, const uint16_t msglen);
	void sendChunks(const char* msg, const uint32_t msglen, int chunks, const int delayUS = 0);
	void reply(const char* msg, const uint16_t msglen);

	//Functions for recieving
	int getRecvSize();
	uint16_t getMsg(char* buffer, const uint16_t bufferlen);
	char* getChunks(uint32_t* bufferlen);
	sockaddr getLastSender();
	

protected:
	uint16_t udpsocket;
	uint16_t port;
	sockaddr_in destAddr;
	sockaddr lastSender;
	socklen_t lastSenderSize;
};

