#include "../include/common.h"
#include "../include/misc.h"
#include "../include/lexer.h"

static char get_next_char(FILE *file);
static void add_char_to_token_str(TOKEN *ret, int *count, const char ch);
static void add_str_to_token_str(TOKEN *ret, int *count, const char *str);
static bool if_next_str(FILE *file, const char ch, const char *str);

extern TOKEN get_next_token(FILE *file) {/*{{{*/
  TOKEN ret = initialize_token();

  int count = 0;
  char ch, ch_more;
  while (true) {
    ch = get_next_char(file);

    // 空白文字を読み飛ばす/*{{{*/
    if (isspace(ch)) {
      while (true) {
        ch = get_next_char(file);
        if (!isspace(ch)) break;
      }
    }/*}}}*/

    if (ch == EOF) break;

    if      (ch == '+' || ch == '-') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = TERM_OPERATOR;
      break;
    }/*}}}*/
    else if (ch == '*' || ch == '/' || ch == '%') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = FACTOR_OPERATOR;
      break;
    }/*}}}*/
    else if (ch == '=') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ch_more = get_next_char(file);

      if (ch_more == '=') {
        add_char_to_token_str(&ret, &count, ch_more);
        ret.kind = COMPARE_OPERATOR;
      } else {
        ungetc(ch_more, file);
        ret.kind = EQUAL;
      }

      break;
    }/*}}}*/
    else if (ch == '!') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ch_more = get_next_char(file);

      if (ch_more == '=') {
        add_char_to_token_str(&ret, &count, ch_more);
        ret.kind = COMPARE_OPERATOR;
      } else EXIT_NOMSG;

      break;
    }/*}}}*/
    else if (ch == '>' || ch == '<') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = COMPARE_OPERATOR;
      ch_more = get_next_char(file);

      if (ch_more == '=') add_char_to_token_str(&ret, &count, ch_more);
      else                ungetc(ch_more, file);

      break;
    }/*}}}*/
    else if (ch == ',') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = COMMA;
      break;
    }/*}}}*/
    else if (ch == ';') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = SEMICOLON;
      break;
    }/*}}}*/
    else if (ch == '(') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = LPAREN;
      break;
    }/*}}}*/
    else if (ch == ')') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = RPAREN;
      break;
    }/*}}}*/
    else if (ch == '{') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = LBRACKET;
      break;
    }/*}}}*/
    else if (ch == '}') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = RBRACKET;
      break;
    }/*}}}*/
    else if (if_next_str(file, ch, "if"    )) {/*{{{*/
      add_str_to_token_str(&ret, &count, "if");
      ret.kind = IF_KEYWORD;
      break;
    }/*}}}*/
    else if (if_next_str(file, ch, "else"  )) {/*{{{*/
      add_str_to_token_str(&ret, &count, "else");
      ret.kind = ELSE_KEYWORD;
      break;
    }/*}}}*/
    else if (if_next_str(file, ch, "while" )) {/*{{{*/
      add_str_to_token_str(&ret, &count, "while");
      ret.kind = WHILE_KEYWORD;
      break;
    }/*}}}*/
    else if (if_next_str(file, ch, "return")) {/*{{{*/
      add_str_to_token_str(&ret, &count, "return");
      ret.kind = RETURN_KEYWORD;
      break;
    }/*}}}*/
    else if (if_next_str(file, ch, "int"   )) {/*{{{*/
      add_str_to_token_str(&ret, &count, "int");
      ret.kind = TYPE_KEYWORD;
      break;
    }/*}}}*/
    else if (isdigit(ch)) {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = NUMBER;

      while (true) {
        ch_more = get_next_char(file);
        if (isdigit(ch_more)) add_char_to_token_str(&ret, &count, ch_more);
        else {
          ungetc(ch_more, file);
          break;
        }
      }

      break;
    }/*}}}*/
    else if (isalpha(ch) || ch == '_') {/*{{{*/
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = IDENTIFY;

      while (true) {
        ch_more = get_next_char(file);
        if (isalpha(ch_more) || isdigit(ch_more) || ch_more == '_') add_char_to_token_str(&ret, &count, ch_more);
        else {
          ungetc(ch_more, file);
          break;
        }
      }

      break;
    }/*}}}*/
    else {
      add_char_to_token_str(&ret, &count, ch);
      ret.kind = OTHER_TOKEN;
      fprintf(stderr, "```%c''' is invalid character\n", ch);
    }

  }

  ret.str[count] = '\0';
  return ret;
}/*}}}*/
static char get_next_char(FILE *file) {/*{{{*/
  char ch = fgetc(file);

  if        (ch == '/') {
    char ch_more = fgetc(file);
    if        (ch_more == '/') { // ダブルスラッシュ型コメントを削除して空白を返す/*{{{*/
      ch = ' ';
      while (true) {
        ch_more = fgetc(file);
        if (ch_more == EOF || ch_more == '\n') break;
      }
    }/*}}}*/
    else if (ch_more == '*') { // スラッシュアスタリスク型コメントを削除して空白を返す/*{{{*/
      ch = ' ';

      bool comment_soon_end = false;
      while (true) {
        ch_more = fgetc(file);
        if      (ch_more == EOF) break;
        else if (ch_more == '*' || comment_soon_end == true) {
          ch_more = fgetc(file);
          if      (ch_more == EOF || ch_more == '/') break;
          else if (ch_more == '*') comment_soon_end = true;
          else                     comment_soon_end = false;
        }
      }
    }/*}}}*/
    else ungetc(ch_more, file); // コメントでなければ、余計に読んだ分を戻す

  } else if (isspace(ch) || ch == '\n') ch = ' ';

  return ch;
}/*}}}*/
static void add_char_to_token_str(TOKEN *ret, int *count, const char ch) {/*{{{*/
  // 後で\0'を追加する余地を残すため TOKEN_STRING_MAX_LENGTH-1 と比べる
  if (*count >= TOKEN_STRING_MAX_LENGTH-1) EXIT_MSG("TOKEN string too long\n");
  ret -> str[*count] = ch;
  (*count)++;
}/*}}}*/
static void add_str_to_token_str(TOKEN *ret, int *count, const char *str) {/*{{{*/
  // str="hoge"なら*count=4
  *count = strlen(str);
  for (int i=0; i < *count; i++) {
    ret -> str[i] = str[i];
  }
}/*}}}*/
static bool if_next_str(FILE *file, const char ch, const char *str) {/*{{{*/
  bool flag = true;
  int str_len = strlen(str);

  ungetc(ch, file);
  for (int i=0; i<str_len; i++) {
    char ch = fgetc(file);
    if (ch != str[i] || ch == EOF) {
      ungetc(ch, file);
      for (int j=0; j<i; j++) ungetc(str[i-1-j], file);
      fgetc(file);
      flag = false;
      break;
    }
  }

  if (flag) {
    char ch = fgetc(file);
    if (isdigit(ch) || isalpha(ch) || ch=='_') {
      ungetc(ch, file);
      for (int j=0; j<str_len; j++) ungetc(str[str_len-1-j], file);
      flag = false;
    } else ungetc(ch, file);
  }

  return flag;
}/*}}}*/
extern int count_token_num(FILE *file) {/*{{{*/
  int count = 0;
  TOKEN blank_token = initialize_token();

  rewind(file);
  while (!compare_token( get_next_token(file), blank_token )) count++;
  rewind(file);

  return count;
}/*}}}*/
