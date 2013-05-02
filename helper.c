/*
 *
 * helper.c
 *
 * Created at:  Thu 02 May 2013 17:12:48 CEST 17:12:48
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
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "navis.h"
#include "helper.h"

/*
 * Check if given <fname> is a directory or not.
 */
BOOL isdir(char *fname)
{
  struct stat st;
  stat(fname, &st);

  return S_ISDIR(st.st_mode);
}

/*
 * My little implementation of getting an extension from a file.
 */
char *getext(char *s)
{
  static char ext[7];
  int i = 0;
  char *p = s + strlen(s) - 1;
  while (p != s){
    if (*p == '.')
      break;
    ext[i] = *p;
    p--; i++;
  }
  /* reverse the extension */
  strrev(ext);

  return ext;
}

/*
 * Reverse given string.
 */
void strrev(char *s)
{
  int len = strlen(s);
  int c, i, j;

  for (i = 0, j = len - 1; i < j; i++, j--){
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

