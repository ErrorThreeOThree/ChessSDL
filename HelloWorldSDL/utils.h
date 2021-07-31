#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

typedef struct llist {
	void *data;
	struct llist *next;
} llist;

llist *llist_insert_last(llist *list, void *data);

void llist_destroy(llist *list, bool free_data);
#endif