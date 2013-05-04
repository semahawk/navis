/*
 *
 * conf.c
 *
 * Created at:  Fri 03 May 2013 21:37:35 CEST 21:37:35
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "navis.h"
#include "conf.h"

/*
 * Functions to append a symbol to the symbols list
 */
static void append(struct lex *lex, enum symbol_t type)
{
  struct symbol *new = malloc(sizeof(struct symbol));
  /* initialize */
  new->line = lex->line;
  new->column = lex->column;
  new->value.type = type;
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void appendInt(struct lex *lex, int i)
{
  struct symbol *new = malloc(sizeof(struct symbol));
  /* initialize */
  new->line = lex->line;
  new->column = lex->column;
  new->value.type = SYM_INTEGER;
  new->value.data.i = i;
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void appendStr(struct lex *lex, enum symbol_t type, char *s)
{
  struct symbol *new = malloc(sizeof(struct symbol));
  /* initialize */
  new->line = lex->line;
  new->column = lex->column;
  new->value.type = type;
  new->value.data.s = strdup(s);
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

/*
 * "Cast" given <type> to string
 */
static const char *symToS(enum symbol_t type)
{
  switch (type){
    case SYM_INTEGER:
      return "integer";
    case SYM_STRING:
      return "string";
    case SYM_NAME:
      return "name";
    case SYM_NEWLINE:
      return "'\\n'";
    case SYM_EOF:
      return "<EOF>";
    default:
      return "#unknown#symToS#";
  }
}

/*
 * If next symbol is not of given <type> throw some nice error message
 */
static void lexForce(struct lex *lex, enum symbol_t type)
{
  if (!lex->current){
    return;
  }

  if (lex->current->value.type != type){
    fprintf(stderr, "navis: expected %s instead of %s in line %u at column %u\n", symToS(type), symToS(lex->current->value.type), lex->current->line, lex->current->column);
    exit(EXIT_FAILURE);
  }

  lex->current = lex->current->next;
}

/*
 * Return TRUE if next symbol is of given <type>
 *        FALSE if not
 */
static BOOL lexPeek(struct lex *lex, enum symbol_t type)
{
  if (!lex->current){
    return FALSE;
  }

  if (lex->current->value.type == type)
    return TRUE;
  else
    return FALSE;
}

/*
 * Skip over the current symbol, no questions asked
 */
static void lexSkip(struct lex *lex)
{
  if (!lex->current){
    return;
  }

  lex->current = lex->current->next;
}

/*
 * Skip to next symbol if the current is a name and equal <name>
 */
static BOOL lexAcceptName(struct lex *lex, char *name)
{
  if (!lex->current){
    return FALSE;
  }

  if (lex->current->value.type != SYM_NAME)
    return FALSE;

  if (!strcmp(lex->current->value.data.s, name)){
    lex->current = lex->current->next;
    return TRUE;
  } else {
    return FALSE;
  }
}

/*
 * Create a symbols list from a given <file>
 */
static void lexConf(struct lex *lex, const char *file)
{
  FILE *fp;
  char *fbuffer = NULL;
  char *p, *tmp;
  int i;
  size_t flen = 0;

  if ((fp = fopen(file, "r")) == NULL){
    perror("navis: lexer");
    exit(EXIT_FAILURE);
  }

  /* get the files length */
  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  /* make room for the contents */
  fbuffer = malloc(flen);
  if (fbuffer == NULL){
    fprintf(stderr, "malloc failed in " __FILE__ " at line %d", __LINE__);
    exit(EXIT_FAILURE);
  }
  /* store the files contents in the fbuffer */
  if (fread(fbuffer, 1, flen, fp) != flen){
    fprintf(stderr, "fread failed in " __FILE__ " at line %d", __LINE__);
    exit(EXIT_FAILURE);
  }
  fbuffer[flen - 1] = '\0';
  /* do the actuall lexing */
  for (p = fbuffer, i = 0; *p != '\0'; p++, i = 0){
    /*
     * XXX name
     */
    if (isalpha(*p)){
      tmp = strdup(p);
      /* fetch the name */
      while (isalpha(*p)){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      appendStr(lex, SYM_NAME, tmp);
      free(tmp);
      /* i is the length of the name */
      lex->column += i;
    }
    /*
     * XXX integer
     */
    else if (isdigit(*p)){
      tmp = strdup(p);
      /* fetch the digit */
      while (isdigit(*p)){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      appendInt(lex, atoi(tmp));
      free(tmp);
      /* i is the length of the name */
      lex->column += i;
    }
    /*
     * XXX "string"
     */
    else if (*p == '"'){
      p++; i++;
      tmp = strdup(p);
      while (*p != '"'){
        if (*p == '\n'){
          lex->line++;
          lex->column = 1;
        }
        p++; i++;
      }
      p--; i--;
      *(tmp + i) = '\0';
      /* skip over the '"' */
      p++;
      appendStr(lex, SYM_STRING, tmp);
      free(tmp);
      lex->column += i;
    }
    /*
     * XXX newline
     */
    else if (*p == '\n'){
      append(lex, SYM_NEWLINE);
      lex->line++;
      lex->column = 1;
    }
    /*
     * XXX semicolon
     */
    else if (*p == ';'){
      append(lex, SYM_SEMICOLON);
      lex->column++;
    }
    /*
     * XXX space
     */
    else if (*p == ' '){
      lex->column++;
    }
    /*
     * XXX tab
     */
    else if (*p == '\t'){
      lex->column++;
    }
    /*
     * XXX unknown
     */
    else {
      fprintf(stderr, "navis: unknown character '%c' in line %u, column %u\n", *p, lex->line, lex->column);
      exit(EXIT_FAILURE);
    }
  }
  append(lex, SYM_EOF);
  /* free the buffer */
  free(fbuffer);
  /* close the f */
  fclose(fp);
}

static void option(struct conf *conf, struct lex *lex)
{
  char portins[10];

  /*
   * XXX port INTEGER
   */
  if (lexAcceptName(lex, "port")){
    lexForce(lex, SYM_INTEGER);
    /*lex->current = lex->current->next;*/
    sprintf(portins, "%d", lex->current->prev->value.data.i);
    conf->port = strdup(portins);
  }

  /*
   * end an option
   */
  if (!lexPeek(lex, SYM_EOF)){
    if (!lexPeek(lex, SYM_NEWLINE) &&
        !lexPeek(lex, SYM_SEMICOLON)){
      fprintf(stderr, "navis: expected a newline or semicolon at the end of an option in line %u at column %u\n", lex->line, lex->column);
      exit(EXIT_FAILURE);
    }
    lexSkip(lex);
  }
}

static void lexDestroy(struct lex *lex)
{
  struct symbol *s;
  struct symbol *next;

  for (s = lex->tail; s != NULL; s = next){
    next = s->next;
    if (s->value.type == SYM_STRING ||
        s->value.type == SYM_NAME){
      free(s->value.data.s);
    }
    free(s);
  }
}

static void parseConf(struct conf *conf, const char *file)
{
  struct lex lex;
  /* initialize */
  lex.line = 1;
  lex.column = 1;
  lex.head = NULL;
  lex.tail = NULL;
  lex.current = NULL;
  lexConf(&lex, file);
  /* start the actual parsing */
  while (!lexPeek(&lex, SYM_EOF)){
    option(conf, &lex);
  }
  lexDestroy(&lex);
}

void fetchConf(struct conf *conf)
{
  /* first, set the defaults */
  conf->port = CONF_PORT;
  /* TODO: after installing, the path is going to be different */
  parseConf(conf, "conf/navis.conf");
}

/*
 * Free any data that was malloced for options
 */
void confDestroy(struct conf *conf)
{
  free(conf->port);
}

