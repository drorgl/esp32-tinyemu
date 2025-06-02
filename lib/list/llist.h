
//
// list.h
//
// Copyright (c) 2010 TJ Holowaychuk <tj@vision-media.ca>
//

#ifndef __CLIBS_LIST_H__
#define __CLIBS_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

// Library version

#define LIST_VERSION "0.0.5"

// Memory management macros
#ifdef LIST_CONFIG_H
#define _STR(x) #x
#define STR(x) _STR(x)
#include STR(LIST_CONFIG_H)
#undef _STR
#undef STR
#endif

#ifndef LIST_MALLOC
#define LIST_MALLOC malloc
#endif

#ifndef LIST_FREE
#define LIST_FREE free
#endif

/*
 * list_t iterator direction.
 */

typedef enum {
    LIST_HEAD
  , LIST_TAIL
} list_direction_t;

/*
 * list_t node struct.
 */

typedef struct list_node {
  struct list_node *prev;
  struct list_node *next;
  void *val;
} list_node_t;

/*
 * list_t struct.
 */

typedef struct {
  list_node_t *head;
  list_node_t *tail;
  unsigned int len;
  void (*free)(void *val);
  int (*match)(void *a, void *b);
} list_t;

/*
 * list_t iterator struct.
 */

typedef struct {
  list_node_t *next;
  list_direction_t direction;
} list_iterator_t;

// Node prototypes.

list_node_t *
list_node_new(void *val);

// list_t prototypes.


/*
 * Allocate and initialize a `list`.
 */
list_t *
list_new(void);

/*
 * Append `node` to `self`, returning `node`.
 */
list_node_t *
list_rpush(list_t *self, list_node_t *node);

/*
 * Prepend `node` to `self`, returning `node`.
 */
list_node_t *
list_lpush(list_t *self, list_node_t *node);

/*
 * Return the `list_node_t` containing `val` or NULL.
 */
list_node_t *
list_find(list_t *self, void *val);

/*
 * Return the `list_node_t` at the given `index`, where `index`
 * may also be a negative integer indicating an index from the
 * list `tail`.
 */
list_node_t *
list_at(list_t *self, int index);

/*
 * Return / detach node from the end of the list, or NULL.
 */
list_node_t *
list_rpop(list_t *self);

/*
 * Return / detach node from the start of the list, or NULL.
 */
list_node_t *
list_lpop(list_t *self);

/*
 * Remove `node` from the list, freeing it and it's value.
 */
void
list_remove(list_t *self, list_node_t *node);

/*
 * Free the list and all nodes.
 */
void
list_destroy(list_t *self);

// list_t iterator prototypes.

/*
 * Allocate and initialize a `list_iterator_t` with the given
 * `direction`, where `direction` may be `LIST_HEAD` or `LIST_TAIL`.
 */
list_iterator_t *
list_iterator_new(list_t *list, list_direction_t direction);

/*
 * Allocate and initialize a `list_iterator_t` with the given start
 * node and `direction`, where `direction` may be `LIST_HEAD` or `LIST_TAIL`.
 */
list_iterator_t *
list_iterator_new_from_node(list_node_t *node, list_direction_t direction);

/*
 * Return the next `list_node_t` or NULL.
 */
list_node_t *
list_iterator_next(list_iterator_t *self);

/*
 * Free the iterator only.
 */
void
list_iterator_destroy(list_iterator_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __CLIBS_LIST_H__ */
