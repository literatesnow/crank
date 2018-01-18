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

#include "crank.h"

int hashstr(char *str, int sz)
{
  unsigned int h;
  int c;

  h = 5381;
  while ((c = *str++))
    h = ((h << 5) + h) + c;

  return (int)(h % sz);
}

int isstrset(hashtable_t *hash, char *key)
{
  char *s;

  s = (char *)HashGet(hash, key);
  if (!s)
    return 0;

  if (*s == '\0')
    return 0;

  if (*s == '0')
    return 0;

  return 1;
}

int isstrequals(hashtable_t *hash, char *key, char *value)
{
  char *p;

  p = (char *)HashGet(hash, key);
  if (!p)
    return 0;

  if (!stricmp(p, value))
    return 1;

  return 0;
}

void HashStrSet(hashtable_t *hash, char *key, char *value)
{
  any_t *a;

  a = HashAdd(hash, key);
  if (!a)
    return;

  if (a->value)
    _free_(a->value);

  a->value = (value) ? _strdup_(value) : NULL;
}

void *HashGet(hashtable_t *hash, char *name)
{
  any_t *any;

  any = HashGetItem(hash, name);
  if (!any)
    return NULL;

  return any->value;
}

any_t *HashGetItem(hashtable_t *hash, char *name)
{
  any_t **p, *q;

  p = &hash->table[hashstr(name, hash->size)];
  if (*p)
  {
    for (q = *p; q; q = q->next)
    {
      //MessageBox(NULL, q->name, name, MB_OK);
      if (!stricmp(q->name, name))
        return q;
    }
  }

  return NULL;
}

any_t *HashAdd(hashtable_t *hash, char *name)
{
  any_t **p, *q;

  q = HashGetItem(hash, name);
  if (q)
    return q;

  p = &hash->table[hashstr(name, hash->size)];
  q = NewAny(p, &hash->head);
  if (q)
  {
    strlcpy(q->name, name, sizeof(q->name));
    q->value = NULL;

    hash->count++;
  }

  return q;
}

void HashRemove(hashtable_t *hash, char *name)
{
  any_t **p, *q;

  p = &hash->table[hashstr(name, hash->size)];
  if (!*p)
    return;

  for (q = *p; q; q = q->next)
  {
    if (!stricmp(q->name, name))
    {
      if (q->prev)
        q->prev->next = q->next;
      if (q->next)
        q->next->prev = q->prev;
      if (!q->prev)
        *p = q->next;

      if (q->prevany)
        q->prevany->nextany = q->nextany;
      if (q->nextany)
        q->nextany->prevany = q->prevany;
      if (!q->prevany)
        hash->head = q->nextany;

      if (q->value)
        _free_(q->value);

      _free_(q);

      hash->count--;

      return;
    }
  }
}

void HashClear(hashtable_t *hash)
{
  any_t **top, *p;
  int i;

  for (i = 0; i < hash->size; i++)
  {
    top = &hash->table[i];
    while (*top)
    {
      p = *top;
      *top = (*top)->next;
      if (p->value)
        _free_(p->value);
      _free_(p);
    }
    hash->table[i] = NULL;
  }

  hash->head = NULL;
  hash->count = 0;
}

hashtable_t *NewHashTable(int sz)
{
  hashtable_t *hash;
  int i;

  hash = (hashtable_t *)_malloc_(sizeof(hashtable_t));
  if (!hash)
    return NULL;

  hash->table = (any_t **)_malloc_(sz * sizeof(any_t *));
  if (!hash->table)
  {
    _free_(hash);
    return NULL;
  }

  for (i = 0; i < sz; i++)
    hash->table[i] = NULL;  

  hash->head = NULL;
  hash->size = sz;
  hash->count = 0;

  return hash;
}

any_t *NewAny(any_t **parent, any_t **head)
{
  any_t *any;

  any = (any_t *)_malloc_(sizeof(any_t));
  if (!any)
    return NULL;

  any->name[0] = '\0';
  any->value = NULL;

  if (*head)
    (*head)->prevany = any;
  any->nextany = *head;
  any->prevany = NULL;
  *head = any; 

  if (!parent)
    return any;

  if (*parent)
    (*parent)->prev = any;
  any->next = *parent;
  any->prev = NULL;
  *parent = any;

  return *parent;
}

void RemoveHashTable(hashtable_t **hash)
{
  if (!*hash)
    return;

  if ((*hash)->table)
  {
    HashClear(*hash);
    _free_((*hash)->table);
  }
  _free_(*hash);
}
