#include <stdlib.h>

#include "utils.h"
#include "log.h"
#include "types.h"

static dllist_elem *create_elem(const dllist *list, const void *data, dllist_elem *prev, dllist_elem *next);

dllist *ddlist_init(dllist *list, void *(*clone_data) (const void *), void (*free_data) (void *))
{
	ASSERT_ERROR (list, "Argument list is NULL");
	list->clone_data = clone_data;
	list->free_data = free_data;
	return list;
}

// removes all elements from end
dllist *dllist_concat(dllist *front, dllist *end)
{
	ASSERT_WARNING (front->clone_data == end->clone_data && front->free_data == end->free_data, "Lists might be of different type!");
	if (!end->head) {
		ASSERT_ERROR (!end->tail, "No head but tail!");
		goto EXIT;
	}

	if (!front->head) {
		ASSERT_ERROR (!front->tail, "No head but tail!");
		front->head = end->head;
		front->tail = end->tail;
	} else {
		ASSERT_ERROR (!front->tail->next, "front->tail->next is not NULL!");
		ASSERT_ERROR (!front->head->prev, "front->head->prev is not NULL!");
		ASSERT_ERROR (!end->head->prev, "end->head->prev is not NULL!");
		ASSERT_ERROR (!end->tail->next, "end->tail->next is not NULL!");
		front->tail->next = end->head;
		end->head->prev = front->tail;
		front->tail = end->tail;
	}
EXIT:
	end->head = NULL;
	end->tail = NULL;
	return front;
}

dllist *dllist_insert_head(dllist *list, const void *data)
{
	dllist_elem *tmp;

	if (!data)
		return list;

	ASSERT_ERROR (list, "Argument list is NULL!");

	if (!list->head) {
		ASSERT_ERROR (!list->tail, "No head but tail!");

		list->head = create_elem(list, data, NULL, NULL);
		list->tail = list->head;

		return list;
	}

	tmp = list->head;
	LOG_DEBUG ("Creating new list element %llu to list %p", dllist_size(list), list);
	list->head = create_elem(list, data, NULL, tmp);
	tmp->prev = list->head;

	return list;
}

dllist * dllist_insert_tail(dllist *list, const void *data)
{
	dllist_elem *tmp;

	if (!data)
		return list;

	ASSERT_ERROR (list, "Argument list is NULL!");

	if (!list->head) {
		dllist_insert_head(list, data);
	}

	tmp = list->tail;
	LOG_DEBUG ("Creating new list element %llu to list %p", dllist_size(list), list);
	list->tail = create_elem(list, data, tmp, NULL);
	ASSERT_ERROR (list->tail, "calloc returned NULL!");
	tmp->next = list->tail;

	return list;
}

void dllist_clear_elems(dllist *list)
{
	dllist_elem *tmp;

	if (!list)
		return;

	while (list->head) {
		tmp = list->head;
		list->head = list->head->next;
		list->free_data(tmp->data);
		free(tmp);
	}
	list->head = NULL;
	list->tail = NULL;
}

dllist * dllist_filter(dllist *list, bool (filter_fn (void *data)))
{
	dllist_elem *iter = NULL, *tmp;

	ASSERT_ERROR (list, "Argument list is NULL!");

	iter = list->head;
	while (NULL != iter) {
		if (!filter_fn(iter->data)) {
			if (iter == list->head && iter == list->tail) {
				list->head = NULL;
				list->tail = NULL;
			} else if (iter == list->head) {
				ASSERT_ERROR (list->head->next, "list->head->next is NULL, although list has more than one element!");
				list->head = list->head->next;
				list->head->prev = NULL;

			} else if (iter == list->tail) {
				ASSERT_ERROR (list->tail->prev, "list->tail->prev is NULL, although list has more than one element!");
				list->tail = list->tail->prev;
				list->tail->next = NULL;
			} else {
				ASSERT_ERROR (iter->next, "iter->next is NULL, although list has more than two element and iter is neither head nor tail!");
				ASSERT_ERROR (iter->prev, "iter->prev is NULL, although list has more than two element and iter is neither head nor tail!");
				iter->prev->next = iter->next;
				iter->next->prev = iter->prev;
			}

			tmp = iter;
			iter = iter->next;
			list->free_data(tmp->data);
			free(tmp);
		} else {
			iter = iter->next;
		}
	}
	return list;
}

bool dllist_exists(const dllist *list, bool (exists_fn (const void *data)))
{
	dllist_elem *iter;
	for (iter = list->head; iter; iter = iter->next) {
		if (exists_fn(iter->data)) {
			return true;
		}
	}
	return false;
}

u64 dllist_size(const dllist *list)
{
	dllist_elem *iter;
	u64 size = 0;
	for (iter = list->head; iter; iter = iter->next) {
		size++;
	}
	return size;
}

dllist *dllist_init(dllist *list, void *(*clone_data) (const void *), void (*free_data) (void *))
{
	ASSERT_ERROR (list, "Argument list is NULL!");
	list->clone_data = clone_data;
	list->free_data = free_data;
	list->head = NULL;
	list->tail = NULL;
	
	return list;
}

dllist *dllist_apply(dllist *list, void (*apply) (void *))
{
	dllist_elem *iter;
	for (iter = list->head; iter; iter = iter->next) {
		apply(iter->data);
	}
	return list;
}

dllist *dllist_duplicate(const dllist *list)
{
	dllist *duplicate = calloc(1, sizeof(dllist));
	ASSERT_ERROR (duplicate, "calloc returned NULL!");
	dllist_elem *list_iter;
	duplicate->clone_data = list->clone_data;
	duplicate->free_data = list->free_data;

	for (list_iter = list->head; list_iter != NULL; list_iter = list_iter->next) {
		dllist_insert_head(duplicate, duplicate->clone_data(list_iter->data));
	}
	
	return duplicate;
}

static dllist_elem *create_elem(const dllist *list, const void *data, dllist_elem *prev, dllist_elem *next)
{
	ASSERT_ERROR (list && data, "Argument list or data is NULL!");

	dllist_elem *e = calloc(1, sizeof (dllist_elem));
	ASSERT_ERROR (e, "Creating first list_elem: calloc failed!");
	e->next = next;
	e->prev = prev;
	e->data = list->clone_data(data);

	return e;
}