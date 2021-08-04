#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#include "types.h"

typedef struct dllist_elem dllist_elem;

typedef struct dllist {
	dllist_elem *head;
	dllist_elem *tail;

	void *(*clone_data) (const void *);
	void (*free_data) (void *);
} dllist;

dllist *dllist_init(dllist *list, void *(*clone_data) (const void *), void (*free_data) (void *));

dllist *dllist_insert_tail(dllist *list, void *data);

void dllist_clear_elems(dllist *list);

dllist *dllist_filter(dllist *list, bool (filter_fn (void *data)));

bool dllist_exists(const dllist *list, bool (exists_fn (const void *data)));

u64 dllist_size(const dllist *list);

dllist *dllist_concat(dllist *front, dllist *end);

dllist *dllist_insert_head(dllist *list, void *data);

dllist *dllist_apply(dllist *list, void (*apply) (void *));

#endif