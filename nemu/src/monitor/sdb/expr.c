#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal

  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"[0-9]+", TK_NUM},
  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case(TK_NUM):

            tokens[nr_token].type = TK_NUM;

            if( substr_len > 30 ) { assert(0); }

            for ( int k = 0; k < substr_len; k++  ) {
              tokens[nr_token].str[k] = *(substr_start + k);
            }
            tokens[nr_token].str[substr_len] = '\0';

            // printf( "Number: %s\n", tokens[nr_token].str );
            nr_token ++;
            break;

          case(TK_NOTYPE): break;
          case('+'): 
          case('-'): 
          case('*'): 
          case('/'): 
          case('('): 
          case(')'): 
          case(TK_EQ): 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = 0;
            nr_token ++;
            break;


          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}



static bool check_parentheses(uint8_t p, uint8_t q) {
  uint8_t cnt = 0;
  for ( uint8_t i = p; i <= q; i ++ ) {
    if( tokens[i].type == '(' ) { cnt ++; }
    else if( tokens[i].type == ')' ) { assert( cnt > 0 ); cnt --; }
  }
  assert( cnt == 0 );
  
  return (tokens[p].type == '(') && (tokens[q].type == ')');

}

static uint32_t eval(uint8_t p, uint8_t q) {
  if( p > q ) {
    assert(0);
  }
  else if( p == q ) {
    assert(tokens[p].type == TK_NUM);
    return atoi(tokens[p].str);
  }
  else if( check_parentheses(p, q) == true ) {
    return eval(p+1, q-1);
  }
  else { // check_parentheses(p, q) == false
    uint8_t op = 0;
    int op_type = TK_NOTYPE;
    uint8_t parentheses_cnt = 0;

    for ( op = p; op < q; op ++ ) {
      if ( tokens[op].type == TK_NUM ) { ; }
      else if ( tokens[op].type == TK_NOTYPE) { assert(0); }
      else if (
          ( tokens[op].type == '+' ) ||
          ( tokens[op].type == '-' ) ||
          ( tokens[op].type == '*' ) ||
          ( tokens[op].type == '/')
        ) {
        if ( parentheses_cnt == 0 ) {
          op_type = tokens[op].type;
          break;
        }
      }
      else if ( tokens[op].type == '(') {
        parentheses_cnt ++;
      }
      else if ( tokens[op].type == ')') {
        assert( parentheses_cnt > 0 );
        parentheses_cnt --;
      }
      else if ( tokens[op].type == TK_EQ) {
        TODO();
      }
    }

    if ( op_type == TK_NOTYPE ) { assert(0); }

    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);

    switch (op_type) {
      case '+': return val1 + val2; break;
      case '-': return val1 - val2; break;
      case '*': return val1 * val2; break;
      case '/': assert(val2 != 0); return val1 / val2; break;
      default: assert(0); return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  } else {
  /* TODO: Insert codes to evaluate the expression. */
  
  *success = true;
  return eval(0, nr_token-1);    
  }


}
