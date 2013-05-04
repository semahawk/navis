/*
 *
 * navis.c
 *
 * Created at:  Wed 01 May 2013 18:34:53 CEST 18:34:53
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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#include <getopt.h>

#include "navis.h"
#include "helper.h"
#include "conf.h"

static struct mime {
  const char * const ext;
  const char * const type;
} mimes[] = {
  { "jpg",      "image/jpeg" },
  { "png",      "image/png"  },
  { "txt",      "text/plain" },
  { "css",      "text/css"   },
  { "js",       "text/plain" },
  { "html",     "text/html"  },
  { NULL, NULL },
};

int main(int argc, char **argv)
{
  /* the main configuration */
  struct conf conf;
  /* manage the command line options */
  int c;
  while (1){
    static struct option long_options[] = {
      { "version", no_argument, 0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "v", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){
      case 'v': printf("Navis v" VERSION ", " __DATE__ " " __TIME__"\n");
                return EXIT_SUCCESS;
      case '?': break;
      default: abort();
    }
  }
  /* listen on sockfd, new connections on newfd */
  int sockfd, newfd;
  struct addrinfo hints, *servinfo, *p;
  /* connector's address information */
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  /* parse the configuration file and save the options */
  fetchConf(&conf);
  /* clear the hints */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; /* use my IP */

  if ((rv = getaddrinfo(NULL, conf.port, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next){
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("navis: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("navis: bind");
      continue;
    }

    break;
  }

  if (p == NULL){
    fprintf(stderr, "navis: failed to bind to port %s\n", conf.port);
    return 2;
  }
  /* all done with this structure */
  freeaddrinfo(servinfo);

  if (listen(sockfd, BACKLOG) == -1){
    perror("listen");
    exit(1);
  }

  /* reap all dead processes */
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1){
    perror("sigaction");
    exit(1);
  }

  printf(" \e[1;32m*\e[0;0m Navis v" VERSION " listening at port %s\n", conf.port);

  while (1){
    unsigned content_length = 0;
    char content_length_str[7];

    sin_size = sizeof their_addr;
    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (newfd == -1){
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

    /* this is the child process */
    if (!fork()){
      /* child doesn't need the listener */
      close(sockfd);
      const char *code = "200 OK";
      const char *method = NULL;
      /* fetch the request header */
      char request_header[MAX_HEADER_REQUEST], *fname;
      FILE *fp;
      /* extension of the requested file */
      char *fext;
      int n;
      if ((n = read(newfd, request_header, MAX_HEADER_REQUEST)) <= 0){
        exit(1);
      }
      request_header[n] = '\0';
      fname = request_header;
      /* check if it's GET or POST and skip over it */
      if (request_header[0] == 'G'){
        /* skip over GET, the space, and the slash */
        fname += 5;
        method = "GET";
      } else if (request_header[0] == 'P'){
        /* skip over POST, the space, and the slash */
        fname += 6;
        method = "POST";
      }
      /* end the fname at the first space encountered */
      n = 0;
      while (fname[n] != ' ') n++;
      fname[n] = '\0';
      printf(" \e[1;34m*\e[0;0m %s (%s) %s\n", method, s, fname);
      /* OPEN THE FILE! */
      fp = fopen(fname, "rb");
      if (!fp){
        fname = "conf/404.html";
        fp = fopen(fname, "rb");
        code = "404 Not Found";
      }
      /* fetch the file extension */
      fext = getext(fname);
      /* get the files size */
      struct stat st;
      stat(fname, &st);
      content_length += st.st_size;
      /* search for an adequate mime type for the extension */
      const char *mime_type = "text/plain";
      for (struct mime *m = mimes; m->ext != NULL; m++){
        if (!strcmp(m->ext, fext)){
          mime_type = m->type;
          break;
        }
      }
      /* if the "file" is a directory print some nice-looking page with
       * hyperlinks of the files and stuff */
      if (isdir(fname)){
        send_directory(newfd, fname);
      } else {
        snprintf(content_length_str, 7, "%d", content_length);
        /* print the header response */
        send_header(newfd, code, content_length_str, mime_type);
        send_file(newfd, fp);
      }
      close(newfd);
      exit(0);
    }
    /* parent doesn't need this */
    close(newfd);
  }
  /* tidy up the configuration */
  confDestroy(&conf);

  return 0;
}

void send_header(int fd, const char *code, char *content_length_str, const char *mime_type)
{
  write(fd, "HTTP/1.1 ", 9);
  write(fd, code, strlen(code));
  write(fd, "\r\nContent-length: ", 18);
  write(fd, content_length_str, strlen(content_length_str));
  write(fd, "\n", 1);
  write(fd, "Content-type:", 13);
  write(fd, mime_type, strlen(mime_type));
  write(fd, "\r\n\r\n", 4);
}

void send_file(int fd, FILE *fp)
{
  char line[255];
  int line_read_size;
  while ((line_read_size = fread(line, sizeof(char), 255, fp)) != 0){
    write(fd, line, line_read_size);
  }
}

void send_directory(int fd, char *fname)
{
  struct dirent *entry;
  unsigned content_length = 0;
  char content_length_str[7];
  /* remove the tailing slash if present */
  if (fname[strlen(fname) - 1] == '/'){
    fname[strlen(fname) - 1] = '\0';
  }
  /* open the directory */
  DIR *dp;
  if ((dp = opendir(fname)) == NULL){
    perror("opendir");
    exit(1);
  }
  /* for the first time, we need the total length of the entries */
  while ((entry = readdir(dp))){
    if (!strcmp(".", entry->d_name))
      continue;

    content_length +=
      /* "<a href='" */
      9 +
      /* the directory's name */
      strlen(fname) +
      /* "/" */
      2 +
      /* the file's name */
      strlen(entry->d_name) +
      /* "'>" */
      2 +
      /* the filename again */
      strlen(entry->d_name) +
      /* "</a><br>" */
      8;
  }
  closedir(dp);

  const char *html_header_1part =
    "<!doctype>"
    "<html>"
      "<head>"
        "<title>Listing of "
    ;

  const char *html_header_2part =
        "</title>"
        "<style type='text/css'>"
          "a, a:active, a:visited { text-decoration: none; color: #336699; }"
          "a:hover { text-decoration: none; color: #993333; }"
        "</style>"
      "</head>"
      "<body>"
    ;
  const char *html_footer =
      "</body>"
    "</html>"
    ;

  content_length += strlen(html_header_1part);
  content_length += strlen(html_header_2part);
  content_length += strlen(html_footer);

  sprintf(content_length_str, "%d", content_length);
  send_header(fd, "200 OK", content_length_str, "text/html");
  /* for the second time, we actually print the entries */
  write(fd, html_header_1part, strlen(html_header_1part));
  write(fd, fname, strlen(fname));
  write(fd, html_header_2part, strlen(html_header_2part));
  char line[255];
  if ((dp = opendir(fname)) == NULL){
    perror("opendir");
    exit(1);
  }
  while ((entry = readdir(dp))){
    if (!strcmp(".", entry->d_name))
      continue;

    sprintf(line, "<a href='/%s/%s'>%s</a><br>", fname, entry->d_name, entry->d_name);
    write(fd, line, strlen(line));
  }
  write(fd, html_footer, strlen(html_footer));
  closedir(dp);
}

void sigchld_handler(int s)
{
  /* unused parameter */
  (void)s;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

