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

#ifdef USE_QUERY
#include "crank.h"
#include "query.h"

extern essential_t es;
extern network_t *network;
extern trigger_t *triggers;
extern activequery_t *queries;
extern protocol_t *protocols;

protocol_t *FindGame(char *id)
{
  protocol_t *prot;

  for (prot = protocols; prot; prot = prot->next) {
    if (!strncasecmp(prot->name, id, prot->len)) {
      return prot;
    }
  }

  return NULL;
}

void ShowHelp(char *nick)
{
  char *usage = QUERY_HELP_USAGE;
  char *modifiers = QUERY_HELP_MODIFIERS;
  char *info = QUERY_HELP_INFO;

  Say(MSG_PRIVMSG, nick, usage);
  Say(MSG_PRIVMSG, nick, modifiers);
  Say(MSG_PRIVMSG, nick, info);
}

void ShowTriggers(char *channel, char *from)
{
  channel_t *chan;
  trigger_t *trig;
  char *s;
  int sz = 0;

#ifdef SUPERDEBUG
  superdebug("ShowTriggers()\n");
#endif

  chan = findchannel(network, channel);
  if (!chan)
    return;

  for (trig = triggers; trig; trig = trig->next)
  {
    if (trig->custom)
      continue;

    sz++;
  }

  if (!sz)
  {
    Say(MSG_NOTICE, from, "No triggers");
    return;
  }

  sz = ((MAX_TRIGGER_NAME + 1) * sz) + strlen(chan->name) + 14 + 1;
  s = (char *)_malloc_(sz);
  if (!s)
    return;

  _strnprintf(s, sz, "Triggers for %s:", chan->name);

  for (trig = triggers; trig; trig = trig->next)
  {
    if (trig->custom)
      continue;
    if (findquerychannel(trig, chan->name))
      strlcat(s, va(" %c%s", QUERYSYMBOL_TRIGGER, trig->name), sz);
  }

  Say(MSG_NOTICE, from, s);
  _free_(s);
}

int QueryParse(char p[][MAX_PARAMSIZE+1], void *data)
{
#ifdef SUPERDEBUG
  superdebug("QueryParse()\n");
#endif

  if (!strcmp(p[0], "trigger"))
    AddTrigger(p);
  else if (!strcmp(p[0], "server"))
    AddGameServer(p);
  else if (!strcmp(p[0], "channel"))
    AddGameChannel(p);
  else
    echo(NULL, ECHO_SHOW, va("Unknown: %s\n", p));

  return 1;
}

int ProtocolParse(char p[][MAX_PARAMSIZE+1], void *data)
{
#ifdef SUPERDEBUG
  superdebug("ProtocolParse()\n");
#endif

  if (!strcmp(p[0], "game") && strlen(p[1]) > 0) {
    protocol_t *prot = NewProtocol(&protocols);
    if (prot) {
      strlcpy(prot->name, p[1], sizeof(prot->name));
      prot->len = strlen(prot->name);
    }

  } else {
    echo(NULL, ECHO_SHOW, va("Unknown: %s\n", p));
  }

  return 1;
}

int DescriptionParse(char p[][MAX_PARAMSIZE+1], void *data)
{
  mapdesc_t *desc = (mapdesc_t *)data;

#ifdef SUPERDEBUG
  superdebug("DescriptionParse()\n");
#endif

  if (!strcmp(p[0], desc->name))
  {
    strlcpy(desc->name, p[1], sizeof(desc->name));
    _strnprintf(desc->size, sizeof(desc->size), " (%sMB)", p[2]);
    desc->match = 1;
    return 0;
  }

  return 1;
}

void MapDescription(mapdesc_t *desc, char *mapname)
{
#ifdef SUPERDEBUG
  superdebug("MapDescription()\n");
#endif

  strlcpy(desc->name, mapname, sizeof(desc->name));
  desc->size[0] = '\0';
  desc->match = 0;

  LoadConfig(CONFIG_DESC, DELIM_CONFIG, &DescriptionParse, desc);
}


void InitServerInfo(serverinfo_t *info)
{
  int i;

#ifdef SUPERDEBUG
  superdebug("InitServerData()\n");
#endif

  if (!info)
    return;

  info->hostname[0] = '\0';
  info->map[0] = '\0';
  info->type[0] = '\0';
  info->mod[0] = '\0';
  info->status[0] = '\0';
  info->version[0] = '\0';
  info->team1[0] = '\0';
  info->team2[0] = '\0';
  info->ip[0] = '\0';
  info->queryport = 0;
  info->numplayers = 0;
  info->maxplayers = 0;
  info->tickets1 = 0;
  info->tickets2 = 0;
  info->gameport = 0;
  info->flags = 0;
    
  for (i = 0; i < MAX_PLAYERS; i++)
    info->players[i] = NULL;
}

#ifdef USE_GENERATE
void QueryGameServer(activequery_t **queries, trigger_t *trig, char *from, char *nick, char *flags, int gen)
#else
void QueryGameServer(activequery_t **queries, trigger_t *trig, char *from, char *nick, char *flags)
#endif
{
  activequery_t *query;
  queryserver_t *qserv;
  FILE *fp;
  char *s;
  int i;

#ifdef SUPERDEBUG
  superdebug("QueryGameServer()\n");
#endif

  if (!trig)
    return;

  query = NewActiveQuery(queries);
  if (!query)
    return;

  query->trigger = trig;
  query->flags = mergemodifiers(trig->flags, modifiers(flags));
#ifdef USE_GENERATE
  query->generate = gen;
#endif
  query->network = network;

  if ((query->flags & QUERYFLAG_QUERYNICK) && nick) {
    from = nick;
  }

  if (from)
  {
    if (from[0] != '#')
      query->flags |= QUERYFLAG_QUERYNICK;

    query->from = _strdup_(from);
  }

  if (flags)
  {
    s = strchr(flags, QUERYSYMBOL_WILDMATCH);
    if (s && (strlen(s) > 2))
    {
      query->wildmatch = _strdup_(s);
      query->flags |= QUERYFLAG_SHOWPLAYERS;
    }
    else
    {
      query->flags &= ~QUERYFLAG_WILDMATCH;
    }
  }

  i = 0;
  _strnprintf(query->uid, sizeof(query->uid), "qstat_%d", query->id);

  //this is not in the thread incase qserv gets changed with admin commands somewhere else
  fp = fopen(va("%s%s%s.in", es.wdir, CONFIG_QSTATDIR, query->uid), "wt");
  if (fp)
  {
    for (qserv = trig->servers; qserv; qserv = qserv->next)
    {
      fprintf(fp, "%s %s", qserv->server->game, qserv->server->addr);
      if (qserv->server->port)
        fprintf(fp, ":%d", qserv->server->port);
      fprintf(fp, "\n");

      i++;
    }

    fclose(fp);
  }

  query->totalservers = i;

#ifdef WIN32
  query->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunQuery, (void *)&*query, 0, &i);
  if (!query->thread)
  {
#endif
#ifdef UNIX   
  if (pthread_create(&query->thread, NULL, &RunQuery, (void *)&*query))
  {
#endif
    echo(NULL, ECHO_SHOW, "Failed to query\n");
  }

}

void *RunQuery(__threadvoid *data)
{
  activequery_t *query = (activequery_t *)data;
#ifdef WIN32
  SHELLEXECUTEINFO sei = { sizeof(sei) };
#endif
  char read[32];
  int r;

#ifdef WIN32
  sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
  sei.nShow = SW_HIDE;
  sei.lpVerb = "open";
  sei.lpFile = CONFIG_QSTAT;
  sei.lpParameters = query->uid;
  sei.lpDirectory = es.wdir;
  sei.cbSize = sizeof(SHELLEXECUTEINFO);

  r = ShellExecuteEx(&sei);
  if (!r)
  {
    query->errorno = GetLastError();
  }
#endif
#ifdef UNIX
  r = system(va("%s%s \"%s\" %s", es.wdir, CONFIG_QSTAT, es.wdir, query->uid));
  if (r == -1)
  {
    query->errorno = errno;
  }
#endif
  else
  {
#ifdef WIN32
    WaitForSingleObject(sei.hProcess, MAX_THREADWAIT);
    CloseHandle(sei.hProcess);
#endif
    _strnprintf(read, sizeof(read), "%s%s.out", CONFIG_QSTATDIR, query->uid);
    LoadConfig(read, DELIM_QSTAT, &ResultParse, query);

#ifndef SUPERDEBUG
    unlink(va("%s%s%s.in", es.wdir, CONFIG_QSTATDIR, query->uid));
    unlink(va("%s%s%s.out", es.wdir, CONFIG_QSTATDIR, query->uid));
#endif
  }

  query->lastresult = NULL;
  __LockMutex(&query->mutex);
  query->done = 1;
  __UnlockMutex(&query->mutex);

  __endthread;
}

int ResultParse(char p[][MAX_PARAMSIZE+1], void *data)
{
  activequery_t *query = (activequery_t *)data;
  player_t *pl;
  result_t *res;

  if (p[0][0] == 'S') //server
  {
    res = NewResult(&query->results);
    if (!res)
      return 0;

    //SXtra Ranked #08 Favourite Maps FF\t210.55.92.74\t33333\t47\t48\tSharqi Peninsula
    //x              0                         1           2    3  4       5

    strlcpy(res->info.hostname, p[0] + 1, sizeof(res->info.hostname));
    strlcpy(res->info.ip, p[1], sizeof(res->info.ip));
    res->info.gameport = atoi(p[2]);
    res->info.queryport = atoi(p[3]);
    if (!res->info.queryport)
      res->info.queryport = res->info.gameport;
    res->info.numplayers = atoi(p[4]);
    res->info.maxplayers = atoi(p[5]);
    strlcpy(res->info.mod, p[6], sizeof(res->info.mod));
    strlcpy(res->info.map, p[7], sizeof(res->info.map));

    res->up = 1;
    query->numservers++;
    query->lastresult = res;

    return 1;
  }

  if (p[0][0] == 'D') //down server
  {
    res = NewResult(&query->results);
    if (!res)
      return 0;

    //D127.0.0.1	33333
    if (!strcmp(p[0] + 1, "0.0.0.0")) {
      strlcpy(res->info.ip, "unknown", sizeof(res->info.ip));
    } else {
      strlcpy(res->info.ip, p[0] + 1, sizeof(res->info.ip));
    }
    res->info.queryport = atoi(p[1]);

    res->up = 0;
    query->lastresult = res;

    return 1;
  }

  if (p[0][0] == 'P') //player
  {
    int score, frags, deaths;

    res = query->lastresult;
    if (!res)
    {
#ifdef _DEBUG
      echo(NULL, ECHO_SHOW, "Player without server\n");
#endif
      return 1;
    }

    //-[!] ElBandito	1	1	MEC	43555939
    //x     0         1 2  3      4

    pl = NewPlayer();
    if (!pl)
      return 0;

    strlcpy(pl->name, p[0] + 1, sizeof(pl->name));

    score = atoi(p[1]);
    frags = atoi(p[2]);
    deaths = atoi(p[3]);

    if (score == NAN_INT)
      score = 0;
    if (frags == NAN_INT)
      frags = 0;
    if (deaths == NAN_INT)
      deaths = 0;

    pl->score = (!score && frags) ? frags : score;
    pl->deaths = deaths;

    strlcpy(pl->team, p[4], sizeof(pl->team));
    pl->ping = atoi(p[5]);
    strlcpy(pl->pid, p[6], sizeof(pl->pid));

    res->info.players[res->playerix++] = pl;

    if (res->playerix > MAX_PLAYERS)
    {
#ifdef _DEBUG
      echo(NULL, ECHO_SHOW, "Miscounted total players\n");
#endif
      return 0;
    }

    return 1;
  }

#ifdef _DEBUG
  echo(NULL, ECHO_SHOW, "Mangled qstat file\n");
#endif

  return 0;
}

void ProcessQueries(void)
{
  activequery_t *query, *nq;
  result_t *res;
  int i;

  if (!queries)
    return;

#ifdef SUPERDEBUG
  superdebug("ProcessQueries()\n");
#endif

  query = queries;

  if (!network)
    network = query->network;

  while (query)
  {
    __LockMutex(&query->mutex);
    i = query->done;
    __UnlockMutex(&query->mutex);
    nq = query->next;

    if (i)
    {
      res = query->lastresult;

      if (!res)
        res = query->results;

      if (res)
      {
        for (i = 0; res && i < 2; res = res->next, i++)
          ShowServerInfo(res, query);
      }

      if (res)
      {
        query->lastresult = res;
      }
      else
      {
        if (query->errorno)
          Say(MSG_PRIVMSG, query->from, va("Query failed [%d]", query->errorno));


        if (network
#ifdef USE_GENERATE
          && !query->generate
#endif
           )
          ShowFooter(query);
#ifdef USE_GENERATE
#ifdef PARANOID
      else if (query->generate)
#else
      else
#endif
      {
        GeneratePage(query);
      }
#endif
        RemoveActiveQuery(&queries, query);
      }
    }

    query = nq;
  }
}

//.Scorched Earth 2 - Xtra TF. | 203.963.923.693:27500 | .try8. | 0/24.
//iPGN Deathmatch | 210.50.4.11:27500 | dimebox1 | 2/16 | 10:n3w m4ps, 4:3nzym3
void ShowServerInfo(result_t *res, activequery_t *query)
{
  char *result, *s;
  char ports[16];
  mapdesc_t desc;
  int b, i, j, top, sz;
  player_t *high;

#ifdef SUPERDEBUG
  superdebug("ShowServerInfo()\n");
#endif

  if (!network)
    return;

  //if (!data->hostname[0] || !data->map[0])
  //  return;
  if (!res->info.hostname[0])
    return;

  sz = MAX_GAME_HOSTNAME + MAX_GAME_STATUS + MAX_IP + MAX_GAME_MAP + 
       MAX_GAME_NUMPLAYERS + MAX_GAME_MAXPLAYERS + ((3 * 4) + 6) +
       (res->info.numplayers * (MAX_GAME_PLAYER_NAME + MAX_GAME_PLAYER_FRAGS + 6));

  result = (char *)_malloc_(sz);
  if (!result)
    return;

  //65535 (65535)
  if (res->info.gameport)
  {
    if (query->flags & QUERYFLAG_SHOWQUERYPORTS)
    {
      _strnprintf(ports, sizeof(ports), "%d (%d)", res->info.gameport, res->info.queryport);
    }
    else
    {
      _strnprintf(ports, sizeof(ports), "%d", res->info.gameport);
    }
  }
  else
  {
    _strnprintf(ports, sizeof(ports), "%d", res->info.queryport);
  }

  MapDescription(&desc, res->info.map);

  if (desc.match)
  {
    s = va(""IRC_BOLD"%s%s"IRC_BOLD" | %s:%s | "IRC_BOLD"%s"IRC_BOLD" - %s%s | %d/%d",
            res->info.hostname, res->info.status, res->info.ip, ports, res->info.map, desc.name, desc.size, res->info.numplayers, res->info.maxplayers);
  }
  else
  {
    s = va(""IRC_BOLD"%s%s"IRC_BOLD" | %s:%s | "IRC_BOLD"%s"IRC_BOLD" | %d/%d",
            res->info.hostname, res->info.status, res->info.ip, ports, res->info.map, res->info.numplayers, res->info.maxplayers);
  }
  strlcpy(result, s, sz);

  b = 0;
  i = 0;

#ifdef _DEBUG
  //showflags(query->flags);
#endif

  if (res->info.numplayers && (query->flags & QUERYFLAG_SHOWPLAYERS))
  {
    do
    {
      top = -99999;
      high = NULL;

      for (j = 0; j < MAX_PLAYERS; j++)
      {
        if (!res->info.players[j])
          continue;

        if (res->info.players[j]->displayed)
          continue;

        if (res->info.players[j]->score > top)
        {
          high = res->info.players[j];
          top = res->info.players[j]->score;
        }
      }

      if (high)
      {
        if (!high->name[0])
          goto _next;

        if ((query->flags & QUERYFLAG_WILDMATCH) && query->wildmatch)
        {
          //printf("%s vs %s\n", (*high)->name, query->wildmatch);
          if (match(high->name, query->wildmatch))
            goto _next;
        }

        strlcpy(high->displayname, high->name, sizeof(high->displayname));
        if (!(query->flags & QUERYFLAG_QUERYNICK))
          obfuscate(high->displayname);
        s = va(" "IRC_BOLD"%d"IRC_BOLD":%s", high->score, high->displayname);

        if (!b)
        {
          strlcat(result, " |", sz);
          b = 1;
        }
        strlcat(result, s, sz);

        i++;
_next:
        high->displayed = 1;
      }
    } while (high);
  }

  res->matched = i;
  query->matched += i;
  query->numplayers += res->info.numplayers;
  query->totalplayers += res->info.maxplayers;
  //query->numservers++; <-set in process

  if (query->lineout < MAX_LINEOUT)
  {
    if ( (!(query->flags & QUERYFLAG_SHOWONLYTIMEOUT)) &&
         (!(!(query->flags & QUERYFLAG_SHOWIFNOPLAYERS) && (res->info.numplayers == 0))) &&
         (!((query->flags & QUERYFLAG_WILDMATCH) && (res->matched == 0))) )
    {
      i = Say(MSG_PRIVMSG, query->from, result);
      query->lineout += i;
    }
  }

  _free_(result);
}

void ShowFooter(activequery_t *query)
{
  result_t *res;
  char *s;
  int sz;

#ifdef SUPERDEBUG
  superdebug("ShowFooter()\n");
#endif

  if (query->lineout >= MAX_LINEOUT)
  {
#ifdef USE_GENERATE
    if (GeneratePage(query))
    {
      if (query->wildmatch)
      {
        removeinclosed(query->wildmatch, '*');
        s = va("...and more: "IRC_BOLD"http://%s%s%s?find=%s"IRC_BOLD"", es.outurl, query->trigger->name, es.outext, query->wildmatch);
      }
      else
      {
        s = va("...and more: "IRC_BOLD"http://%s%s%s"IRC_BOLD"", es.outurl, query->trigger->name, es.outext);
      }
      Say(MSG_PRIVMSG, query->from, s);
    }
    else
#endif
    {
      Say(MSG_PRIVMSG, query->from, "...and more, other results not shown.");
    }
  }
  else
  {
    if ((query->flags & QUERYFLAG_SHOWONLYTIMEOUT) || (query->flags & QUERYFLAG_SHOWIFNOPLAYERS))
    {
      sz = 12 + ((24 * query->totalservers) + 1); //"%s:%d"
      s = (char *)_malloc_(sz);
      if (s)
      {
        s[0] = '\0';

        for (res = query->results; res; res = res->next)
        {
          if (!res->up)
          {
            if (s[0])
              strlcat(s, " ", sz);
            strlcat(s, va("%s:%d", res->info.ip, res->info.queryport), sz); 
          }
        }
        if (s[0])
          Say(MSG_PRIVMSG, query->from, va("Timeout: %s", s));
        _free_(s);
      }
    }

#ifdef USE_GENERATE
    //might as well, we've just queried it
    if (query->flags & QUERYFLAG_GENERATEPAGE)
    {
      GeneratePage(query);
    }
#endif
  }

  if (query->flags & QUERYFLAG_SHOWSTATS)
  {
    if (query->flags & QUERYFLAG_WILDMATCH)
    {
      s = va("Players: "IRC_BOLD"%d"IRC_BOLD"/"IRC_BOLD"%d"IRC_BOLD" Servers: "IRC_BOLD"%d"IRC_BOLD"/"IRC_BOLD"%d"IRC_BOLD" Matched: "IRC_BOLD"%d"IRC_BOLD"/"IRC_BOLD"%d"IRC_BOLD" ["IRC_BOLD"%c"GAMEQUERY_ME""IRC_BOLD" for help]",
             query->numplayers, query->totalplayers, query->numservers, query->totalservers, query->matched, query->numplayers, QUERYSYMBOL_TRIGGER);
    }
    else
    {
      s = va("Players: "IRC_BOLD"%d"IRC_BOLD"/"IRC_BOLD"%d"IRC_BOLD" Servers: "IRC_BOLD"%d"IRC_BOLD"/"IRC_BOLD"%d"IRC_BOLD" ["IRC_BOLD"%c"GAMEQUERY_ME""IRC_BOLD" for help]",
             query->numplayers, query->totalplayers, query->numservers, query->totalservers, QUERYSYMBOL_TRIGGER);
    }

    Say(MSG_PRIVMSG, query->from, s);
  }
}

void AdminLoadQuery(char *name)
{
#ifdef SUPERDEBUG
  superdebug("AdminLoadQuery()\n");
#endif

  if (!AdminValidate(name, (COMRADEFLAG_COMMAND/* | COMRADEFLAG_SERVERS*/)))
    return;

  RemoveTriggers();
  RemoveGameServers();
  LoadConfig(CONFIG_QUERY, DELIM_CONFIG, &QueryParse, NULL);
  Say(MSG_NOTICE, name, "Loaded");
}

void AdminSaveQuery(char *name)
{
  FILE *fp;
  trigger_t *trig;
  queryserver_t *qserv;
  querychannel_t *qchan;

#ifdef SUPERDEBUG
  superdebug("AdminSaveQuery()\n");
#endif

  if (!AdminValidate(name, (COMRADEFLAG_COMMAND | COMRADEFLAG_SERVERS)))
    return;

  fp = fopen(va("%s%s", es.wdir, CONFIG_QUERY), "wt");
  if (!fp)
  {
    Say(MSG_NOTICE, name, "Failed to open " CONFIG_QUERY " for writing");
    return;
  }

  for (trig = triggers; trig; trig = trig->next)
  {
    if (trig->custom)
      continue;

    fprintf(fp, "trigger:%s:", trig->name);
    if (trig->flags != QUERYFLAG_DEFAULT)
    {
      if (trig->flags & QUERYFLAG_SHOWPLAYERS)
        fprintf(fp, "%c", QUERYSYMBOL_SHOWPLAYERS);
      if (trig->flags & QUERYFLAG_HIDEPLAYERS)
        fprintf(fp, "%c", QUERYSYMBOL_HIDEPLAYERS);
      if (trig->flags & QUERYFLAG_SHOWIFNOPLAYERS)
        fprintf(fp, "%c", QUERYSYMBOL_SHOWIFNOPLAYERS);
      if (trig->flags & QUERYFLAG_HIDEIFNOPLAYERS)
        fprintf(fp, "%c", QUERYSYMBOL_HIDEIFNOPLAYERS);
      if (trig->flags & QUERYFLAG_SHOWSTATS)
        fprintf(fp, "%c", QUERYSYMBOL_SHOWSTATS);
      if (trig->flags & QUERYFLAG_HIDESTATS)
        fprintf(fp, "%c", QUERYSYMBOL_HIDESTATS);
      if (trig->flags & QUERYFLAG_QUERYNICK)
        fprintf(fp, "%c", QUERYSYMBOL_QUERYNICK);
      if (trig->flags & QUERYFLAG_GENERATEPAGE)
        fprintf(fp, "%c", QUERYSYMBOL_GENERATEPAGE);
    }
    if (trig->id)
      fprintf(fp, ":%d", trig->id);
    fprintf(fp, "\n");

    //channel:s:#query
    for (qchan = trig->channels; qchan; qchan = qchan->next)
    {
      fprintf(fp, "channel:%s:%s\n", trig->name, qchan->channel->name);
    }

    //server:s:qws:203.96.92.69:27500
    for (qserv = trig->servers; qserv; qserv = qserv->next)
    {
      fprintf(fp, "server:%s:%s:%s:%d\n", trig->name, qserv->server->game,
        qserv->server->addr, qserv->server->port);
    }

    fprintf(fp, "\n");
  }

  fclose(fp);

  Say(MSG_NOTICE, name, "Saved");
}

void AdminAddChannel(char *name, char *trigger, char *channel)
{
  trigger_t *trig;
  querychannel_t *qchan;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("AdminAddChannel()\n");
#endif

  if (!trigger || !channel)
    return;

  if (!AdminValidate(name, (COMRADEFLAG_COMMAND | COMRADEFLAG_SERVERS)))
    return;

  trig = findtrigger(trigger);
  if (!trig)
  {
    Say(MSG_NOTICE, name, va("Trigger %s not found", trigger));
    return;
  }

  qchan = findquerychannel(trig, channel);
  if (qchan)
  {
    Say(MSG_NOTICE, name, va("Channel %s already exists", channel));
    return;
  }

  chan = findchannel(network, channel);
  if (!chan)
  {
    Say(MSG_NOTICE, name, va("Channel %s not found", channel));
    return;
  }

  qchan = NewQueryChannel(trig);
  if (!qchan)
    return;

  qchan->channel = chan;
  Say(MSG_NOTICE, name, "Added");
}

void AdminRemoveChannel(char *name, char *trigger, char *channel)
{
  trigger_t *trig;
  querychannel_t *qchan;

#ifdef SUPERDEBUG
  superdebug("AdminRemoveChannel()\n");
#endif

  if (!trigger || !channel)
    return;

  if (!AdminValidate(name, (COMRADEFLAG_COMMAND | COMRADEFLAG_SERVERS)))
    return;

  trig = findtrigger(trigger);
  if (!trig)
  {
    Say(MSG_NOTICE, name, va("Trigger %s not found", trigger));
    return;
  }

  qchan = findquerychannel(trig, channel);
  if (!qchan)
  {
    Say(MSG_NOTICE, name, va("Channel %s not found", channel));
    return;
  }

  RemoveQueryChannel(trig, qchan);

  Say(MSG_NOTICE, name, "Removed");
}

void AdminAddServer(char *name, char *trigger, char *list)
{
  protocol_t *prot;
  queryserver_t *qserv;
  gameserver_t *gserv;
  trigger_t *trig;
  //game_t *game;
  char *s;
  char *ip;
  int i;
  int port;

#ifdef SUPERDEBUG
  superdebug("AdminAddServer()\n");
#endif

  if (!trigger || !list)
    return;

  trig = findtrigger(trigger);

  if (!AdminValidate(name, COMRADEFLAG_COMMAND) && !AdminValidateTrigger(name, trig))
    return;

  if (!trig)
  {
    Say(MSG_NOTICE, name, va("Trigger %s not found", trigger));
    return;
  }

  s = strchr(list, ' ');
  if (s)
    *s++ = '\0';

  prot = FindGame(list);
  if (!prot)
  {
    Say(MSG_NOTICE, name, va("Game %s not found", list));
    return;
  }

  list = s;
  i = 0;

  while ((list = listnext(list, &ip, &port)))
  {
    qserv = findqueryserver(trig, ip, port);
    if (qserv)
    {
      //AllocSay("NOTICE", name, va("Server %s:%d already exists", ip, port));
      continue;
    }

    gserv = findgameserver(ip, port);
    if (!gserv)
    {
      gserv = NewGameServer();
      if (!gserv)
        continue;

      strlcpy(gserv->addr, ip, sizeof(gserv->addr));
      gserv->port = port;
      strlcpy(gserv->game, prot->name, sizeof(gserv->game));

      gserv->custom = 0;
      port = 0;
    }

    qserv = NewQueryServer(trig);
    if (!qserv)
    {
      if (!port)
        RemoveGameServer(gserv);
      continue;
    }

    qserv->server = gserv;
    i++;
  }

  Say(MSG_NOTICE, name, va("Added %d server%s", i, (i == 1) ? "" : "s"));
}

//!delserver qwtf 203.96.91.1:27500 203.96.92.69 ...
void AdminRemoveServer(char *name, char *trigger, char *list)
{
  queryserver_t *qserv;
  gameserver_t *gserv;
  trigger_t *trig;
  char *ip;
  int i;
  int port;

#ifdef SUPERDEBUG
  superdebug("AdminRemoveServer()\n");
#endif

  if (!trigger || !list)
    return;

  trig = findtrigger(trigger);

  if (!AdminValidate(name, COMRADEFLAG_COMMAND) && !AdminValidateTrigger(name, trig))
    return;

  if (!trig)
  {
    Say(MSG_NOTICE, name, va("Trigger %s not found", trigger));
    return;
  }

  i = 0;

  while ((list = listnext(list, &ip, &port)))
  {
    if (!port)
      continue;

    qserv = findqueryserver(trig, ip, port);
    if (!qserv)
    {
      //AllocSay("NOTICE", name, va("Server %s:%d not found", ip, port));
      continue;
    }

    gserv = qserv->server;
    RemoveQueryServer(trig, qserv);
    RemoveUnusedGameServer(gserv);

    i++;
  }

  Say(MSG_NOTICE, name, va("Removed %d server%s", i, (i == 1) ? "" : "s"));
}

//nick, channel
trigger_t *allowedtrigger(char *name, char *from)
{
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("allowedtrigger()\n");
#endif
  
  trig = findtrigger(name);
  if (!trig)
    return NULL;

  if (findquerychannel(trig, from))
    return trig;

  if (finduser(network, from))
    return trig;

  return NULL;
}

int modifiers(char *s)
{
  int flags = 0;

#ifdef SUPERDEBUG
  superdebug("modifiers()\n");
#endif

  if (!s)
    return 0;

  while (*s)
  {
    switch (*s)
    {
      case QUERYSYMBOL_SHOWPLAYERS: flags |= QUERYFLAG_SHOWPLAYERS; break;
      case QUERYSYMBOL_HIDEPLAYERS: flags |= QUERYFLAG_HIDEPLAYERS; break;
      case QUERYSYMBOL_SHOWIFNOPLAYERS: flags |= QUERYFLAG_SHOWIFNOPLAYERS; break;
      case QUERYSYMBOL_HIDEIFNOPLAYERS: flags |= QUERYFLAG_HIDEIFNOPLAYERS; break;
      case QUERYSYMBOL_SHOWSTATS: flags |= QUERYFLAG_SHOWSTATS; break;
      case QUERYSYMBOL_HIDESTATS: flags |= QUERYFLAG_HIDESTATS; break;
      case QUERYSYMBOL_QUERYNICK: flags |= QUERYFLAG_QUERYNICK; break;
      case QUERYSYMBOL_WILDMATCH: flags |= QUERYFLAG_WILDMATCH; break;
//      case QUERYSYMBOL_HIDEIFNOWILDMATCH: flags |= QUERYFLAG_HIDEIFNOWILDMATCH; break;
//      case QUERYSYMBOL_MATCHCAST: flags |= QUERYFLAG_MATCHCAST; break;
      case QUERYSYMBOL_SHOWONLYTIMEOUT: flags |= QUERYFLAG_SHOWONLYTIMEOUT; break;
      case QUERYSYMBOL_SHOWQUERYPORTS: flags |= QUERYFLAG_SHOWQUERYPORTS; break;
      case QUERYSYMBOL_GENERATEPAGE: flags |= QUERYFLAG_GENERATEPAGE; break;

      default: break;
    }
    if (*s == QUERYSYMBOL_WILDMATCH)
      break;
    s++;
  }

  return flags;
}

int mergemodifiers(int a, int b)
{
#ifdef SUPERDEBUG
  superdebug("mergemodifiers()\n");
#endif

  if (b & QUERYFLAG_HIDEPLAYERS)
    a &= ~QUERYFLAG_SHOWPLAYERS;
  if (b & QUERYSYMBOL_HIDEIFNOPLAYERS)
    a &= ~QUERYFLAG_SHOWIFNOPLAYERS;
  if (b & QUERYFLAG_HIDESTATS)
    a &= ~QUERYFLAG_SHOWSTATS;

  if (b & QUERYFLAG_SHOWPLAYERS)
    a |= QUERYFLAG_SHOWPLAYERS;
  if (b & QUERYFLAG_SHOWIFNOPLAYERS)
    a |= QUERYFLAG_SHOWIFNOPLAYERS;
  if (b & QUERYFLAG_SHOWSTATS)
    a |= QUERYFLAG_SHOWSTATS;
  if (b & QUERYFLAG_WILDMATCH)
    a |= QUERYFLAG_WILDMATCH;
//  if (b & QUERYFLAG_HIDEIFNOWILDMATCH)
//    a |= QUERYFLAG_HIDEIFNOWILDMATCH;
  if (b & QUERYFLAG_MATCHCAST)
    a |= QUERYFLAG_MATCHCAST;
  if (b & QUERYFLAG_QUERYNICK)
    a |= QUERYFLAG_QUERYNICK;
  if (b & QUERYFLAG_SHOWONLYTIMEOUT)
    a |= QUERYFLAG_SHOWONLYTIMEOUT;
  if (b & QUERYFLAG_SHOWQUERYPORTS)
    a |= QUERYFLAG_SHOWQUERYPORTS;

  //if (a & QUERYFLAG_GENERATEPAGE)
  //  a &= ~QUERYFLAG_GENERATEPAGE;

  return a;
}

void obfuscate(char *name)
{
  int i, len, mid;
  char *p;

  if (!es.leetify)
    return;

  i = 0;
  p = name;
  while (*p)
  {
    if (*p == 'i')
    {
      *p = '1'; i = 1;
    }
    else if (*p == 'O')
    {
      *p = '0'; i = 1;
    }

    p++;
  }

  if (!i)
  {
    len = strlen(name);
    mid = len / 2;
    if (mid < 1 || mid >= MAX_NICKNAME)
      mid = 1;
    if (len + 1 >= MAX_NICKNAME)
    {
      name[mid] = '.';
      return;
    }

    for (i = len; i >= mid; i--)
    {
      name[i + 1] = name[i];
    }
    name[mid] = '.';
    name[len + 1] = '\0';
  }

  return;
}

#endif
