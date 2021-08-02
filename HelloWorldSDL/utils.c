#include <stdlib.h>

#include "utils.h"
#include "log.h"
#include "types.h"

u64 dllist_destroy_r(dllist *list, u64 cnt, bool free_data);

// TODO: implement dllist_concat!

dllist ** dllist_insert_last(dllist **list, void *data)
{
	u64 i = 0;
	dllist *tmp;

	if (!data)
		return list;

	ASSERT_ERROR (list, "Argument list is NULL!");
	if (!*list) {

		*list = calloc(1, sizeof (dllist));

		ASSERT_ERROR (*list, "Creating new list failed");
		LOG_DEBUG ("Creating new list at address %p", list);

		(*list)->data = data;

		return list;
	}

	tmp = *list;

	while (tmp->next) {
		tmp = tmp->next;
		++i;
	}

	tmp->next = calloc(1, sizeof (dllist));
	ASSERT_ERROR (tmp->next, "Creating new list element failed");
	LOG_DEBUG ("Creating new list element nr %u to list %p", i, list);

	tmp->next->data = data;

	return list;
}

void dllist_destroy(dllist *list, bool free_data)
{
	u64 depth;
	if (!list) {
		return;
	}
	LOG_DEBUG ("Destroying list at address %p %s", list, free_data?"and freeing data":"");
	depth = dllist_destroy_r(list, 0, free_data);

	LOG_DEBUG ("Freed %u list elements", depth);
}

u64 dllist_destroy_r(dllist *list, u64 cnt, bool free_data)
{
	u64 depth = 0;
	ASSERT_WARNING (cnt == 100, "Reached recursive iteration 100");
	if (list->next) depth = dllist_destroy_r(list->next, 1 + cnt, free_data);
	if (free_data)
		free(list->data);
	free(list);

	return depth + cnt;
}

dllist * dllist_filter(dllist *list, bool (filter_fn (void *data)))
{
	dllist *tmp;
	while (list) {
		if (!filter_fn(list)) {
			tmp = list;
			if (list->next)
				list->next->prev = list->prev;
			if (list->prev)
				list->prev->next = list->next;
		}
		list = list->next;
	}
	return list;
}

bool dllist_exists(const dllist *list, bool (exists_fn (const void *data)))
{
	while (list) {
		if (exists_fn(list)) {
			return true;
		}
		list = list->next;
	}
	return false;
}

u64 dllist_size(const dllist *list)
{
	u64 size = 0;
	while (list) {
		size++;
		list = list->next;
	}
	return size;
}