#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "common.h"

#define BUFSIZE (0x1000)

extern char *optarg;

void serve(int sock, int prot)
{
	int length, i, j;
	uint32_t number;
	char buffer[BUFSIZE];

	for(; 0 < (length = read(fileno(stdin), &buffer, BUFSIZE)); )
	{
		if(prot == 1)
		{
			for(i = 0; i < length; i++)
			{
				for(j = i; j < length && buffer[j] != '\\'; j++);
				if(sendall(sock, buffer + i, j - i, 0) < 0 || (j < length && sendall(sock, "\\\\", 2, 0) < 0))
				{
					fprintf(stderr, "server fault: socket\n");
					exit(1);
				}
				i = j;
			}
			if(sendall(sock, "\\0", 2, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			for(length = 0; 0 <= (i = recvexact(sock, buffer + length, 1, 0)); length++)
			{
				if(buffer[length] == '\\')
				{
					if(recvexact(sock, buffer + length + 1, 1, 0) < 0)
					{
						fprintf(stderr, "server fault: socket\n");
						exit(1);
					}
					if(buffer[length + 1] == '0')
					{
						break;
					}
					else if(buffer[length + 1] != '\\')
					{
						fprintf(stderr, "server fault: escape\n");
						exit(1);
					}
				}
				if(write(fileno(stdout), buffer + length, 1) < 0)
				{
					perror("write");
					exit(1);
				}
			}
			if(i < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
		}
		else
		{
			number = htonl(length);
			if(sendall(sock, &number, 4, 0) < 0 || sendall(sock, buffer, length, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if(recvexact(sock, &number, 4, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			length = ntohl(number);
			if(recvexact(sock, buffer, length, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if(write(fileno(stdout), buffer, length) < 0)
			{
				perror("write");
				exit(1);
			}
		}
	}
	if(length < 0)
	{
		perror("read");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	int sock;
	uint16_t port, prot;
	char c;
	char *argv_host = NULL, *argv_port = NULL, *argv_prot = NULL, *host;
	struct sockaddr_in addr;
	struct neghdr neg;

	while((c = getopt(argc, argv, "h:p:m:")) != -1)
	{
		switch(c)
		{
			case 'h':
				argv_host = optarg;
				host = argv_host;
				break;
			case 'p':
				argv_port = optarg;
				port = strtoul(argv_port, NULL, 10);
				break;
			case 'm':
				argv_prot = optarg;
				prot = strtoul(argv_prot, NULL, 10);
				break;
		}
	}
	if(argv_host == NULL || argv_port == NULL || argv_prot == NULL)
	{
		fprintf(stderr, "usage: %s -h host -p port -m protocol\n", argv[0]);
		exit(1);
	}
	if(prot != 1 && prot != 2)
	{
		fprintf(stderr, "wrong argument: protocol\n");
		exit(1);
	}

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host);
	if(connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("connect");
		exit(1);
	}
	if(recvexact(sock, &neg, sizeof(struct neghdr), 0) < 0)
	{
		fprintf(stderr, "server fault: socket\n");
		exit(1);
	}
	if(ntohs(neg.op) != 0 || ntohs(neg.proto) != 0)
	{
		fprintf(stderr, "server fault: protocol\n");
		exit(1);
	}
	if(checksum(&neg, sizeof(struct neghdr)) != 0xffff)
	{
		fprintf(stderr, "server fault: checksum\n");
		exit(1);
	}
	neg.op = 1;
	neg.proto = prot;
	neg.checksum = 0;
	neg.checksum = ~checksum(&neg, sizeof(struct neghdr));
	if(sendall(sock, &neg, sizeof(struct neghdr), 0) < 0)
	{
		fprintf(stderr, "server fault: socket\n");
		exit(1);
	}

	serve(sock, prot);
	close(sock);

	return 0;
}
