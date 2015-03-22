#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "common.h"

extern char *optarg;

int main(int argc, char *argv[])
{
	int sock;
	uint16_t port, prot;
	uint32_t number;
	char c;
	char buffer[4];
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
	addr.sin_port = htons((uint16_t)port);
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
	for(; read(fileno(stdin), &c, 1); )
	{
		if(prot == 1)
		{
			if(sendall(sock, &c, 1, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if(c == '\\' && sendall(sock, "\\", 1, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if( sendall(sock, "\\0", 2, 0) < 0 ||
				recvexact(sock, buffer, 2, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if(buffer[0] == '\\')
			{
				if(buffer[1] == '\\')
				{
					write(fileno(stdout), "\\", 1);
					if(recvexact(sock, buffer, 2, 0) < 0)
					{
						fprintf(stderr, "server fault: socket\n");
						exit(1);
					}
				}
				else if(buffer[1] != '0')
				{
					fprintf(stderr, "server fault: escape\n");
					exit(1);
				}
			}
			else
			{
				write(fileno(stdout), buffer, 1);
				if(recvexact(sock, buffer, 1, 0) < 0)
				{
					fprintf(stderr, "server fault: socket\n");
					exit(1);
				}
			}
		}
		if(prot == 2)
		{
			number = htonl(1);
			if( sendall(sock, &number, 4, 0) < 0 ||
				sendall(sock, &c, 1, 0) < 0 ||
				recvexact(sock, &number, 4, 0) < 0)
			{
				fprintf(stderr, "server fault: socket\n");
				exit(1);
			}
			if(ntohl(number))
			{
				if(recvexact(sock, buffer, 1, 0) < 0)
				{
					fprintf(stderr, "server fault: socket\n");
					exit(1);
				}
				write(fileno(stdout), buffer, 1);
			}
		}
	}
	close(sock);
	return 0;
}
