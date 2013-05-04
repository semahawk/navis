/*
 *
 * conf.h
 *
 * Created at:  Fri 03 May 2013 21:34:05 CEST 21:34:05
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

#ifndef CONF_H
#define CONF_H

/*
 * Default options
 */
#define CONF_PORT "80"

/*
 * Navis data types
 */
struct conf {
  /* obviously, the port to listen on */
  char *port;
};

enum symbol_t {
  SYM_INTEGER,
  SYM_STRING,
  SYM_NAME,
  SYM_NEWLINE,
  SYM_SEMICOLON,
  SYM_EOF
};

struct symbol {
  struct {
    enum symbol_t type;
    union {
      int i;
      char *s;
    } data;
  } value;
  unsigned line;
  unsigned column;
  struct symbol *next;
  struct symbol *prev;
};

struct lex {
  /* first element of the symbols list */
  struct symbol *head;
  /* last element of the symbols list */
  struct symbol *tail;
  /* current element of the symbols list */
  struct symbol *current;
  /* current line */
  unsigned line;
  unsigned column;
};

void fetchConf(struct conf *);
void confDestroy(struct conf *);

#endif /* CONF_H */

