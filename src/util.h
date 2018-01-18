/*
  Copyright (C) 2003-2008 Chris Cuthbertson

  This file is part of crank.

  crank is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  crank is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with crank.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _H_UTIL
#define _H_UTIL

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef WIN32
#include <winsock.h>
#endif

#ifdef UNIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define MAX_BUFFER_SIZE 2048
#define MAX_BUFFERS 2

#ifdef WIN32
#define _strnprintf _snprintf
#define _vstrnprintf _vsnprintf
#define mkdir _mkdir
#define _close(s) closesocket(s)
#define _sleep(t) Sleep(t)
#define strncasecmp strnicmp
#define socklen_t int
#define _mutex CRITICAL_SECTION
#define __endthread return 0
#define __threadvoid LPVOID
#define __InitMutex(m) InitializeCriticalSection(m)
#define __LockMutex(m) EnterCriticalSection(m)
#define __UnlockMutex(m) LeaveCriticalSection(m)
#define __DestroyMutex(m) DeleteCriticalSection(m)
#endif
#ifdef UNIX
#define _strnprintf snprintf
#define _vstrnprintf vsnprintf
#define stricmp strcasecmp
#define _close(s) close(s)
#define _malloc_ malloc
#define _free_ free
#define _strdup_ strdup
#define _realloc_ realloc
#define _sleep(t) usleep(t * 1000)
#define _mutex pthread_mutex_t
#define __endthread pthread_exit(NULL)
#define __threadvoid void
#define __InitMutex(m) pthread_mutex_init(m, NULL)
#define __LockMutex(m) pthread_mutex_lock(m)
#define __UnlockMutex(m) pthread_mutex_unlock(m)
#define __DestroyMutex(m) pthread_mutex_destroy(m)
#define MAX_PATH 260
#endif

char *va(char *fmt, ...);
int match(char *name, char *mask);
char *workingdir(char *path);
char *randch(char *s, int sz);
void destroy(char *p, char *dec, int sz);
char *sctime(char *num);
char *listnext(char *list, char **ip, int *port);
void splitmsg(char *msg, char **text, char **game, char **list);
void removeinclosed(char *s, char c);
int compareaddr(struct sockaddr_in *a, struct sockaddr_in *b);
unsigned long atolz(char *s);
int memcat(char *dst, int dstlen, const char *src, int srclen, int siz);
int validip(char *s);
char *resolve(char *host, int *start);
char *lookup(char *ip);
void timelength(double t, char *up, int sz);

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);

#ifdef SUPERDEBUG
void superdebug(char *msg);
#endif

#endif
