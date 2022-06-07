#include "sdb.h"
#include "string.h"
#include "stdio.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  char  expr[256];
  uint64_t pre;
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

void new_wp( char* args ) {
  WP* p = head;
  while( p != NULL ) {
    p = p -> next;
  }
  p = free_;
  assert( p != NULL );

  assert( strlen(args) < 256 );
  strcpy(p->expr, args);

  p->pre = expr(p->expr, NULL);

  free_ = free_->next;
  p->next = NULL;

  return ;
}

void free_wp(int NO) {
  WP* p = head;
  WP* q;

  assert( p != NULL);

  while( p->NO != NO ) {
    if ( (p -> next) == NULL) {
      printf( "watchpoint %d Not Found!\n", NO);
    }
    p = p -> next;
  }
  q = p;
  p -> next = q -> next;
  q -> next = free_;
  free_ = q;

}

void info_w() {
  WP *p = head;
  while( p != NULL ) {
    printf( "watchpoint %d: expr is %s, perivious result is 0x%lx ( %ld ) \n", p->NO, p->expr, p->pre, p->pre );
    p = p->next;
  }

}
/* TODO: Implement the functionality of watchpoint */

