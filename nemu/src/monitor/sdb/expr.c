#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "memory/paddr.h"

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NE, TK_AND, TK_OR, TK_NUM, TK_HEX, TK_REG, DEREF

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
  {"\\!\\=", TK_NE},
  {"\\&\\&", TK_AND},
  {"\\|\\|", TK_OR},
  {"\\$[a-z0-3]+", TK_REG},

  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\0\\x[0123456789abcdefABCDEF]+", TK_HEX},
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

          case(TK_HEX):

            tokens[nr_token].type = TK_HEX;

            if( substr_len > 32 ) { assert(0); }

            for ( int k = 2; k < substr_len; k++  ) {
              tokens[nr_token].str[k-2] = *(substr_start + k);
            }
            tokens[nr_token].str[substr_len-2] = '\0';

            // printf( "Number: %s\n", tokens[nr_token].str );
            nr_token ++;
            break;

          case(TK_REG):
            tokens[nr_token].type = TK_REG;

            if( substr_len > 30 ) { assert(0); }

            for ( int k = 1; k < substr_len; k++  ) {
              tokens[nr_token].str[k-1] = *(substr_start + k);
            }
            tokens[nr_token].str[substr_len-1] = '\0';

            nr_token ++;

          case(TK_NOTYPE): break;

          case(TK_EQ): 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '=';
            tokens[nr_token].str[1] = '=';
            tokens[nr_token].str[2] = 0;
            nr_token ++;
            break;
          case(TK_NE): 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '!';
            tokens[nr_token].str[1] = '=';
            tokens[nr_token].str[2] = 0;
            nr_token ++;
            break;
          case(TK_AND): 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '&';
            tokens[nr_token].str[1] = '&';
            tokens[nr_token].str[2] = 0;
            nr_token ++;
            break;
          case(TK_OR): 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '|';
            tokens[nr_token].str[1] = '|';
            tokens[nr_token].str[2] = 0;
            nr_token ++;
            break;
          case('+'): 
          case('-'): 
          case('*'):
          case('/'): 
          case('('): 
          case(')'): 
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

  // printf(" p = %d, q = %d\n", p, q);

  printf( "CHECKING PARENTHESES!!!\n" );
  for ( uint8_t i = p; i <= q; i ++ ) {
    if ( (tokens[i].type == TK_NUM) || ( tokens[i].type == TK_EQ ) || ( tokens[i].type == TK_NE ) || ( tokens[i].type == TK_AND ) || ( tokens[i].type == TK_OR ) ) {
      printf("%s ", tokens[i].str);
    } else if( tokens[i].type == TK_HEX ) {
      printf("0x%s ", tokens[i].str);
    } else {
      printf("%c", tokens[i].type);      
    }

  }
   printf("\n");

  uint8_t cnt = 0;
  for ( uint8_t i = p; i <= q; i ++ ) {
    if( tokens[i].type == '(' ) { cnt ++; }
    else if( tokens[i].type == ')' ) { assert( cnt > 0 ); cnt --; }
  }
  assert( cnt == 0 );
  
  return (tokens[p].type == '(') && (tokens[q].type == ')');

}

static uint64_t eval(uint8_t p, uint8_t q) {
  if( p > q ) {
    assert(0);
  }
  else if( p == q ) {
    if (tokens[p].type == TK_NUM) {
      return atoi(tokens[p].str);      
    } else if ( tokens[p].type == TK_HEX ) {
      return strtoul (tokens[p].str, NULL, 16);      
    } else if ( tokens[p].type == TK_REG ) {
      bool isSuccess;
      uint64_t reg = isa_reg_str2val(tokens[p].str, &isSuccess);
      if( isSuccess ) {
        return reg;        
      } else {
        printf( "Reg Mis-match\n" );
        return 0;
      }

    } else {
      assert(0);
      return 0;
    }


  }
  else if( check_parentheses(p, q) == true ) {
    return eval(p+1, q-1);
  }
  else { // check_parentheses(p, q) == false
    uint8_t op = 0;
    int op_type = TK_NOTYPE;
    uint8_t parentheses_cnt = 0;

    for ( int i = p; i < q; i ++ ) {
      if ( (tokens[i].type == TK_NUM) || (tokens[i].type == TK_HEX) ) { ; }
      else if ( tokens[i].type == TK_NOTYPE) { assert(0); }
      else if (
          ( tokens[i].type == '+' ) ||
          ( tokens[i].type == '-' ) ||
          ( tokens[i].type == '*' ) ||
          ( tokens[i].type == '/' ) ||
          ( tokens[i].type == TK_EQ ) ||
          ( tokens[i].type == TK_NE ) ||
          ( tokens[i].type == TK_AND ) ||
          ( tokens[i].type == TK_OR ) ||
          ( tokens[i].type == DEREF )
        ) {
        // printf( "tokens[i].type is %c\n", tokens[i].type );
        // printf( "parentheses_cnt is %d\n", parentheses_cnt );
        if ( parentheses_cnt == 0 ) {
          if ( ( (tokens[i].type == DEREF) && ((op_type == '*') || (op_type == '/')) ) ||
               (( (tokens[i].type == '*') || (tokens[i].type == '/') || (tokens[i].type == DEREF) ) && ((op_type == '+') || (op_type == '-'))) ||
               (( (tokens[i].type == '+') || (tokens[i].type == '-') || (tokens[i].type == '*') || (tokens[i].type == '/') || (tokens[i].type == DEREF) ) && ((op_type == TK_EQ) || (op_type == TK_NE) || (op_type == TK_AND) || (op_type == TK_OR)))
            ) {
            // printf( "continue!!!\n" );
            continue;
          } else {
            op_type = tokens[i].type;
            op = i;
          }
        }
        // printf( "op_type is %c\n", op_type );
      }
      else if ( tokens[i].type == '(') {
        // printf( "tokens[i].type is %c\n", tokens[i].type );
        parentheses_cnt ++;
      }
      else if ( tokens[i].type == ')') {
        // printf( "tokens[i].type is %c\n", tokens[i].type );
        assert( parentheses_cnt > 0 );
        parentheses_cnt --;
      }
      else {
        TODO();
      }
    }

    if ( op_type == TK_NOTYPE ) { assert(0); }

    if( op_type == DEREF ) {
      assert( op == p );
      uint64_t val1 = (eval(op + 1, q));
      return paddr_read(val1, 4);
    } else {
      uint64_t val1 = eval(p, op - 1);
      uint64_t val2 = eval(op + 1, q);

      switch (op_type) {
        case '+': return val1 + val2; break;
        case '-': return val1 - val2; break;
        case '*': return val1 * val2; break;
        case '/': assert(val2 != 0); return val1 / val2; break;
        case(TK_EQ):  return val1 == val2; break;
        case(TK_NE):  return val1 != val2; break;
        case(TK_AND): return val1 && val2; break;
        case(TK_OR):  return val1 || val2; break;

        default: assert(0); return 0;
      }      
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

    for ( int i = 0; i < nr_token; i ++) {
      if (
        (tokens[i].type == '*') &&
          ( 
            i == 0 ||
            (tokens[i - 1].type == TK_EQ) ||
            (tokens[i - 1].type == TK_NE) ||
            (tokens[i - 1].type == TK_AND) ||
            (tokens[i - 1].type == TK_OR) ||
            (tokens[i - 1].type == DEREF) ||
            (tokens[i - 1].type == '+') ||
            (tokens[i - 1].type == '-') ||
            (tokens[i - 1].type == '*') ||
            (tokens[i - 1].type == '/') ||
            (tokens[i - 1].type == '(')
          ) ) {
        tokens[i].type = DEREF;
        tokens[i].str[0] = '*';
        tokens[i].str[1] = 0;
      }
    }

    return eval(0, nr_token-1);    
  }


}

