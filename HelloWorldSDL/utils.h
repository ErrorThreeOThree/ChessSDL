#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "types.h"

typedef struct dllist {
	void *data;
	struct dllist *next;
	struct dllist *prev;
} dllist;

dllist ** dllist_insert_last(dllist **list, void *data);

void dllist_destroy(dllist *list, bool free_data);

dllist *dllist_filter(dllist *list, bool (filter_fn (void *data)));

bool dllist_exists(const dllist *list, bool (exists_fn (const void *data)));

u64 dllist_size(const dllist *list);

#endif