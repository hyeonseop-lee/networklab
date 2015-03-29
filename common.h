// common.h

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

int recvexact(int socket, void *buffer, size_t length, int flags);
int sendall(int socket, void *buffer, size_t length, int flags);
uint16_t checksum(const void *buffer, size_t length);

struct neghdr
{
	uint8_t op;
	uint8_t proto;
	uint16_t checksum;
	uint32_t trans_id;
};

struct list_elem
{
	struct list_elem *prev;
	struct list_elem *next;
};

struct list
{
	struct list_elem head;
	struct list_elem tail;
};

#define list_entry(elem, type, memb) ((type *)(((uint8_t *)elem) - offsetof(type, memb)))

void list_init(struct list *list);
void list_insert(struct list *list, struct list_elem *elem);
void list_remove(struct list_elem *elem);
struct list_elem *list_begin(struct list *list);
struct list_elem *list_end(struct list *list);
struct list_elem *list_next(struct list_elem *elem);
struct list_elem *list_prev(struct list_elem *elem);
