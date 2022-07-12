#include <unity.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <llist.h>

#include <runner.h>

// Helpers

void setUp(){}
void tearDown(){}

#define test(fn) \
  puts("... \x1b[33m" # fn "\x1b[0m"); \
  test_##fn();

static int freeProxyCalls = 0;

void
freeProxy(void *val) {
  ++freeProxyCalls;
  free(val);
}

typedef struct {
  char *name;
} User;

static int
User_equal(User *a, User *b) {
  return 0 == strcmp(a->name, b->name);
}

// Tests

void
test_list_node_new() {
  char *val = "some value";
  list_node_t *node = list_node_new(val);
  TEST_ASSERT_EQUAL(node->val, val);
  free(node);
}

void
test_list_rpush() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_node_new("a");
  list_node_t *b = list_node_new("b");
  list_node_t *c = list_node_new("c");

  // a b c
  list_rpush(list, a);
  list_rpush(list, b);
  list_rpush(list, c);

  // Assertions
  TEST_ASSERT_EQUAL_STRING(a->val , list->head->val);
  TEST_ASSERT_EQUAL_STRING(c->val , list->tail->val);
  TEST_ASSERT_EQUAL(3 , list->len);
  TEST_ASSERT_EQUAL_STRING(b->val , a->next->val);
  TEST_ASSERT_EQUAL(NULL , a->prev);
  TEST_ASSERT_EQUAL_STRING(c->val , b->next->val);
  TEST_ASSERT_EQUAL_STRING(a->val , b->prev->val);
  TEST_ASSERT_EQUAL(NULL , c->next);
  TEST_ASSERT_EQUAL_STRING(b->val , c->prev->val);

  list_destroy(list);
}

void
test_list_lpush() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_node_new("a");
  list_node_t *b = list_node_new("b");
  list_node_t *c = list_node_new("c");

  // c b a
  list_rpush(list, a);
  list_lpush(list, b);
  list_lpush(list, c);

  // Assertions
  TEST_ASSERT_EQUAL(c->val , list->head->val);
  TEST_ASSERT_EQUAL(a->val , list->tail->val);
  TEST_ASSERT_EQUAL(3 , list->len);
  TEST_ASSERT_EQUAL(NULL , a->next);
  TEST_ASSERT_EQUAL(b->val , a->prev->val);
  TEST_ASSERT_EQUAL(a->val , b->next->val);
  TEST_ASSERT_EQUAL(c->val , b->prev->val);
  TEST_ASSERT_EQUAL(b->val , c->next->val);
  TEST_ASSERT_EQUAL(NULL , c->prev);

  list_destroy(list);
}

void
test_list_lpush_only() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_node_new("a");
  list_node_t *b = list_node_new("b");
  list_node_t *c = list_node_new("c");

  // c b a
  list_lpush(list, a);
  list_lpush(list, b);
  list_lpush(list, c);

  // Assertions
  TEST_ASSERT_EQUAL(c , list->head);
  TEST_ASSERT_EQUAL(a , list->tail);
  TEST_ASSERT_EQUAL(3 , list->len);
  TEST_ASSERT_EQUAL(NULL , a->next);
  TEST_ASSERT_EQUAL(b , a->prev);
  TEST_ASSERT_EQUAL(a , b->next);
  TEST_ASSERT_EQUAL(c , b->prev);
  TEST_ASSERT_EQUAL(b , c->next);
  TEST_ASSERT_EQUAL(NULL , c->prev);

  list_destroy(list);
}


void
test_list_lpush_existing() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_node_new("a");
  list_node_t *b = list_node_new("b");
  list_node_t *c = list_node_new("c");

  // c b a
  list_lpush(list, a);
  list_lpush(list, b);
  list_lpush(list, c);
  list_lpush(list, a);
  list_lpush(list, b);
  list_lpush(list, c);

  // Assertions
  TEST_ASSERT_EQUAL(c , list->head);
  TEST_ASSERT_EQUAL(a , list->tail);
  TEST_ASSERT_EQUAL(3 , list->len);
  TEST_ASSERT_EQUAL(NULL , a->next);
  TEST_ASSERT_EQUAL(b , a->prev);
  TEST_ASSERT_EQUAL(a , b->next);
  TEST_ASSERT_EQUAL(c , b->prev);
  TEST_ASSERT_EQUAL(b , c->next);
  TEST_ASSERT_EQUAL(NULL , c->prev);

  list_destroy(list);
}

void
test_list_at() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_node_new("a");
  list_node_t *b = list_node_new("b");
  list_node_t *c = list_node_new("c");

  // a b c
  list_rpush(list, a);
  list_rpush(list, b);
  list_rpush(list, c);

  // Assertions
  TEST_ASSERT_EQUAL(a , list_at(list, 0));
  TEST_ASSERT_EQUAL(b , list_at(list, 1));
  TEST_ASSERT_EQUAL(c , list_at(list, 2));
  TEST_ASSERT_EQUAL(NULL , list_at(list, 3));

  TEST_ASSERT_EQUAL(c , list_at(list, -1));
  TEST_ASSERT_EQUAL(b , list_at(list, -2));
  TEST_ASSERT_EQUAL(a , list_at(list, -3));
  TEST_ASSERT_EQUAL(NULL , list_at(list, -4));

  list_destroy(list);
}

void
test_list_destroy() {
  // Setup
  list_t *a = list_new();
  list_destroy(a);

  // a b c
  list_t *b = list_new();
  list_rpush(b, list_node_new("a"));
  list_rpush(b, list_node_new("b"));
  list_rpush(b, list_node_new("c"));
  list_destroy(b);

  // Assertions
  list_t *c = list_new();
  c->free = freeProxy;
  list_rpush(c, list_node_new(list_node_new("a")));
  list_rpush(c, list_node_new(list_node_new("b")));
  list_rpush(c, list_node_new(list_node_new("c")));
  list_destroy(c);
  TEST_ASSERT_EQUAL(3 , freeProxyCalls);
}

void
test_list_find() {
  // Setup
  list_t *langs = list_new();
  list_node_t *js = list_rpush(langs, list_node_new("js"));
  list_node_t *ruby = list_rpush(langs, list_node_new("ruby"));

  list_t *users = list_new();
  users->match = User_equal;
  User userTJ = { "tj" };
  User userSimon = { "simon" };
  User userTaylor = { "taylor" };
  list_node_t *tj = list_rpush(users, list_node_new(&userTJ));
  list_node_t *simon = list_rpush(users, list_node_new(&userSimon));

  // Assertions
  list_node_t *a = list_find(langs, "js");
  list_node_t *b = list_find(langs, "ruby");
  list_node_t *c = list_find(langs, "foo");
  TEST_ASSERT_EQUAL(js , a);
  TEST_ASSERT_EQUAL(ruby , b);
  TEST_ASSERT_EQUAL(NULL , c);

  list_destroy(langs);

  a = list_find(users, &userTJ);
  b = list_find(users, &userSimon);
  c = list_find(users, &userTaylor);
  TEST_ASSERT_EQUAL(tj , a);
  TEST_ASSERT_EQUAL(simon , b);
  TEST_ASSERT_EQUAL(NULL , c);

  list_destroy(users);
}

void
test_list_remove() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_rpush(list, list_node_new("a"));
  list_node_t *b = list_rpush(list, list_node_new("b"));
  list_node_t *c = list_rpush(list, list_node_new("c"));

  // Assertions
  TEST_ASSERT_EQUAL(3 , list->len);

  list_remove(list, b);
  TEST_ASSERT_EQUAL(2 , list->len);
  TEST_ASSERT_EQUAL(a , list->head);
  TEST_ASSERT_EQUAL(c , list->tail);
  TEST_ASSERT_EQUAL(c , a->next);
  TEST_ASSERT_EQUAL(NULL , a->prev);
  TEST_ASSERT_EQUAL(NULL , c->next);
  TEST_ASSERT_EQUAL(a , c->prev);

  list_remove(list, a);
  TEST_ASSERT_EQUAL(1 , list->len);
  TEST_ASSERT_EQUAL(c , list->head);
  TEST_ASSERT_EQUAL(c , list->tail);
  TEST_ASSERT_EQUAL(NULL , c->next);
  TEST_ASSERT_EQUAL(NULL , c->prev);

  list_remove(list, c);
  TEST_ASSERT_EQUAL(0 , list->len);
  TEST_ASSERT_EQUAL(NULL , list->head);
  TEST_ASSERT_EQUAL(NULL , list->tail);

  list_destroy(list);
}


void
test_list_remove_new() {
  // Setup
  list_t *list = list_new();
  list_node_t *a =  list_node_new("a");
  list_rpush(list,a);
  list_node_t *b =  list_node_new("b");
  list_rpush(list,b);
  list_node_t *c =  list_node_new("c");
  list_rpush(list,c);

  // Assertions
  TEST_ASSERT_EQUAL(3 , list->len);

  list_remove(list, b);
  TEST_ASSERT_EQUAL(2 , list->len);
  TEST_ASSERT_EQUAL(a , list->head);
  TEST_ASSERT_EQUAL(c , list->tail);
  TEST_ASSERT_EQUAL(c , a->next);
  TEST_ASSERT_EQUAL(NULL , a->prev);
  TEST_ASSERT_EQUAL(NULL , c->next);
  TEST_ASSERT_EQUAL(a , c->prev);

  list_remove(list, a);
  TEST_ASSERT_EQUAL(1 , list->len);
  TEST_ASSERT_EQUAL(c , list->head);
  TEST_ASSERT_EQUAL(c , list->tail);
  TEST_ASSERT_EQUAL(NULL , c->next);
  TEST_ASSERT_EQUAL(NULL , c->prev);

  list_remove(list, c);
  TEST_ASSERT_EQUAL(0 , list->len);
  TEST_ASSERT_EQUAL(NULL , list->head);
  TEST_ASSERT_EQUAL(NULL , list->tail);

  list_destroy(list);
}


void
test_list_remove_without_listing() {
  // Setup
  list_t *list = list_new();
  list_node_t *a =  list_node_new("a");
  list_remove(list, a);

  TEST_ASSERT_EQUAL(0 , list->len);
  TEST_ASSERT_EQUAL(NULL , list->head);
  TEST_ASSERT_EQUAL(NULL , list->tail);

  list_destroy(list);
}

void
test_list_rpop() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_rpush(list, list_node_new("a"));
  list_node_t *b = list_rpush(list, list_node_new("b"));
  list_node_t *c = list_rpush(list, list_node_new("c"));

  // Assertions
  TEST_ASSERT_EQUAL(3 , list->len);

  TEST_ASSERT_EQUAL(c , list_rpop(list));
  TEST_ASSERT_EQUAL(2 , list->len);
  TEST_ASSERT_EQUAL(a , list->head);
  TEST_ASSERT_EQUAL(b , list->tail);
  TEST_ASSERT_EQUAL(a , b->prev);
  TEST_ASSERT_EQUAL(NULL , list->tail->next && "new tail node next is not NULL");
  TEST_ASSERT_EQUAL(NULL , c->prev && "detached node prev is not NULL");
  TEST_ASSERT_EQUAL(NULL , c->next && "detached node next is not NULL");

  free(c);

  TEST_ASSERT_EQUAL(b , list_rpop(list));
  TEST_ASSERT_EQUAL(1 , list->len);
  TEST_ASSERT_EQUAL(a , list->head);
  TEST_ASSERT_EQUAL(a , list->tail);

  free(b);

  TEST_ASSERT_EQUAL(a , list_rpop(list));
  TEST_ASSERT_EQUAL(0 , list->len);
  TEST_ASSERT_EQUAL(NULL , list->head);
  TEST_ASSERT_EQUAL(NULL , list->tail);

  free(a);

  TEST_ASSERT_EQUAL(NULL , list_rpop(list));
  TEST_ASSERT_EQUAL(0 , list->len);

  list_destroy(list);
}

void
test_list_lpop() {
  // Setup
  list_t *list = list_new();
  list_node_t *a = list_rpush(list, list_node_new("a"));
  list_node_t *b = list_rpush(list, list_node_new("b"));
  list_node_t *c = list_rpush(list, list_node_new("c"));

  // Assertions
  TEST_ASSERT_EQUAL(3 , list->len);

  TEST_ASSERT_EQUAL(a , list_lpop(list));
  TEST_ASSERT_EQUAL(2 , list->len);
  TEST_ASSERT_EQUAL(b , list->head);
  TEST_ASSERT_EQUAL(NULL , list->head->prev && "new head node prev is not NULL");
  TEST_ASSERT_EQUAL(NULL , a->prev && "detached node prev is not NULL");
  TEST_ASSERT_EQUAL(NULL , a->next && "detached node next is not NULL");

  free(a);

  TEST_ASSERT_EQUAL(b , list_lpop(list));
  TEST_ASSERT_EQUAL(1 , list->len);

  free(b);

  TEST_ASSERT_EQUAL(c , list_lpop(list));
  TEST_ASSERT_EQUAL(0 , list->len);
  TEST_ASSERT_EQUAL(NULL , list->head);
  TEST_ASSERT_EQUAL(NULL , list->tail);

  free(c);

  TEST_ASSERT_EQUAL(NULL , list_lpop(list));
  TEST_ASSERT_EQUAL(0 , list->len);

  list_destroy(list);
}

void
test_list_iterator_t() {
  // Setup
  list_t *list = list_new();
  list_node_t *tj = list_node_new("tj");
  list_node_t *taylor = list_node_new("taylor");
  list_node_t *simon = list_node_new("simon");

  // tj taylor simon
  list_rpush(list, tj);
  list_rpush(list, taylor);
  list_rpush(list, simon);

  // Assertions

  // From head
  list_iterator_t *it = list_iterator_new(list, LIST_HEAD);
  list_node_t *a = list_iterator_next(it);
  list_node_t *b = list_iterator_next(it);
  list_node_t *c = list_iterator_next(it);
  list_node_t *d = list_iterator_next(it);

  TEST_ASSERT_EQUAL(a , tj);
  TEST_ASSERT_EQUAL(b , taylor);
  TEST_ASSERT_EQUAL(c , simon);
  TEST_ASSERT_EQUAL(NULL , d);

  list_iterator_destroy(it);

  // From tail
  it = list_iterator_new(list, LIST_TAIL);
  list_node_t *a2 = list_iterator_next(it);
  list_node_t *b2 = list_iterator_next(it);
  list_node_t *c2 = list_iterator_next(it);
  list_node_t *d2 = list_iterator_next(it);

  TEST_ASSERT_EQUAL(a2 , simon);
  TEST_ASSERT_EQUAL(b2 , taylor);
  TEST_ASSERT_EQUAL(c2 , tj);
  TEST_ASSERT_EQUAL(NULL , d2);
  list_iterator_destroy(it);

  list_destroy(list);
}

MAIN(){
  UNITY_BEGIN();
  printf("\nlist_t: %ld\n", sizeof(list_t));
  printf("list_node_t: %ld\n", sizeof(list_node_t));
  printf("list_iterator_t: %ld\n\n", sizeof(list_iterator_t));
  RUN_TEST(test_list_node_new);
  RUN_TEST(test_list_rpush);
  RUN_TEST(test_list_lpush);
  RUN_TEST(test_list_lpush_only);
  RUN_TEST(test_list_lpush_existing);
  RUN_TEST(test_list_find);
  RUN_TEST(test_list_at);
  RUN_TEST(test_list_remove);
  RUN_TEST(test_list_remove_new);
  RUN_TEST(test_list_remove_without_listing);
  RUN_TEST(test_list_rpop);
  RUN_TEST(test_list_lpop);
  RUN_TEST(test_list_destroy);
  RUN_TEST(test_list_iterator_t);
  puts("... \x1b[32m100%\x1b[0m\n");
  UNITY_END(); // stop unit testing
  return 0;
}
