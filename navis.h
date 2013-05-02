/*
 *
 * navis.h
 *
 * Created at:  Thu 02 May 2013 09:41:40 CEST 09:41:40
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

#ifndef NAVIS_H
#define NAVIS_H

#include <stdio.h>

/* maximum size of the request header */
#define MAX_HEADER_REQUEST 1024
/* the port users will connect to */
#define PORT "1337"
/* how many pending connections queue will hold */
#define BACKLOG 10

#undef BOOL
#  define BOOL short
#undef TRUE
#  define TRUE 1
#undef FALSE
#  define FALSE 0

void send_header(int fd, const char *code, char *content_length, const char *mime_type);
void send_file(int fd, FILE *fp);
char *getext(char *s);
void strrev(char *s);
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

#endif /* NAVIS_H */
