AVL-tree
--------
A high performance generic AVL-tree container C implementation.

It can be used as a set or a map, containing any type of data.

Author
------
Jung-Sang Ahn <jungsang.ahn@gmail.com>

How to use
----------
Below example describes how to use AVL-tree as an ordered map of integer pairs.

We define a node for an integer pair, and a comparison function of given two nodes:

```C
#include "avltree.h"

struct kv_node{
    struct avl_node avl;
    // put your data here
    int key;
    int value;
};

int cmp_func(struct avl_node *a, struct avl_node *b, void *aux)
{
    struct kv_node *aa, *bb;
    aa = _get_entry(a, struct kv_node, avl);
    bb = _get_entry(b, struct kv_node, avl);

    if (aa->key < bb->key)
        return -1;
    else if (aa->key > bb->key)
        return 1;
    else
        return 0;
}
```

Example code:

* Initialize tree
```C
struct avl_tree tree;

avl_init(&tree, NULL);
```

* Insert ```{1, 10}``` pair
```C
struct kv_node *node;

node = (struct kv_node*)malloc(sizeof(struct kv_node));

node->key = 1;
node->value = 10;
avl_insert(&tree, &node->avl, cmp_func);
```
* Insert ```{2, 20}``` pair
```C
struct kv_node *node;

node = (struct kv_node*)malloc(sizeof(struct kv_node));

node->key = 2;
node->value = 20;
avl_insert(&tree, &node->avl, cmp_func);
```
* Find the value corresponding to key `1`
```C
struct kv_node query;
struct kv_node *node;
struct avl_node *cursor;

query.key = 1;
cursor = avl_search(&tree, &query.avl, cmp_func);

// get 'node' from 'cursor'
node = _get_entry(cursor, struct kv_node, avl);
printf("%d\n", node->value);    // it will display 10
```
* Iteration
```C
struct kv_node *node;
struct avl_node *cursor;

cursor = avl_first(&tree);
while (cursor) {
    // get 'node' from 'cursor'
    node = _get_entry(cursor, struct kv_node, avl);

    // ... do something with 'node' ...

    // get next cursor
    cursor = avl_next(cursor);
}
```
* Remove the key-value pair corresponding to key `1`
```C
struct kv_node query;
struct kv_node *node;
struct avl_node *cursor;

query.key = 1;
cursor = avl_search(&tree, &query.avl, cmp_func);
if (cursor) {
    // get 'node' from 'cursor'
    node = _get_entry(cursor, struct kv_node, avl);
    // remove from tree
    avl_remove(&tree, cursor);
    // free 'node'
    free(node);
}
```

