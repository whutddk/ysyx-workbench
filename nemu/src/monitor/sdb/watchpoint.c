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

  if(head != NULL) {
    while( (p -> next) != NULL ) {
      p = p -> next;
    }
    p -> next = free_;
    p = free_;
  } else {
    head = free_;
    p = free_;
  }

    
  // assert( p != NULL );

  assert( strlen(args) < 256 );
  // printf( "Test %s \n", args);
  strcpy(p->expr, args);
  // printf( "Test %s \n", p->expr);

  bool isSuccess;
  p->pre = expr(p->expr, &isSuccess);

  free_ = free_->next;
  p->next = NULL;


  return ;
}

void free_wp(int NO) {
  WP* p = head;
  WP* q;

  assert( p != NULL);

  if (head->NO == NO) {
    q = head->next;

    head->next = free_;
    free_ = head;
    head = q;

  } else {
    while( 1 ) {
      if ( (p -> next) == NULL) {
        printf( "watchpoint %d Not Found!\n", NO);
        return;
      }
      if( ((p -> next) -> NO) == NO ) {
        break;
      }
      p = p -> next;
    }
    q = p;
    p -> next = p -> next -> next;
    q -> next = free_;
    free_ = q;

  }


  




}

void info_w() {
  WP *p = head;
  while( p != NULL ) {
    printf( "watchpoint %d: expr is %s, perivious result is 0x%lx ( %ld ) \n", p->NO, p->expr, p->pre, p->pre );
    p = p->next;
  }

}

int cmp_w() {
  WP *p = head;
  int res = 0;
  while( p != NULL ) {

    bool isSuccess;
    uint64_t new = expr(p->expr, &isSuccess);

    if( (p -> pre) != new ) {
      res = 1;
      printf( "Watchpoint %d Trigger: expr is %s, perivious result is 0x%lx ( %ld ), new result is 0x%lx ( %ld ) \n", p->NO, p->expr, p->pre, p->pre, new, new );      
    }
    p->pre = new;
    p = p->next;
  }

  return res;
}


