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

#include "util.h"

char *va(char *fmt, ...)
{
  static char buf[MAX_BUFFER_SIZE+1];
  va_list argptr;

	va_start(argptr, fmt);
	_vstrnprintf(buf, MAX_BUFFER_SIZE, fmt, argptr);
	va_end(argptr);
  buf[MAX_BUFFER_SIZE-1] = '\0';

  return buf;
}

//match() - written by binary
//http://www.koders.com/c/fidCBE3A292A3132154295D87B5CBD159AD012C5F64.aspx
#define	lc(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 'a' - 'A') : c)
int match(char *name, char *mask)
{
  register char *m;
  register char *n;
  register char cm;
  char *wsn;
  char *wsm;

#ifdef SUPERDEBUG
  superdebug("match()\n");
#endif
  
  m = mask;
  cm = *m;

  n = name;  
  if (cm == '*')
  {
    if (m[1] == '\0') //mask is just "*", so true
      return 0;
  }
  else if (cm != '?' && lc(cm) != lc(*n))
  {
    return 1; //most likely first chars won't match
  }
  else
  {
    m++;
    n++;
  }

  cm = lc(*m);
  wsm = NULL;

  while (1)
  {
    if (cm == '*') //found the * wildcard
    {
      m++;  //go to next char of mask
      if (!*m) //if at end of mask,
        return 0; //function becomes true.

      while (*m == '*') //while the char at m is "*"
      {
        m++; //go to next char of mask
        if (!*m) //if at end of mask,
          return 0; //function becomes true.
      }

      cm = *m;
      if (cm == '\\') //don't do ? checking if a "\"
      {
        cm = *(++m); //just skip this char, no ? checking
      }
      else if (cm == '?') //if it's a ?
      {
        do
        {
          m++; //go to the next char of both
          n++;
          if (!*n) //if end of test string...
            return (!*m ? 0 : 1); //true if end of mask str, else false
        } while (*m == '?'); //while we have ?'s
        cm = *m;
        if (!cm) //last char of mask is ?, so it's true
          return 0;
      }

      cm = lc(cm);
      while (lc(*n) != cm)
      { //compare
        n++; //go to next char of n
        if (!*n) //if at end of n string
          return 1; //function becomes false.
      }
      wsm = m; //mark after where wildcard found
      m++;
      cm = lc(*m); //go to next mask char
      wsn = n; //mark spot first char was found
      n++; //go to next char of n
      continue;
    }

    if (cm == '?') //found ? wildcard
    {
      m++;
      //m++;
      cm = lc(*m); //just skip and go to next
      n++;
      if (!*n) //return true if end of both,
        return (cm ? 1 : 0); //false if end of test str only
      continue;
    }

    if (cm == '\\') // next char will not be a wildcard.
    { //skip wild checking, don't continue
      m++;
      cm = lc(*m);
      n++;
    }

    //Complicated to read, but to save CPU time.  Every ounce counts.
    // --- de-registere'd, it's 2005, we've got plenty of cpu
    // --- ^ oh man, great idea pal
    if (lc(*n) != cm) //if the current chars don't equal,
    {
      if (!wsm) //if there was no * wildcard,
        return 1; //function becomes false.
      n = wsn + 1; //start on char after the one we found last
      m = wsm; //set m to the spot after the "*"
      cm = lc(*m);
      while (cm != lc(*n))
      {  //compare them
        n++; //go to next char of n
        if (!*n) //if we reached end of n string,
          return 1; //function becomes false.
      }
      wsn = n; //mark spot first char was found
    }
    if (!cm) //cm == cn, so if !cm, then we've
      return 0; //reached end of BOTH, so it matches
    m++; //go to next mask char
    n++; //go to next testing char
    cm = lc(*m); //pointers are slower
  }
}

char *workingdir(char *path)
{
#if 0
  char *p, *c;

  if (*path == '"')
    path++;

  p = path;
  c = path;

  while (*p && *p != ' ')
  {
#ifdef WIN32
    if (*p == '\\')
#endif
#ifdef UNIX
    if (*p == '/')
#endif
      c = ++p;
    else
      p++;
  }
  *c = '\0';
#endif
  
  char *s, *p;
  int i;

  s = path;
  if (*s == '\"')
    s++;

  i = strlen(s);
  p = &path[(i > 0) ? i - 1 : i];

  while (i > 0 &&
#ifdef WIN32
    *p != '\\'
#endif
#ifdef UNIX
    *p != '/'
#endif
    )
  {
    p--;
    i--;
  }

  *(p + 1) = '\0';

  return s;
}

int validip(char *s)
{
  int i;
  int dot;

#ifdef SUPERDEBUG
  superdebug("validip()\n");
#endif

  i = 0;
  dot = 0;
  while (*s)
  {
    if (!((*s >= 48 && *s <= 57) || (*s == 46)))
      return 0;
    if (*s == 46)
      dot++;
    i++;
    s++;
  }
  if ((dot == 3) && (i < 16))
    return 1;

  return 0;
}

char *randch(char *s, int sz)
{
  int i;
  char *p;

#ifdef SUPERDEBUG
  superdebug("randch()\n");
#endif

  srand(time(NULL));

  for (p = s, i = 0; i < (sz - 1); i++)
    *p++ = /*(i % 2 == 0) ? ((*/rand() % 10/*)*/ + '0';/* : ((rand() % 26) + 'a');*/
  *p = '\0';

  return s;
}

char *sctime(char *num)
{
  time_t t;

#ifdef SUPERDEBUG
  superdebug("sctime()\n");
#endif

  t = atolz(num);
  return ctime(&t);
}

char *listnext(char *list, char **ip, int *port)
{
  char *p;

#ifdef SUPERDEBUG
  superdebug("listnext()\n");
#endif

  if (!list || !*list)
    return NULL;

  *ip = list;
  while (*list && *list != ' ')
    list++;
  if (*list)
    *list++ = '\0';
  p = strchr(*ip, ':');
  if (!p)
    *port = 0;
  else
  {
    *port = atoi(&(*ip)[p - (*ip) + 1]);
    *p = '\0';
  }
  while (*list && *list == ' ')
    list++;

  return list;
}

void splitmsg(char *msg, char **text, char **game, char **list)
{
  char *s;
  int i;

#ifdef SUPERDEBUG
  superdebug("splitmsg()\n");
#endif

  i = 0;
  s = msg;
  *text = s;
  *game = NULL;
  *list = NULL;

  for (; *s; s++)
  {
    if (*s == ' ')
    {
      if (!i)
      {
        *s++ = '\0';
        *game = s;
        i++;
      }
      else
      {
        *s++ = '\0';
        *list = s;
        break;
      }
    }
  }
}

void removeinclosed(char *s, char c)
{
  char *p;

#ifdef SUPERDEBUG
  superdebug("removeinclosed()\n");
#endif

  if (!*s || (*s != c))
    return;

  for (p = s + 1; *p && *p != c;)
    *s++ = *p++;
  *s = '\0';
}

int compareaddr(struct sockaddr_in *a, struct sockaddr_in *b)
{
#ifdef SUPERDEBUG
  //superdebug("compareaddr()\n");
#endif

  if ((a->sin_addr.s_addr == b->sin_addr.s_addr) && (a->sin_port == b->sin_port))
    return 1;
  return 0;
}

int memcat(char *dst, int dstlen, const char *src, int srclen, int siz)
{
#ifdef SUPERDEBUG
  superdebug("memcat()\n");
#endif

  if (dstlen + srclen > siz)
    return 0;

  dst += dstlen;

  memcpy(dst, src, srclen);
  dst[srclen] = 0;

  return 1;
}

void timelength(double t, char *up, int sz)
{
  int days, hours, mins, secs;

  days = (int)floor(t / 60 / 60 / 24);
  t -= days * 60 * 60 * 24;
  hours = (int)floor(t / 60 / 60);
  t -= hours * 60 * 60;
  mins = (int)floor(t / 60);
  t -= mins * 60;
  secs = (int)t;

  up[0] = '\0';
  if (days > 0)
    strlcat(up, va("%dD", days), sz);
  if (hours > 0)
    strlcat(up, va("%s%dh", (up[0]) ? " " : "", hours), sz);
  if (mins > 0)
    strlcat(up, va("%s%dm", (up[0]) ? " " : "", mins), sz);
  if (secs > 0)
    strlcat(up, va("%s%ds", (up[0]) ? " " : "", secs), sz);
  if (!up[0])
    _strnprintf(up, sz, "%0.2fs", t);
}

unsigned long atolz(char *s)
{
	unsigned long val;
	int c;

#ifdef SUPERDEBUG
  superdebug("atolz()\n");
#endif
	
	if (*s == '-')
		return 1;
		
	val = 0;

	while (1)
  {
		c = *s++;
		if (c < '0' || c > '9')
			return val;
		val = val * 10 + c - '0';
	}
	
	return 1;
}

char *resolve(char *host, int *start)
{
	struct hostent *hp;
  char *addr;
  int end;

  hp = gethostbyname(host);
  if (!hp)
    return NULL;

  if (!hp->h_addr_list[0])
    return NULL;

  for (end = 0; hp->h_addr_list[end]; end++)
    ;

  if (*start >= end)
    *start = 0;

  addr = va("%s", inet_ntoa(*((struct in_addr *)hp->h_addr_list[*start])));
  if (((*start)++) >= end)
    *start = 0;

  return addr;
}

char *lookup(char *ip)
{
	struct hostent *hp;
  unsigned int addr;

  addr = inet_addr(ip);
  hp = gethostbyaddr((char *) &addr, 4, AF_INET);
  return va("%s", hp ? hp->h_name : "");
}

/*	$OpenBSD: strlcat.c and strlcpy.c,v 1.11 2003/06/17 21:56:24 millert Exp $
 *
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return (dlen + strlen(s));
	while (*s != '\0')
  {
		if (n != 1)
    {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return (dlen + (s - src));	/* count does not include NUL */
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0)
  {
		do
    {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
  {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return (s - src - 1);	/* count does not include NUL */
}
