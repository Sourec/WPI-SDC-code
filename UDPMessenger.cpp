#include "UDPMessenger.hpp"
#include <stdio.h>

/**
 * @brief Initialize a new UDP messenger with a given address and port.
 *
 * @param addr Address to bind to as a string.
 * @param port Port number to bind to.
 *
 * @todo Throw exception upon socket/bind failure.
 */
UDPMessenger::UDPMessenger(const char* addr, const uint16_t port)
{
	udpsocket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	
	sockaddr_in baddr;
	memset((char*)&baddr, 0, sizeof(baddr));
	inet_pton(AF_INET, addr, &(baddr.sin_addr));
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(port);	

	if (bind(udpsocket, (sockaddr*)&baddr, sizeof(baddr)) < 0)
	{
		printf("Failed to bind\n");
		close(udpsocket);
	}

	memset((char*)&destAddr, 0, sizeof(destAddr));
	memset((char*)&lastSender, 0, sizeof(lastSender));
	lastSenderSize = sizeof(lastSender);
}

/**
 * @brief Destroy this UDPMessenger and close its socket.
 */
UDPMessenger::~UDPMessenger()
{
	close(udpsocket);
}

/**
 * @brief Sets the destination to send *to* so it can be
 * sent packets in the future easier.
 *
 * @param dest Destination server IP address as a string
 * @param port Port on the recieving server.
 */
void UDPMessenger::setDest(const char* dest, const uint16_t port)
{
	memset((char*)&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	inet_pton(AF_INET, dest, &destAddr.sin_addr);
}

/**
 * @brief Send a UDP packet to a specified IP address with a specified port.
 * Note that it also saves the address so that the other variation of sendPacket()
 * may be called next time.
 *
 * @param dest Destination IP as a string
 * @param port Destination port
 * @param msg Message to send. Max limit of 65507 bytes.
 * @param msglen Length of message
 */
void UDPMessenger::sendPacket(const char* dest, const uint16_t port, const char* msg, const uint16_t msglen)
{
	setDest(dest, port);
	sendto(udpsocket, msg, msglen, 0, (sockaddr*)&destAddr, sizeof(destAddr));
}

/**
 * @brief Send a UDP packet to the saved destination.
 *
 * @param msg Message to send. Max limit of 65507 bytes.
 * @param msglen Length of message
 *
 * @note If you haven't previously called setDest or specified a destination/
 * port in some way, this function may not act properly.
 */
void UDPMessenger::sendPacket(const char* msg, const uint16_t msglen)
{
	sendto(udpsocket, msg, msglen, 0, (sockaddr*)&destAddr, sizeof(destAddr));
}

/**
 * @brief Send a message in X number of chunks. Uses custom protocol to
 * coordnate sending
 *
 * @param msg Message to send. Arbitrary size.
 * @param msglen Length of the message. Arbitrary size.
 * @param chunks The number of chunks to send. If the number of chunks is too
 * low, the lowest possible number of chunks will be chosen for you instead.
 * @param delayUS The delay between each chunk in us (microseconds, 1/1000 of a millisecond)
 */
void UDPMessenger::sendChunks(const char* msg, const uint32_t msglen, int chunks, const int delayUS)
{
	//This function first determines the size that each chunk should
	//be. Since the data size will virtually never fit evenly into the
	//chunks, the last chunk has the size of msglen % chunks and all other
	//chunks have size floor(msg/chunks).
	//
	//Stages follow:
	//1. Send BEGIN message. Retransmit if response not recieved within
	//   timeout. (todo)
	//2. Continue sending appropriately numbered chunks until the
	//   chunk-1th packet. Watch for any responses from the server requesting
	//   retransmission.
	//3. Send the last chunk in a BIT_7-flagged packet (end chunking). If
	//   response not recieved within timeout, resend.
	
	//Prep - define constants needed
	if (msglen / chunks > UDP_MAX_SIZE - CHUNK_HDR_SIZE)
		chunks = ceil((float)msglen / (float)UDP_MAX_SIZE); //Idiot check
	const uint16_t chunkSize = floor(msglen / chunks);
	const int lastChunkSize = (msglen % chunks) + chunkSize;

	//Stage 1: begin chunking
	//Below is used throughout the function. Realloc'd once, freed at the end.
	char* packet = (char*)malloc(CHUNK_HDR_SIZE + chunkSize);

	//Send beginning packet
	packet[0] = F_INIT_CHUNKING;
	memcpy(packet+1, &chunks, 2);
	memcpy(packet + CHUNK_HDR_SIZE, msg, chunkSize);
	sendPacket(packet, CHUNK_HDR_SIZE + chunkSize);

	//Stage 2: Send chunks 2-(chunks-1)
	for (uint16_t i = 2; i < chunks; i++)
	{
		packet[0] = F_CHUNK_NUM;
		memcpy(packet + 1, &i, sizeof(i)); 
		memcpy(packet + CHUNK_HDR_SIZE, msg + ((i-1) * chunkSize), chunkSize);
		sendPacket(packet, CHUNK_HDR_SIZE + chunkSize);
		usleep(delayUS); //us, microseconds! 1/1000 of a ms
	}

	//Stage 3: Send final chunk
	//First resize the packet array though, it needs to fit the final (larger) chunk
	packet = (char*)realloc(packet, CHUNK_HDR_SIZE + lastChunkSize);

	packet[0] = F_END_CHUNKING;
	memcpy(packet + 1, &chunks, 2);
	memcpy(packet + 3, msg + (msglen - lastChunkSize), lastChunkSize);
	sendPacket(packet, CHUNK_HDR_SIZE+ lastChunkSize );
	
	free(packet);
}

/**
 * @brief Reply to the sender of the last recieved packet.
 *
 * @param msg Message to send. Max limit of 65507 bytes.
 * @param msglen Length of message.
 * @note Assumes that a packet has already been recieved. If one hasn't, this
 * will send a packet to an arbitrary IP and port... something like that.
 */
void UDPMessenger::reply(const char* msg, const uint16_t msglen)
{
	sendto(udpsocket, msg, msglen, 0, &lastSender, sizeof(lastSender));
}

/**
 * @brief Gets the size of the last message recieved, but does NOT read from it.
 *
 * @return Size of message in bytes. -1 if there's no packet.
 */
int UDPMessenger::getRecvSize()
{
	//Discards actual data but leaves it in place so it can still be read
	//from. This way, whatever is recieving can allocate an appropriately-
	//sized buffer.
	return recv(udpsocket, nullptr, 0, MSG_PEEK);
}

/**
 * @brief Gets last data recieved and saves the sender's address information.
 *
 * @param buffer Buffer to place recieved data in.
 * @param bufferlen Length of buffer.
 *
 * @return The size of the packet recieved.
 */
uint16_t UDPMessenger::getMsg(char* buffer, const uint16_t bufferlen)
{
	return recvfrom(udpsocket, buffer, bufferlen, 0, &lastSender, &lastSenderSize);
}

/**
 * @brief Recieve a message in chunk form.
 *
 * @param bufferlen Pointer to an int that the length of the returned buffer
 * will be stored in.
 *
 * @return A buffer containing the recieved message, pieced together from
 * recieved chunks. It will need to be freed when no longer used.
 *
 * @note This function will BLOCK until chunks are actually recieved!
 * @todo Make this function NOT block, either by timeout or by message peeking
 */
char* UDPMessenger::getChunks(uint32_t* bufferlen)
{
	//Actual incoming packet size is unknown, so get ready for the
	//biggest possible packet.
	char packet[UDP_MAX_SIZE];
	uint16_t packetLen = 0;
	while (true)
	{
		//Keep checking for chunk packets until one is recieved
		while(getRecvSize() == -1);
		packetLen = getMsg(packet, UDP_MAX_SIZE);
		if (packet[0] & F_INIT_CHUNKING)
			break;
	}
	const uint16_t chunkSize = packetLen - CHUNK_HDR_SIZE;
	uint16_t totalChunks = 0;
	memcpy(&totalChunks, packet + 1, sizeof(totalChunks));
	
	char* databuffer = (char*)malloc(totalChunks * chunkSize); //will need to be expanded for last packet
	memset(databuffer, 0, totalChunks * chunkSize);
	memcpy(databuffer, packet + CHUNK_HDR_SIZE, chunkSize);

	while (true) //todo: kill
	{
		packetLen = getMsg(packet, UDP_MAX_SIZE);
		if (packet[0] & F_END_CHUNKING)
			break;

		uint16_t chunkNum = 0;
		memcpy(&chunkNum, packet + 1, sizeof(chunkNum)); //extract chunk number
		memcpy(databuffer + ((chunkNum - 1) * chunkSize), packet + CHUNK_HDR_SIZE, chunkSize);
	}

	//process last packet, already ready for processing
	const uint16_t lastChunkSize = packetLen - CHUNK_HDR_SIZE;
	databuffer = (char*)realloc(databuffer, (totalChunks - 1) * chunkSize + lastChunkSize);
	memcpy(databuffer + ((totalChunks -1) * chunkSize), packet + CHUNK_HDR_SIZE, lastChunkSize);

	*bufferlen = ((totalChunks - 1) * chunkSize) + lastChunkSize;
	return databuffer;
}	

/**
 * @brief Returns the address of the last sender, based on the last data recieved.
 *
 * @return A sockaddr struct containing relevant info - cast to sockaddr_in if
 * necessary.
 */
sockaddr UDPMessenger::getLastSender()
{
	return lastSender;
}
