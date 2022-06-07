#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
  WP* p = head;
  while( p != NULL ) {
    p = p -> next;
  }
  p = free_;
  assert( p != NULL );

  free_ = free_->next;
  p->next = NULL;

  return p;
}

void free_wp(WP *wp){
  WP* p = head;

  assert( p != NULL);

  while( (p -> next ) != wp ) {
    p = p -> next;
    assert( (p -> next) != NULL);
  }

  p -> next = wp -> next;
  wp -> next = free_;
  free_ = wp;

}
/* TODO: Implement the functionality of watchpoint */

