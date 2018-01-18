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
#ifdef USE_QUERY
#include "query.h"
#endif

#ifdef USE_GENERATE
extern essential_t es;
extern network_t *networks;
#ifdef USE_QUERY
extern trigger_t *triggers;
extern activequery_t *queries;
#endif

#ifdef USE_QUERY
void QueryGenerateServers(void)
{
  trigger_t *trig;
#if 0
  struct tm *t;
	time_t long_time;
#endif
  double d;

  d = DoubleTime();

	if (d < es.nextoutput)
    return;

#ifdef SUPERDEBUG
  superdebug("QueryGenerateServers()\n");
#endif

#if 0
	time(&long_time);
	t = localtime(&long_time);
  if (!t)
    es.nextoutput = d + (60 * 15);
  else
  {
    if (t->tm_hour >= 19 && t->tm_hour <= 23)
      es.outrate = (60 * 4);
    else if (t->tm_hour >= 15 && t->tm_hour <= 18)
      es.outrate = (60 * 6);
    else if (t->tm_hour >= 12 && t->tm_hour <= 14)
      es.outrate = (60 * 8);
    else if (t->tm_hour >= 0 && t->tm_hour <= 3)
      es.outrate = (60 * 6);
    else if (t->tm_hour >= 4 && t->tm_hour <= 8)
      es.outrate = (60 * 30);
    else
      es.outrate = (60 * 10);
  }
#else
  es.outrate = (60 * 4);
#endif
  es.nextoutput = d + es.outrate;

  for (trig = triggers; trig; trig = trig->next)
  {
    if (trig->flags & QUERYFLAG_GENERATEPAGE)
    {
      QueryGameServer(&queries, trig, NULL, NULL, NULL, 1);
    }
  }
}
#endif

#define MAX_IRC_TEMPLATES 4
#define TPL_PAGE 0
#define TPL_NETWORK 1
#define TPL_CHANNEL 2
#define TPL_USER 3

void GenerateNetworkPage(void)
{
  FILE *html;
  network_t *net;
  channel_t *chan;
  nick_t *nick;
  user_t *user;
  int i;
  int totalusers;
  int usercount;
  double d;
  char *templates[MAX_IRC_TEMPLATES];
  char *names[MAX_IRC_TEMPLATES] = {"page.tpl", "network.tpl", "channel.tpl", "user.tpl"};
  char *content[MAX_IRC_TEMPLATES];
  vital_t vital[MAX_IRC_TEMPLATES];
  hashtable_t *values;

#ifdef SUPERDEBUG
  superdebug("GenerateNetworkPage()\n");
#endif

  if (!es.outpath || !es.outext)
    return;

  d = DoubleTime();

	if (d < es.nextnetgen)
    return;
    
  values = NewHashTable(32);
  if (!values)
    return;

#ifdef SUPERDEBUG
  superdebug("GenerateNetworkPage(): check ok\n");
#endif

  for (i = 0; i < MAX_IRC_TEMPLATES; i++)
  {
    templates[i] = NULL;
    content[i] = NULL;
    vital[i].pos = 0;
    vital[i].sz = 0;
  }
  
  html = fopen(va("%s%s%s", es.outpath, "irc", es.outext), "wt");
  if (!html)
    goto _err;
    
#ifdef SUPERDEBUG
  superdebug("GenerateNetworkPage(): begin\n");
#endif

  for (i = 0; i < MAX_IRC_TEMPLATES; i++)
  {
    templates[i] = ReadTemplate(va("%s%s%s/%s", es.wdir, CONFIG_TEMPLATEDIR, "irc", names[i]));
    if (!templates[i])
      goto _err;
  }

  for (net = networks; net; net = net->next)
  {
    totalusers = 0;

    if (net->state == STATE_CONNECTED)
    {
      for (chan = net->channels; chan; chan = chan->next)
      {
        usercount = 0;

        if (chan->lastjoin == 0.0)
        {
          for (nick = chan->nicks; nick; nick = nick->next)
          {
            user = nick->user;
            if (!user)
              continue;

            AddValue(values, "nick", va("%s", user->nick), 1);
            AddValue(values, "ident", va("%s", user->ident), 1);
            AddValue(values, "address", va("%s", user->address), 1);
            AddValue(values, "ircop", va("%d", user->flags & USERFLAG_IRCOP), 0);
            AddValue(values, "chanop", va("%d", nick->flags & NICKFLAG_CHANOP), 0);
            AddValue(values, "halfop", va("%d", nick->flags & NICKFLAG_HALFOP), 0);
            AddValue(values, "voice", va("%d", nick->flags & NICKFLAG_VOICED), 0);

            ProcessTemplate(&content[TPL_USER], &vital[TPL_USER], templates[TPL_USER], values);
            HashClear(values);
            usercount++;
          }


          AddValue(values, "users", content[TPL_USER], 0);
        }

        AddValue(values, "name", va("%s", chan->name), 1);
        AddValue(values, "onchan", va("%d", (chan->lastjoin == 0.0)), 0);
        AddValue(values, "topic", va("%s", chan->topic), 1);
        AddValue(values, "topicsetby", va("%s", chan->topicsetby), 1);
        AddValue(values, "modes", va("%s", chan->modes), 1);
        AddValue(values, "usercount", va("%d", usercount), 0);

        ProcessTemplate(&content[TPL_CHANNEL], &vital[TPL_CHANNEL], templates[TPL_CHANNEL], values);
        HashClear(values);
        ClearContent(&content[TPL_USER], &vital[TPL_USER]);
      }

      for (user = net->users; user; user = user->next)
      {
        totalusers++;
      }

      AddValue(values, "hostname", va("%s", net->hostname), 1);
      AddValue(values, "mynick", va("%s", net->mynick), 1);
      AddValue(values, "totalusers", va("%d", totalusers), 0);
      AddValue(values, "channels", content[TPL_CHANNEL], 0);
    }

    AddValue(values, "group", va("%s", net->group), 1);
    AddValue(values, "address", va("%s", (net->state == STATE_CONNECTED) ? inet_ntoa(net->connectaddr.sin_addr) : "not connected"), 1);
    AddValue(values, "port", va("%d", (net->state == STATE_CONNECTED) ? ntohs(net->connectaddr.sin_port) : net->state), 0);

    ProcessTemplate(&content[TPL_NETWORK], &vital[TPL_NETWORK], templates[TPL_NETWORK], values);
    HashClear(values);
    ClearContent(&content[TPL_CHANNEL], &vital[TPL_CHANNEL]);
  }

  AddValue(values, "networks", content[TPL_NETWORK], 0);
  AddValue(values, "lastupdate", va("%ld", time(NULL)), 0);

  ProcessTemplate(&content[TPL_PAGE], &vital[TPL_PAGE], templates[TPL_PAGE], values);
  HashClear(values);

  fprintf(html, "%s", content[TPL_PAGE]);

_err:
  if (html)
    fclose(html);
  RemoveHashTable(&values);

  for (i = 0; i < MAX_IRC_TEMPLATES; i++)
  {
    if (templates[i])
      _free_(templates[i]);
    if (content[i])
      _free_(content[i]);
  }

  es.nextnetgen = d + TIME_NETWORK_GENERATE;

#ifdef SUPERDEBUG
  superdebug("GenerateNetworkPage(): done\n");
#endif
}

#define MAX_GAME_TEMPLATES 4
#define TPL_PAGE 0
#define TPL_SERVER 1
#define TPL_PLAYER 2
#define TPL_TIMEOUT 3

#ifdef USE_QUERY
int GeneratePage(activequery_t *query)
{
  FILE *html;
  result_t *res, *high;
  player_t *p;
  int r, i, top;
  char *templates[MAX_GAME_TEMPLATES];
  char *content[MAX_GAME_TEMPLATES];
  const char *names[MAX_GAME_TEMPLATES] = {"page.tpl", "server.tpl", "player.tpl", "timeouts.tpl"};
  vital_t vital[MAX_GAME_TEMPLATES];
  hashtable_t *values;

#ifdef SUPERDEBUG
  superdebug("GeneratePage()\n");
#endif

  if (!es.outurl || !es.outpath || !es.outext || !query || query->trigger->custom)
    return 0;

  r = 0; 
  values = NewHashTable(64);
  if (!values)
    return 0;
   
  for (i = 0; i < MAX_GAME_TEMPLATES; i++)
  {
    templates[i] = NULL;
    content[i] = NULL;
    vital[i].pos = 0;
    vital[i].sz = 0;
  }

#ifdef SUPERDEBUG
  superdebug("GeneratePage(): check ok\n");
#endif

  html = fopen(va("%s%s%s", es.outpath, query->trigger->name, es.outext), "wt");
  if (!html)
    goto _err;

#ifdef SUPERDEBUG
  superdebug("GeneratePage(): begin\n");
#endif

  for (i = 0; i < MAX_GAME_TEMPLATES; i++)
  {
    templates[i] = ReadTemplate(va("%s%s%s/%s", es.wdir, CONFIG_TEMPLATEDIR, query->trigger->name, names[i]));
    if (!templates[i])
      templates[i] = ReadTemplate(va("%s%s%s/%s", es.wdir, CONFIG_TEMPLATEDIR, "default", names[i]));
    if (!templates[i])
      goto _err;
  }

  r = 1; //return
  
  do
  {
    top = -99999;
    high = NULL;

    for (res = query->results; res; res = res->next)
    {
      if (res->processed)
        continue;

      i = (!res->up) ? 0 : res->info.numplayers;
      if (i > top)
      {
        high = res;
        top = i;
      }
    }
     
    if (high)
    {
      if (!high->up || !high->info.hostname[0])
      {
        AddValue(values, "address", va("%s", high->info.ip), 1);
        AddValue(values, "queryport", va("%d", high->info.queryport), 0);

        ProcessTemplate(&content[TPL_TIMEOUT], &vital[TPL_TIMEOUT], templates[TPL_TIMEOUT], values);
        HashClear(values);
      }
      else
      {
        for (i = 0; i < high->info.numplayers; i++)
        {
          p = high->info.players[i];

          if (p)
          {
            AddValue(values, "name", va("%s", p->name), 1);
            AddValue(values, "score", va("%d", p->score), 0);
            //AddValue(values, "frags", va("%d", p->frags), 0);
            AddValue(values, "deaths", va("%d", p->deaths), 0);
            AddValue(values, "ping", va("%d", p->ping), 0);
            AddValue(values, "pid", va("%s", p->pid), 1);
            AddValue(values, "team", va("%s", p->team), 1);

            ProcessTemplate(&content[TPL_PLAYER], &vital[TPL_PLAYER], templates[TPL_PLAYER], values);
            HashClear(values);
          }
        }

        AddValue(values, "address", va("%s", high->info.ip), 1);
        AddValue(values, "queryport", va("%d", high->info.queryport), 0);
        AddValue(values, "gameport", va("%d", high->info.gameport), 0);
        AddValue(values, "hostname", va("%s", high->info.hostname), 1);
        AddValue(values, "numplayers", va("%d", high->info.numplayers), 0);
        AddValue(values, "maxplayers", va("%d", high->info.maxplayers), 0);
        AddValue(values, "map", va("%s", high->info.map), 1);
        AddValue(values, "mod", va("%s", high->info.mod), 1);
        AddValue(values, "ranked", va("%s", (high->info.flags & GAME_FLAG_RANKED) ? "yes" : "no"), 0);
        AddValue(values, "version", va("%s", high->info.version), 1);
        AddValue(values, "password", va("%s", (high->info.flags & GAME_FLAG_PASSWORD) ? "yes" : "no"), 0);
        AddValue(values, "players", content[TPL_PLAYER], 0);

        ProcessTemplate(&content[TPL_SERVER], &vital[TPL_SERVER], templates[TPL_SERVER], values);
        HashClear(values);
        ClearContent(&content[TPL_PLAYER], &vital[TPL_PLAYER]);
      }

      high->processed = 1;
    }
  } while (high);

  AddValue(values, "servers", content[TPL_SERVER], 0);
  AddValue(values, "timeouts", content[TPL_TIMEOUT], 0);
  AddValue(values, "lastupdate", va("%ld", time(NULL)), 0);
  AddValue(values, "trigger", query->trigger->name, 1);
  AddValue(values, "generate", va("%d", (query->flags & QUERYFLAG_GENERATEPAGE) ? 1 : 0), 0);
  AddValue(values, "replys", va("%d", query->numservers), 0);
  AddValue(values, "updaterate", va("%d", es.outrate), 0);

  ProcessTemplate(&content[TPL_PAGE], &vital[TPL_PAGE], templates[TPL_PAGE], values);
  HashClear(values);

  fprintf(html, "%s", content[TPL_PAGE]);

_err:
  if (html)
    fclose(html);
  RemoveHashTable(&values);

  for (i = 0; i < MAX_GAME_TEMPLATES; i++)
  {
    if (templates[i])
      _free_(templates[i]);
    if (content[i])
      _free_(content[i]);
  }

#ifdef SUPERDEBUG
  superdebug("GeneratePage(): done\n");
#endif

  return r;
}
#endif

char *ReadTemplate(char *tpl)
{
  FILE *fp;
  char *section, *s;
  char c;
  int i, len;

#ifdef SUPERDEBUG
  superdebug("ReadTemplate()\n");
#endif

  fp = fopen(tpl, "rt");
  if (!fp)
    return NULL;

  i = 0;
  len = SECTION_CHUNK;

  section = (char *)_malloc_(len);
  if (section)
  {
    while (1)
    {
      c = fgetc(fp);
      if (c == EOF)
        break;

      if (i >= len)
      {
        s = (char *)_realloc_(section, len + SECTION_CHUNK);
        if (s)
        {
          section = s;
          len += SECTION_CHUNK;
        }
      }

      if (i < len)
      {
        section[i++] = c;
      }
    }

    section[i] = '\0';
  }

  fclose(fp);

  return section;
}

void ProcessTemplate(char **section, vital_t *vital, char *tpl, hashtable_t *values)
{
  char key[MAX_HASH_NAME+1];
  char *val;
  int sp, kp, sv;
  int ignore, len;

#ifdef SUPERDEBUG
  superdebug("ProcessTemplate()\n");
#endif
 
  sp = 0;
  ignore = 0;
  len = strlen(tpl);

  while (sp < len)
  {
    if (tpl[sp] == '$' && tpl[sp+1] == '{' && (tpl[(sp) ? sp-1 : 0] != '\\'))
    {
      sv = sp;
      kp = 0;
      sp += 2;

      while (sp < len && tpl[sp] != '}')
      {
        if (kp < sizeof(key))
          key[kp++] = tpl[sp];
        sp++;
      }

      if (kp && sp < len)
      {
        key[kp] = '\0';
        if (!strncmp(key, "IF ", 3))
          ignore = !isstrset(values, key + 3);
        else if (!strcmp(key, "EL"))
          ignore = !ignore;
        else if (!strcmp(key, "FI"))
          ignore = 0;
        else if (!ignore)
        {
          val = (char *)HashGet(values, key);
          if (val)
            append(section, &vital->pos, &vital->sz, val, strlen(val));
        }
      }
      else
      {
        sp = sv;
        goto _next;
      }
    }
    else if (tpl[sp] == '\\')
    {
      if (sp && tpl[sp-1] == '\\')
        goto _next;
    }
    else
    {
_next:
      if (!ignore)
        append(section, &vital->pos, &vital->sz, &tpl[sp], 1);
    }

    sp++;
  }

  //(*section)[vital->pos] = '\0';  <--- EEK don't do that, if there's not enough room it runs off the edge of the world
  append(section, &vital->pos, &vital->sz, "\0", 1);
  vital->pos--; //pretend that we didn't terminate it
}

void ClearContent(char **content, vital_t *vital)
{
#ifdef SUPERDEBUG
  superdebug("ClearContent()\n");
#endif

  if (*content)
  {
    _free_(*content);
    *content = NULL;
  }
  vital->pos = 0;
  vital->sz = 0;
}

void AddValue(hashtable_t *hash, char *name, char *val, int html)
{
  any_t *a;
  char *s;
  int i, len;

#ifdef SUPERDEBUG
  superdebug("AddValue()\n");
#endif

  if (!val || !*val)
    return;

  a = HashAdd(hash, name);
  if (!a)
    return;

  if (!html)
  {
    len = strlen(val);
  }
  else
  {
    len = 0;
    s = val;

    while (*s)
    {
      switch (*s)
      {
        case '<': len += 4; break; // &lt;
        case '>': len += 4; break; // &gt;
        case '&': len += 5; break; // &amp;
        case '\'': len += 5; break; //&#39;
        case '\"': len += 6; break; // &quot;
        case '\\': len += 5; break; //&#92;
        default: len++; break;
      }

      s++;
    }
  }

  len++;
  s = (char *)_malloc_(len);
  if (s)
  {
    if (!html)
    {
      strlcpy(s, val, len);
    }
    else
    {
      for (i = 0; i < len && *val; i++, val++)
      {
        switch (*val)
        {
          case '<': memcat(s, i, "&lt;", 4, len); i += 3; break;
          case '>': memcat(s, i, "&gt;", 4, len); i += 3; break;
          case '&': memcat(s, i, "&amp;", 5, len); i += 4; break;
          case '\'': memcat(s, i, "&#39;", 5, len); i += 4; break;
          case '\"': memcat(s, i, "&quot;", 6, len); i += 5; break;
          case '\\': memcat(s, i, "&#92;", 5, len); i += 4; break;
          default: s[i] = *val; break;
        }
      }

      s[(i < len) ? i : len] = '\0';
    }
  }

  a->value = s;
}

void append(char **src, int *srcpos, int *srcmax, char *dest, int destlen)
{
  int i, j;
  char *s;

#ifdef SUPERDEBUG
  superdebug("append()\n");
#endif

  for (i = 0; i < destlen; i++)
  {
    if (*srcpos >= *srcmax)
    {
      j = *srcmax + MAX_SECTION_CHUNK;
      s = _realloc_(*src, j);
      if (s)
      {
        *src = s;
        *srcmax = j;
      }
      else
      {
#ifdef _DEBUG
        printf("uh, realloc failed\n");
#endif
      }
    }

    if (*srcpos < *srcmax)
    {
      (*src)[(*srcpos)++] = dest[i];
    }
    else
    {
#ifdef _DEBUG
      printf("no!\n");
#endif
    }
  }
}

#endif // USE_GENERATE

