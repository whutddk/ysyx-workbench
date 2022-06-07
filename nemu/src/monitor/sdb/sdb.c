#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include "stdlib.h"
#include "memory/paddr.h"


static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int cnt;
  if ( args == NULL ) {
    cnt = 1;
  } else {
    cnt = atoi(args);
  }

  if ( cnt == 0 ) {
    printf("Parameter Error!\n");
  } else {
    cpu_exec(cnt);
  }
  return 0;
}

static int cmd_info(char *args) {
  // printf( "arg is %s\n", args );
  extern const char* regs[];
  for (int i = 0; i < 32; i ++) {
    if (strcmp(args, regs[i]) == 0) {
      printf("%s = 0x%lx ( %ld )  \n", args, cpu.gpr[i], cpu.gpr[i]);
      break;
    }
  }
  if (strcmp(args, "pc") == 0) {
    printf("%s = 0x%lx\n", args, cpu.pc);
  }

  if (strcmp(args, "r") == 0) {
    isa_reg_display();
  }

  if (strcmp(args, "w") == 0) {
    printf("Warning!! Un-implement feature\n");
  }

  return 0;
}

static int cmd_x(char *args) {
  int cnt;
  long address ;


  char *cnt_str = strtok(args, " ");
  char *address_str ;

  if (cnt_str == NULL) { 
    cnt = 1;
    address = 0x80000000;
  } else {
    cnt = atoi(cnt_str);

    // printf("%s\n", cnt_str);

    address_str = cnt_str + strlen(cnt_str) + 1;
    // printf("%s\n", address_str);


    if ( address_str[0] == '0' && address_str[1] == 'x') {
      address = strtoul (address_str+2, NULL, 16);
    } else {
      address = atoi(address_str);
    }
    
  }


  // printf("cnt = %d, address = 0x%lx ( %ld )\n", cnt, address, address);
  for ( int i = 0; i < cnt; i ++ ) {
    printf( "%lx -> 0x%lx\n", (address + i*4), paddr_read(address + i*4, 4) );
  }
  

  return 0;
}

static int cmd_expr(char *args) {
  bool isSuccess;
  uint64_t res = expr(args, &isSuccess);
  
  if ( isSuccess ) {
    printf( "Expr result is %ld\n", res);  
       
  } else {
    printf( "Expr Failed\n");
  }
  
  return 0;   

}


static int cmd_help(char *args);



static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Continue the execution of the program by N steps, and than pause, default N is 1", cmd_si },
  { "info", "Display ths status of register for \" info r \" or Display ths status of WatchPoint for \" info w \" ", cmd_info },
  { "x", "x N EXPR, Dispaly N words in hex-format, whose memory-address is begining at EXPR", cmd_x},
  { "expr", "test expr", cmd_expr}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
