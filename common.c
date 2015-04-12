#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"

int recvexact(int socket, void *buffer, size_t length, int flags)
{
	int now, result;
	for(now = 0; now < length; )
	{
		if((result = recv(socket, buffer + now, length - now, flags)) < 0)
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
		if((result = send(socket, buffer + now, length - now, flags)) < 0)
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

void list_init(struct list *list)
{
	list->head.prev = NULL;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
	list->tail.next = NULL;
}

void list_insert(struct list *list, struct list_elem *elem)
{
	struct list_elem *prev = list->tail.prev, *next = &list->tail;
	prev->next = elem;
	elem->prev = prev;
	elem->next = next;
	next->prev = elem;
}

void list_remove(struct list_elem *elem)
{
	struct list_elem *prev = elem->prev, *next = elem->next;
	prev->next = next;
	next->prev = prev;
}

struct list_elem *list_begin(struct list *list)
{
	return list->head.next;
}

struct list_elem *list_end(struct list *list)
{
	return &list->tail;
}

struct list_elem *list_next(struct list_elem *elem)
{
	return elem->next;
}

struct list_elem *list_prev(struct list_elem *elem)
{
	return elem->prev;
}
