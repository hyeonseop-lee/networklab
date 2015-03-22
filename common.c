#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

int recvexact(int socket, void *buffer, size_t length, int flags)
{
	int now, result;
	for(now = 0; now < length; )
	{
		if((result = recv(socket, buffer + now, length, flags)) < 0)
		{
			perror("recv");
			exit(1);
		}
		if(result == 0)
		{
			return -1;
		}
		now += result;
	}
	return 0;
}

int sendall(int socket, void *buffer, size_t length, int flags)
{
	int now, result;
	for(now = 0; now < length; )
	{
		if((result = send(socket, buffer + now, length, flags)) < 0)
		{
			perror("send");
			exit(1);
		}
		if(result == 0)
		{
			return -1;
		}
		now += result;
	}
	return 0;
}

uint16_t checksum(const void *buffer, size_t length)
{
	uint32_t sum;
	const uint16_t *ptr;
	for(ptr = buffer, sum = 0; length; ptr++, length -= 2)
	{
		sum += *ptr;
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (uint16_t)sum;
}
