#include <stdlib.h>

#include "utils.h"
#include "log.h"
#include "types.h"

u64 llist_destroy_r(llist *list, u64 cnt, bool free_data);

llist *llist_insert_last(llist *list, void *data)
{
	u64 i = 0;
	if (!list) {

		list = calloc(1, sizeof (llist));

		ASSERT_ERROR (list, "Creating new list failed");
		LOG_DEBUG ("Creating new list at address %p", list);

		list->data = data;

		return list;
	}

	while (list->next) {
		list = list->next;
		++i;
	}

	list->next = calloc(1, sizeof (llist));
	ASSERT_ERROR (list->next, "Creating new list element failed");
	LOG_DEBUG ("Creating new list element nr %u to list %p", i, list);

	list->next->data = data;

	return list;
}

void llist_destroy(llist *list, bool free_data)
{
	u64 depth;
	if (!list) {
		LOG_WARNING ("list is NULL");
		return;
	}
	LOG_DEBUG ("Destroying list at address %p %s", list, free_data?"and freeing data":"");
	depth = llist_destroy_r(list, 0, free_data);

	LOG_DEBUG ("Freed %u list elements", depth);
}

u64 llist_destroy_r(llist *list, u64 cnt, bool free_data)
{
	u64 depth = 0;
	ASSERT_WARNING (cnt == 100, "Reached recursive iteration 100");
	if (list->next) depth = llist_destroy_r(list->next, 1 + cnt, free_data);
	if (free_data)
		free(list->data);
	free(list);

	return depth + cnt;
}