// common.h

#include <stdint.h>
#include <stdlib.h>

int recvexact(int socket, void *buffer, size_t length, int flags);
int sendall(int socket, void *buffer, size_t length, int flags);
int recvmessage(int socket, void *buffer, size_t length, int prot);
int sendmessage(int socket, void *buffer, size_t length, int prot);
uint16_t checksum(const void *buffer, size_t length);


struct neghdr
{
	uint8_t op;
	uint8_t proto;
	uint16_t checksum;
	uint32_t trans_id;
};
