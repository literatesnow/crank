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

extern network_t *networks;
extern network_t *network;
#ifdef USE_QUERY
extern trigger_t *triggers;
extern gameserver_t *gameservers;
#endif

void AddNetwork(char p[][MAX_PARAMSIZE+1])
{
  network_t *net;

#ifdef SUPERDEBUG
  superdebug("AddNetwork()\n");
#endif

  if (!*p[1])
    return;

  net = findnetwork(p[1]);
  if (net)
    return;

  net = (network_t *)_malloc_(sizeof(network_t));
  if (!net)
    return;

  net->state = SOCKETSTATE_FREE;
  net->sock = 0;
  net->nick[0] = '\0';
  net->mynick[0] = '\0';
  net->ident[0] = '\0';
  net->fullname[0] = '\0';
  net->hostname[0] = '\0';
  net->retake = 0;
  net->lines = 0;
  net->lastretake = 0.0;
  net->lastconnect = 0.0;
  net->lastkeepalive = 0.0;
  net->lastsend = 0.0;
  net->buffertransfer = 0;
  net->transfer = 0;
  net->servers = NULL;
  net->connect = NULL;
  net->channels = NULL;
  net->users = NULL;
  net->comrades = NULL;
  net->buffers = NULL;
#ifdef USE_LOGGING
  net->log = NULL;
#endif
  net->flood = NULL;
  net->block = NULL;
  net->split = NULL;
#ifdef USE_STATS
  net->stats.triggered = 0;
  net->stats.tcprx = 0;
  net->stats.tcptx = 0;
  net->stats.udprx = 0;
  net->stats.udptx = 0;
#endif
  net->auth.registered = 0;
  net->auth.authed = 0;
  net->auth.flags = 0;
  net->auth.authserv[0] = '\0';
  net->auth.authline[0] = '\0';
  net->auth.authreply[0] = '\0';

  memset(&net->connectaddr.sin_zero, '\0', sizeof(net->connectaddr.sin_zero));
  net->connectaddr.sin_family = AF_INET;
  strlcpy(net->group, p[1], sizeof(net->group));
  strlcpy(net->grp, p[2], sizeof(net->grp));

  net->next = networks;
  networks = net;
}

void RemoveNetworks(void)
{
  network_t *net;
  server_t *server;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("RemoveLists()\n");
#endif

  while (networks)
  {
    net = networks;

#ifdef USE_LOGGING
    if (net->log)
      fclose(net->log);
#endif

    while (net->servers)
    {
      server = net->servers;
      net->servers = net->servers->next;
      _free_(server);
    }

    while (net->channels)
    {
      chan = net->channels;

      if (chan->log)
        fclose(chan->log);
      if (chan->link)
        _free_(chan->link);

      RemoveNicks(chan);

      net->channels = net->channels->next;
      _free_(chan);
    }

    RemoveUsers(net);
    RemoveBuffers(net);
    RemoveFloodChecks(net);
    RemoveComrades(net);

    if (net->sock)
      _close(net->sock);

    networks = networks->next;
    _free_(net);
  }
}

void AddChannel(char p[][MAX_PARAMSIZE+1])
{
  network_t *net;
  channel_t *chan;
  channel_t *prev;

#ifdef SUPERDEBUG
  superdebug("AddChannel()\n");
#endif
  
  if (!*p[1])
    return;

  net = findnetwork(p[1]);
  if (!net)
    return;

  prev = net->channels;
  while (prev && prev->next)
    prev = prev->next;

  chan = (channel_t *)_malloc_(sizeof(channel_t));
  if (!chan)
    return;

  strlcpy(chan->name, p[0], sizeof(chan->name));
  strlcpy(chan->key, p[2], sizeof(chan->key));
  strlcpy(chan->defaultmodes, p[3], sizeof(chan->defaultmodes));
  strlcpy(chan->defaulttopic, p[4], sizeof(chan->defaulttopic));
  chan->topic[0] = '\0';
  chan->topicsetby[0] = '\0';
  chan->modes[0] = '\0';
  //chan->flags = 0;
  chan->lastjoin = 0.0;
  chan->nicks = NULL;
  chan->log = NULL;
  chan->link = NULL;
  chan->next = NULL;
  chan->prev = NULL;

  if (prev)
  {
    prev->next = chan;
    chan->prev = prev;
  }
  if (!net->channels)
    net->channels = chan;
}

void AddLink(char p[][MAX_PARAMSIZE+1])
{
  channel_t *chan1, *chan2;
  network_t *net1, *net2;

  net1 = findnetwork(p[1]);
  net2 = findnetwork(p[3]);

  if (!net1 || !net2)
    return;

  chan1 = findchannel(net1, p[2]);
  chan2 = findchannel(net2, p[4]);

  if (!chan1 || !chan2)
    return;

  chan1->link = _malloc_(sizeof(link_t));
  if (!chan1->link)
    return;
  chan1->link->network = NULL;
  chan1->link->channel = NULL;

  chan2->link = _malloc_(sizeof(link_t));
  if (!chan2->link)
    return;
  chan2->link->network = NULL;
  chan2->link->channel = NULL;

  chan1->link->network = net2;
  chan1->link->channel = chan2;
  chan2->link->network = net1;
  chan2->link->channel = chan1;
}

void AddComrade(char p[][MAX_PARAMSIZE+1])
{
  network_t *net;
  comrade_t *comrade;
  char *s;
  int i;

#ifdef SUPERDEBUG
  superdebug("AddComrade()\n");
#endif

  net = findnetwork(p[1]);
  if (!net)
    return;

  comrade = (comrade_t *)_malloc_(sizeof(comrade_t));
  if (!comrade)
    return;

  i = 0;
  s = p[3];
  while (*s)
  {
    switch (*s)
    {
      case COMRADESYMBOL_AUTOOP: i |= COMRADEFLAG_AUTOOP; break;
      case COMRADESYMBOL_SERVERS: i |= COMRADEFLAG_SERVERS; break;
      case COMRADESYMBOL_MATCHCAST: i |= COMRADEFLAG_MATCHCAST; break;
      case COMRADESYMBOL_COMMAND: i |= COMRADEFLAG_COMMAND; break;
      case COMRADESYMBOL_OVERRIDE: i |= COMRADEFLAG_OVERRIDE; break;
      default: break;
    }
    s++;
  }

  strlcpy(comrade->mask, p[2], sizeof(comrade->mask));
  comrade->access = atoi(p[4]);
  comrade->flags = i;

  comrade->next = net->comrades;
  net->comrades = comrade;
}

void RemoveComrades(network_t *net)
{
  comrade_t *comrade;

#ifdef SUPERDEBUG
  superdebug("RemoveComrades()\n");
#endif

  if (!net)
    return;

  while (net->comrades)
  {
    comrade = net->comrades;
    net->comrades = net->comrades->next;
    _free_(comrade);
  }
}

void AddNetServer(char p[][MAX_PARAMSIZE+1])
{
  server_t *server;
  server_t *prev;
  network_t *net;

#ifdef SUPERDEBUG
  superdebug("AddNetServer()\n");
#endif

  net = findnetwork(p[1]);
  if (!net)
    return;

#ifdef SUPERDEBUG
  superdebug("AddNetServer()\n");
#endif

  prev = net->servers;
  while (prev && prev->next)
    prev = prev->next;

  server = (server_t *)_malloc_(sizeof(server_t));
  if (!server)
    return;

#if 0
  memset(&server->addr.sin_zero, '\0', sizeof(server->addr.sin_zero));
  server->addr.sin_family = AF_INET;
  server->addr.sin_port = htons((short)((*port) ? atoi(port) : DEFAULT_IRC_PORT));
  server->addr.sin_addr.s_addr = inet_addr(ip);
  strlcpy(server->pass, pass, sizeof(server->pass));
  server->next = NULL;
#endif

  strlcpy(server->addr, p[2], sizeof(server->addr));
  server->port = (p[3]) ? atoi(p[3]) : DEFAULT_IRC_PORT;
  strlcpy(server->pass, p[4], sizeof(server->pass));
  server->pos = 0;
  server->next = NULL;

  if (prev)
    prev->next = server;
  if (!net->servers)
    net->servers = server;
}

void AddNick(char *channel, char *names, int multi)
{
  channel_t *chan;
  nick_t *nick;
  user_t *user;
  //char *str;
  char *s;
  int flags;
  int total;
  int i;
  //int len;

#ifdef SUPERDEBUG
  superdebug("AddNick()\n");
#endif

  //printf("AddNick(): %s - %s %d\n", channel, names, multi);
  //len = (multi) ? ((MAX_NICKNAME * MAX_QUERYUSERHOST) + 1) : (MAX_NICKNAME + 1);
  //str = (char *)_malloc_(len);
  //if (!str)
  //  return;

  chan = findchannel(network, channel);
  if (!chan)
    return;
    //goto _done;

  //str[0] = '\0';
  //i = 0;
  total = 0;
 
  while (*names) //@Azmodan @crank___ @zod^^ mercury Silh evlow @bliP Ret- @donaldo madman Reddy @ChanServ
  {
    flags = 0;
    i = 1;
    while (*names && i)
    {
      switch (*names)
      {
        case SYMBOL_CHANOP: flags |= NICKFLAG_CHANOP; names++; break;
        case SYMBOL_HALFOP: flags |= NICKFLAG_HALFOP; names++; break;
        case SYMBOL_VOICED: flags |= NICKFLAG_VOICED; names++; break;
        default: i = 0; break;
      }
    }

    s = names;
    while (*s && *s != ' ')
      s++;
    if (*s)
      *s++ = '\0';

    user = finduser(network, names);
    if (!user)
    {
      user = NewUser(names);
      if (!user)
        goto _next;
    }

    nick = findnick(chan, names); //nick on this channel?
    if (!nick)
    {
      nick = NewNick(chan, user);
      if (!nick)
        goto _next;
    }
    ////if (!multi)
    ////  flags |= NICKFLAG_JOINED;
    nick->flags = flags;

    if (!multi && (!(user->flags & USERFLAG_SENTUSERHOST)))
    {
      user->flags |= USERFLAG_SENTUSERHOST | USERFLAG_SHOWIRCOP;

      Forward(va("WHO %s\r\n", user->nick));
    //
    //  strlcat(str, names, len);
    //  if (*s && (i < MAX_QUERYUSERHOST))
    //    strlcat(str, " ", len);
    //
    //  i++;
    }

    total++;

    //if (i > (MAX_QUERYUSERHOST - 1) || !*s)
    //{
    //  if (str[0])
    //  {
    //    Forward(va("USERHOST %s\r\n", str));
    //    str[0] = '\0';
    //  }
    //  i = 0;
    //}

_next:
    names = s;
  }

  if (multi)
  {
    Forward(va("WHO %s\r\n", chan->name));
    if (total == 1) //alone, alone, all alone....
      ChannelMode(chan, va("%s", DEFAULT_CHANNEL_MODE));
  }

//_done:
  //_free_(str);
}

nick_t *NewNick(channel_t *chan, user_t *user)
{
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("NewNick()\n");
#endif

  //printf("NewNick(): %s %s\n", chan->name, user->nick);

  nick = (nick_t *)_malloc_(sizeof(nick_t));
  if (!nick)
    return NULL;

  nick->user = user;
  nick->flags = 0;

  if (chan->nicks)
    chan->nicks->prev = nick;
  nick->next = chan->nicks;
  nick->prev = NULL; //this is set next time NewUser() is called
  chan->nicks = nick;

  return nick;
}

void RemoveNick(channel_t *chan, char *name)
{
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("RemoveNick()\n");
#endif

  //printf("RemoveNick(): %s %s\n", channel, name);

  if (!chan)
    return;

  nick = findnick(chan, name);
  if (!nick)
    return;

  if (nick->prev)
    nick->prev->next = nick->next;
  if (nick->next)
    nick->next->prev = nick->prev;
  if (!nick->prev)
    chan->nicks = nick->next;

  _free_(nick);

  //printf("RemoveNick(): removed %s from %s\n", name, channel);

#ifdef _DEBUG
  //NickOut(chan);
#endif
}

void RemoveNicks(channel_t *chan)
{
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("RemoveNicks()\n");
#endif

  if (!chan)
    return;

  while (chan->nicks)
  {
    nick = chan->nicks;
    chan->nicks = chan->nicks->next;
    _free_(nick);
  }
}

user_t *NewUser(char *nick)
{
  user_t *user;

#ifdef SUPERDEBUG
  superdebug("NewUser()\n");
#endif

  //printf("NewUser(): %s\n", nick);

  user = (user_t *)_malloc_(sizeof(user_t));
  if (!user)
    return NULL;

  strlcpy(user->nick, nick, sizeof(user->nick));
  user->ident[0] = '\0';
  user->address[0] = '\0';
  user->flags = 0;
  user->comrade = NULL;

  if (network->users)
    network->users->prev = user;
  user->next = network->users;
  user->prev = NULL; //this is set next time NewUser() is called
  network->users = user;

  return user;
}

void RemoveUser(char *name)
{
  user_t *user;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("RemoveUser()\n");
#endif

  //printf("RemoveUser(): %s\n", nick);

  user = finduser(network, name);
  if (!user)
    return;

  //remove user from all channels
  for (chan = network->channels; chan; chan = chan->next)
  {
    RemoveNick(chan, user->nick);
  }

  if (user->prev)
    user->prev->next = user->next;
  if (user->next)
    user->next->prev = user->prev;
  if (!user->prev)
    network->users = user->next;

  _free_(user);

  //printf("RemoveUser(): removed %s\n", nick);
}

void RemoveUsers(network_t *net)
{
  user_t *user;

#ifdef SUPERDEBUG
  superdebug("RemoveUsers()\n");
#endif

  if (!net)
    return;

  while (net->users)
  {
    user = net->users;
    net->users = net->users->next;
    _free_(user);
  }
}

void RemoveFrom(channel_t *where, char *nick)
{
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("RemoveFrom()\n");
#endif

  if (where)
  {
#ifdef _DEBUG
    //echo(NULL, ECHO_SHOW, "RemoveFrom(): _free_'ing nick %s from %s\n", nick, where->name);
#endif
    RemoveNick(where, nick);
  }

  for (chan = network->channels; chan; chan = chan->next) //see if this nick is on any other channels
  {
    if (findnick(chan, nick))
      return;
  }

#ifdef _DEBUG
  //echo(NULL, ECHO_SHOW, "RemoveFrom(): _free_'ing user %s\n", nick);
#endif

  //nah, remove
  RemoveUser(nick);
}

//away - 352 bliP #foo ~rad example.com example.com bliP G@ :0 splended!
//back - 352 bliP #foo ~rad example.com example.com bliP H@ :0 splended!
//away oper - 352 bliP #nztf ~rad example.com example.com bliP G*@iwsg :0 splended!
//back oper - 352 bliP #nztf ~rad example.com example.com bliP H*@iwsg :0 splended!
void AddWhoReply(char *channel, char *name, char *ident, char *address, char *info)
{
  user_t *user;
  channel_t *chan;
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("AddWhoReply()\n");
#endif

  //who reply gives us wrong channel sometimes
  //chan = findchannel(network, channel);
  //if (!chan)
  //  return;

  user = finduser(network, name);
  if (!user)
    return;

  strlcpy(user->ident, ident, sizeof(user->ident));
  strlcpy(user->address, address, sizeof(user->address));
  user->flags |= USERFLAG_SENTUSERHOST;

  if (*(++info) == '*')
  {
    user->flags |= USERFLAG_IRCOP;

    //if (!(user->flags & USERFLAG_SHOWIRCOP))
    //  return;

    for (chan = network->channels; chan; chan = chan->next)
    {
      for (nick = chan->nicks; nick; nick = nick->next)
      {
        if ((nick->user == user) && (nick->flags & NICKFLAG_SHOWOP))
        {
          echo(chan->name, ECHO_WRITE, va("* Note: %s is an IRC Operator\n", user->nick)); //display
          echo(NULL, ECHO_SHOW, va("%s * Note: %s is an IRC Operator\n", chan->name, user->nick)); //display
          linkmsg(chan, va("* Note: %s is an IRC Operator", user->nick)); //link

          nick->flags &= ~NICKFLAG_SHOWOP;
        }
      }
    }
  }
}

void AddBuffer(network_t *net, char *data, int sz)
{
  buffer_t *buffer;
  buffer_t *prev;
 // char *s;

#ifdef SUPERDEBUG
  superdebug("AddBuffer()\n");
#endif

  if (!net || !data || (net->state < SOCKETSTATE_SOCKCONNECTED))
    return;

  if (sz == -1)
    sz = strlen(data);

  if (!sz)
    return;

  prev = net->buffers;
  while (prev && prev->next)
    prev = prev->next;

#if 0
  //doesn't matter if we send a line in one packet or two still counted as one by server's floodprot
  //so no point in appending existing lines
  if (prev && (prev->lines <= 2) && ((prev->len + sz) < MAX_TRANSFER) && strncmp(data, "PRIVMSG", 7))
  {
    printf("AddBuffer(): append\n");

    /*s = (char *)_malloc_(prev->len + 1);
    strlcpy(s, prev->data, prev->len + 1);
    _free_(prev->data);

    prev->data = (char *)_malloc_(prev->len + sz + 1);
    strlcpy(prev->data, s, prev->len + sz + 1);
    strlcat(prev->data, data, prev->len + sz + 1);

    _free_(s);*/

    prev->len += sz;
    prev->data = _realloc_(prev->data, prev->len + 1);
    if (!prev->data)
      return;
    strlcat(prev->data, data, prev->len + 1);
    prev->lines++;

    //s = prev->data;
    //strlcpy(prev->data + prev->len, data, prev->len + sz + 1);
    //prev->data = s;
    return;
  }
#endif

  buffer = (buffer_t *)_malloc_(sizeof(buffer_t));
  if (!buffer)
    return;

  buffer->data = (char *)_malloc_(sz + 1);
  if (buffer->data)
  {
    strlcpy(buffer->data, data, sz + 1);
    buffer->len = sz;
  }
  buffer->lines = 0;
  buffer->next = NULL;

  if (prev)
    prev->next = buffer;
  if (!net->buffers)
    net->buffers = buffer;
}

void RemoveBuffers(network_t *net)
{
  buffer_t *buffer;

#ifdef SUPERDEBUG
  superdebug("RemoveBuffers()\n");
#endif

  if (!net)
    return;

  if (net->block)
  {
    _free_(net->block);
    net->block = NULL;
  }
  if (net->split)
  {
    _free_(net->split);
    net->split = NULL;
  }

  while (net->buffers)
  {
    buffer = net->buffers;
    net->buffers = net->buffers->next;
    _free_(buffer->data);
    _free_(buffer);
  }
}

void AddFloodCheck(network_t *net, char *from)
{
  floodcheck_t *flood;

#ifdef SUPERDEBUG
  superdebug("AddFloodCheck()\n");
#endif

  if (!net)
    return;

  flood = findfloodcheck(net, from);
  if (!flood)
  {
    flood = NewFloodCheck(net);
    if (!flood)
      return;

    flood->name = from; 
    flood->count = 1;
    flood->expires = DoubleTime() + FLOOD_MAX_TIME;
  }
  else
  {
    flood->count++;
  }
}

floodcheck_t *NewFloodCheck(network_t *net)
{
  floodcheck_t *flood;

#ifdef SUPERDEBUG
  superdebug("NewFloodCheck()\n");
#endif

  if (!net)
    return NULL;

  flood = (floodcheck_t *)_malloc_(sizeof(floodcheck_t));
  if (!flood)
    return NULL;

  flood->name = NULL;
  flood->warned = 0;
  flood->expires = 0.0;
  flood->count = 0;

  if (net->flood)
    net->flood->prev = flood;
  flood->next = net->flood;
  flood->prev = NULL;
  net->flood = flood;

  return flood;
}

void RemoveFloodCheck(network_t *net, floodcheck_t *flood)
{
#ifdef SUPERDEBUG
  superdebug("RemoveFloodCheck()\n");
#endif

  if (!net || !flood)
    return;

  if (flood->prev)
    flood->prev->next = flood->next;
  if (flood->next)
    flood->next->prev = flood->prev;
  if (!flood->prev)
    net->flood = flood->next;

  _free_(flood);
}

void RemoveFloodChecks(network_t *net)
{
  floodcheck_t *flood;

#ifdef SUPERDEBUG
  superdebug("RemoveFloodChecks()\n");
#endif

  if (!net)
    return;

  while (net->flood)
  {
    flood = net->flood;
    net->flood = net->flood->next;
    _free_(flood);
  }
}

//channel:s:#nztf
#ifdef USE_QUERY
void AddGameChannel(char p[][MAX_PARAMSIZE+1])
{
  querychannel_t *qchan;
  channel_t *chan;
  network_t *net;
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("AddGameChannel()\n");
#endif

  if (!*p[1])
    return;

  trig = findtrigger(p[1]);
  if (!trig)
  {
    echo(NULL, ECHO_SHOW, va("Trigger %s not found\n", p[1]));
    return;
  }

  for (chan = NULL, net = networks; net; net = net->next)
  {
    chan = findchannel(net, p[2]);
    if (chan)
      break;
  }

  if (!chan)
  {
    echo(NULL, ECHO_SHOW, va("No such channel %s\n", p[2]));
    return;
  }

  qchan = findquerychannel(trig, p[1]);
  if (qchan)
    return;

  qchan = NewQueryChannel(trig);
  if (!qchan)
    return;

  qchan->channel = chan;
}
#endif

//server:qwtf:qws:210.50.4.21:27500
#ifdef USE_QUERY
void AddGameServer(char p[][MAX_PARAMSIZE+1])
{
  protocol_t *prot;
  gameserver_t *gserv;
  queryserver_t *qserv;
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("AddGameServer()\n");
#endif

  if (!*p[1])
    return;

  trig = findtrigger(p[1]);
  if (!trig)
  {
    echo(NULL, ECHO_SHOW, va("Trigger %s not found\n", p[1]));
    return;
  }

  qserv = findqueryserver(trig, p[3], atoi(p[4]));
  if (qserv)
  {
    echo(NULL, ECHO_SHOW, va("Dup server %s:%s for trigger %s\n", p[3], p[4], p[1]));
    return;
  }

  prot = FindGame(p[2]);
  if (!prot) {
    echo(NULL, ECHO_SHOW, va("Game %s not found\n", p[2]));
    return;
  }

  qserv = NewQueryServer(trig);
  if (!qserv)
    return;

  gserv = findgameserver(p[3], atoi(p[4]));
  if (!gserv)
  {
    gserv = NewGameServer();
    if (!gserv)
    {
      RemoveQueryServer(trig, qserv);
      return;
    }

    strlcpy(gserv->addr, p[3], sizeof(gserv->addr));
    gserv->port = atoi(p[4]);
    strlcpy(gserv->game, prot->name, sizeof(gserv->game));
    gserv->custom = 0;
  }

  qserv->server = gserv;
}
#endif

#ifdef USE_QUERY
gameserver_t *NewGameServer(void)
{
  gameserver_t *gserv;

#ifdef SUPERDEBUG
  superdebug("NewGameServer()\n");
#endif

  gserv = (gameserver_t *)_malloc_(sizeof(gameserver_t));
  if (!gserv)
    return NULL;

  gserv->addr[0] = '\0';
  gserv->port = 0;
  gserv->game[0] = '\0';

  if (gameservers)
    gameservers->prev = gserv;
  gserv->next = gameservers;
  gserv->prev = NULL;
  gameservers = gserv;

  return gserv;
}
#endif

#ifdef USE_QUERY
void RemoveGameServer(gameserver_t *gserv)
{
#ifdef SUPERDEBUG
  superdebug("RemoveGameServer()\n");
#endif

  if (!gserv)
    return;

  if (gserv->prev)
    gserv->prev->next = gserv->next;
  if (gserv->next)
    gserv->next->prev = gserv->prev;
  if (!gserv->prev)
    gameservers = gserv->next;

  _free_(gserv);
}
#endif

#ifdef USE_QUERY
void RemoveGameServers(void)
{
  gameserver_t *gserv;

#ifdef SUPERDEBUG
  superdebug("RemoveGameServers()\n");
#endif

  while (gameservers)
  {
    gserv = gameservers;
    gameservers = gameservers->next;
    _free_(gserv);
  }
}
#endif

#ifdef USE_QUERY
void RemoveUnusedGameServer(gameserver_t *gserv)
{
  trigger_t *trig;
  queryserver_t *qserv;

#ifdef SUPERDEBUG
  superdebug("RemoveUnusedGameServer()\n");
#endif

  for (trig = triggers; trig; trig = trig->next)
  {
    for (qserv = trig->servers; qserv; qserv = qserv->next)
    {
      if (qserv->server == gserv)
        return;
    }
  }

  RemoveGameServer(gserv);
}
#endif

#ifdef USE_QUERY
activequery_t *NewActiveQuery(activequery_t **parent)
{
  activequery_t *query;

#ifdef SUPERDEBUG
  superdebug("NewActiveQuery()\n");
#endif

  if (!parent)
    return NULL;

  query = (activequery_t *)_malloc_(sizeof(activequery_t));
  if (!query)
    return NULL;

  query->trigger = NULL;
  query->from = NULL;
  query->results = NULL;
  query->wildmatch = NULL;
  //query->timeout = 0.0;
  //query->retrytime = 0.0;
  query->done = 0;
  query->flags = 0;
  //query->replys = 0;
  query->numservers = 0;
  query->totalservers = 0;
  query->numplayers = 0;
  query->totalplayers = 0;
  query->matched = 0;
  query->lineout = 0;
  query->id = 0;
  query->errorno = 0;
#ifdef USE_GENERATE
  query->generate = 0;
#endif
  __InitMutex(&query->mutex);
  query->network = NULL;
  query->lastresult = NULL;

  if (*parent)
  {
    (*parent)->prev = query;
    query->id = (*parent)->id + 1;
  }
  query->next = (*parent);
  query->prev = NULL;
  (*parent) = query;

  return query;
}
#endif

#ifdef USE_QUERY
void RemoveActiveQuery(activequery_t **parent, activequery_t *query)
{
#ifdef SUPERDEBUG
  superdebug("RemoveActiveQuery()\n");
#endif

  if (!query)
    return;

  if (query->trigger->custom)
    RemoveTrigger(query->trigger);

#ifdef WIN32
  CloseHandle(query->thread);
#endif
  __DestroyMutex(&query->mutex);

  if (query->prev)
    query->prev->next = query->next;
  if (query->next)
    query->next->prev = query->prev;
  if (!query->prev)
    *parent = query->next;

  if (query->from)
    _free_(query->from);
  if (query->wildmatch)
    _free_(query->wildmatch);

  RemoveResults(&query->results);

  _free_(query);
}
#endif

#ifdef USE_QUERY
void RemoveActiveQueries(activequery_t **parent)
{
  activequery_t *query;

#ifdef SUPERDEBUG
  superdebug("RemoveActiveQueries()\n");
#endif

  if (!parent)
    return;

  while (*parent)
  {
    query = *parent;
    *parent = (*parent)->next;

#ifdef WIN32
    WaitForSingleObject(query->thread, INFINITE);
#endif
#ifdef UNIX
    pthread_join(query->thread, NULL);
#endif

    RemoveActiveQuery(parent, query);
  }
}
#endif

#ifdef USE_QUERY
queryserver_t *NewQueryServer(trigger_t *trig)
{
  queryserver_t *qserv;

#ifdef SUPERDEBUG
  superdebug("NewQueryServer()\n");
#endif

  qserv = (queryserver_t *)_malloc_(sizeof(queryserver_t));
  if (!qserv)
    return NULL;

  qserv->server = NULL;

  if (trig->servers)
    trig->servers->prev = qserv;
  qserv->next = trig->servers;
  qserv->prev = NULL;
  trig->servers = qserv;

  return qserv;
}
#endif

#ifdef USE_QUERY
void RemoveQueryServer(trigger_t *trig, queryserver_t *qserv)
{
#ifdef SUPERDEBUG
  superdebug("RemoveQueryServer()\n");
#endif

  if (!trig)
    return;

  if (qserv->prev)
    qserv->prev->next = qserv->next;
  if (qserv->next)
    qserv->next->prev = qserv->prev;
  if (!qserv->prev)
    trig->servers = qserv->next;

  _free_(qserv);
}
#endif

#ifdef USE_QUERY
void RemoveQueryServers(trigger_t *trig)
{
  queryserver_t *qserv;

#ifdef SUPERDEBUG
  superdebug("RemoveQueryChannel()\n");
#endif

  if (!trig)
    return;

  while (trig->servers)
  {
    qserv = trig->servers;
    trig->servers = trig->servers->next;
    _free_(qserv);
  }
}
#endif

/*
trigger:s:~
channel:s:#foo
channel:s:#bar
channel:s:#blah
server:s:qws:203.96.92.69:27500
*/
#ifdef USE_QUERY
void AddTrigger(char p[][MAX_PARAMSIZE+1])
{
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("AddTrigger()\n");
#endif

  if (!*p[1])
    return;

  if (findtrigger(p[1]))
  {
    echo(NULL, ECHO_SHOW, va("Trigger %s already exists\n", p[1]));
    return;
  }

  trig = NewTrigger();
  if (!trig)
    return;

  strlcpy(trig->name, p[1], sizeof(trig->name));
  trig->flags = modifiers(p[2]);
  trig->id = atoi(p[3]);
  if (!trig->flags)
    trig->flags = QUERYFLAG_DEFAULT;
  trig->custom = 0;

  //printf("%s flags: ",trig->name);
  //showflags(trig->flags);
}
#endif

#ifdef USE_QUERY
querychannel_t *NewQueryChannel(trigger_t *trig)
{
  querychannel_t *qchan;

#ifdef SUPERDEBUG
  superdebug("NewQueryChannel()\n");
#endif

  qchan = (querychannel_t *)_malloc_(sizeof(querychannel_t));
  if (!qchan)
    return NULL;

  qchan->channel = NULL;

  if (trig->channels)
    trig->channels->prev = qchan;
  qchan->next = trig->channels;
  qchan->prev = NULL;
  trig->channels = qchan;

  return qchan;
}
#endif

#ifdef USE_QUERY
void RemoveQueryChannel(trigger_t *trig, querychannel_t *qchan)
{
#ifdef SUPERDEBUG
  superdebug("RemoveQueryChannel()\n");
#endif

  if (!trig)
    return;

  if (qchan->prev)
    qchan->prev->next = qchan->next;
  if (qchan->next)
    qchan->next->prev = qchan->prev;
  if (!qchan->prev)
    trig->channels = qchan->next;

  _free_(qchan);
}
#endif

#ifdef USE_QUERY
void RemoveQueryChannels(trigger_t *trig)
{
  querychannel_t *qchan;

#ifdef SUPERDEBUG
  superdebug("RemoveQueryChannel()\n");
#endif

  if (!trig)
    return;

  while (trig->channels)
  {
    qchan = trig->channels;
    trig->channels = trig->channels->next;
    _free_(qchan);
  }
}
#endif

#ifdef USE_QUERY
trigger_t *NewTrigger(void)
{
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("NewTrigger()\n");
#endif

  trig = (trigger_t *)_malloc_(sizeof(trigger_t));
  if (!trig)
    return NULL;

  trig->name[0] = '\0';
  trig->id = 0;
  trig->flags = 0;
  trig->custom = 0;
  trig->servers = NULL;
  trig->channels = NULL;

  if (triggers)
    triggers->prev = trig;
  trig->next = triggers;
  trig->prev = NULL;
  triggers = trig;

  return trig;
}
#endif

#ifdef USE_QUERY
void RemoveTrigger(trigger_t *trig)
{
  queryserver_t *qserv;

#ifdef SUPERDEBUG
  superdebug("RemoveTrigger()\n");
#endif

  if (!trig)
    return;

  for (qserv = trig->servers; qserv; qserv = qserv->next)
  {
    if (qserv->server->custom)
      RemoveGameServer(qserv->server);
  }
  RemoveQueryChannels(trig);
  RemoveQueryServers(trig);

  if (trig->prev)
    trig->prev->next = trig->next;
  if (trig->next)
    trig->next->prev = trig->prev;
  if (!trig->prev)
    triggers = trig->next;

  _free_(trig);
}
#endif

#ifdef USE_QUERY
void RemoveTriggers(void)
{
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("RemoveTriggers()\n");
#endif

  while (triggers)
  {
    trig = triggers;

    RemoveQueryChannels(trig);
    RemoveQueryServers(trig);

    triggers = triggers->next;
    _free_(trig);
  }
}
#endif

#ifdef USE_QUERY
result_t *NewResult(result_t **parent)
{
  result_t *res;

#ifdef SUPERDEBUG
  superdebug("NewResult()\n");
#endif

  if (!parent)
    return NULL;

  res = (result_t *)_malloc_(sizeof(result_t));
  if (!res)
    return NULL;

  InitServerInfo(&res->info);
  res->processed = 0;
  res->matched = 0;
  res->playerix = 0;
  //res->result = NULL;
  res->packets = NULL;

  res->next = *parent;
  *parent = res;

  return res;
}
#endif

#ifdef USE_QUERY
void RemoveResult(result_t **parent)
{
  result_t *res;

#ifdef SUPERDEBUG
  superdebug("RemoveResult()\n");
#endif

  if (!parent)
    return;

  res = *parent;

  //if (res->result)
  //  _free_(res->result);

  RemovePlayers(&res->info);

  *parent = (*parent)->next;
  _free_(res);
}
#endif

#ifdef USE_QUERY
void RemoveResults(result_t **parent)
{
#ifdef SUPERDEBUG
  superdebug("RemoveResults()\n");
#endif

  if (!parent)
    return;

  while (*parent)
    RemoveResult(parent);
}
#endif

#ifdef USE_QUERY
void RemovePlayers(serverinfo_t *info)
{
  int i;

#ifdef SUPERDEBUG
  superdebug("RemovePlayers()\n");
#endif

  if (!info)
    return;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (info->players[i])
      _free_(info->players[i]);
  }
}
#endif

#ifdef USE_QUERY
protocol_t *NewProtocol(protocol_t **prots)
{
  protocol_t *prot;

  prot = (protocol_t *) _malloc_(sizeof(protocol_t));
  if (!prot) {
    return NULL;
  }

  prot->name[0] = '\0';
  prot->len = 0;

  prot->next = *prots;
  *prots = prot;

  return prot;
}
#endif

#ifdef USE_QUERY
void RemoveProtocols(protocol_t **prots)
{
  protocol_t *prot;

#ifdef SUPERDEBUG
  superdebug("RemoveProtocols()\n");
#endif

  if (!*prots) {
    return;
  }

  while (*prots) {
    prot = *prots;
    *prots = (*prots)->next;
    _free_(prot);
  }
}
#endif

#ifdef USE_QUERY
player_t *NewPlayer(void)
{
  player_t *player;

#ifdef SUPERDEBUG
  superdebug("NewPlayer()\n");
#endif

  player = (player_t *)_malloc_(sizeof(player_t));
  if (!player)
    return NULL;

  player->name[0] = '\0';
  player->displayname[0] = '\0';
  player->team[0] = '\0';
  player->pid[0] = '\0';
  player->score = 0;
  //player->frags = 0;
  player->deaths = 0;
  player->id = -1;
  player->ping = 0;
  player->displayed = 0;
  player->prev = NULL;
  player->next = NULL;
  return player;
}
#endif

#ifdef USE_QUERY
trigger_t *NewCustomTrigger(char *from, char *nick, char *p, char *list)
{
  protocol_t *prot;
  trigger_t *trig;
  gameserver_t *gserv;
  queryserver_t *qserv;
#ifdef USE_OLDQUERY
  double t;//, e;
  char *r;
  int j;
#endif
  char *s;
  int i;

#ifdef SUPERDEBUG
  superdebug("NewCustomTrigger()\n");
#endif

  if (!p || !list)
    return NULL;

  prot = FindGame(p);
  if (!prot)
  {
    Say(MSG_NOTICE, nick, va("Unknown game \"%s\", use %c"GAMEQUERY_ME" for help", p, QUERYSYMBOL_TRIGGER));
    return NULL;
  }

  trig = NewTrigger();
  if (!trig)
    return NULL;

  strlcpy(trig->name, "seige", sizeof(trig->name));
  trig->flags = QUERYFLAG_DEFAULT;
  trig->custom = 1;

#ifdef USE_OLDQUERY
  t = DoubleTime() + 3.0;
#endif

//!query qws>+~ 203.96.92.69 203.96.92.69:27510
  while ((list = listnext(list, &s, &i)))
  {
#ifdef USE_OLDQUERY
    if (DoubleTime() >= t)
    {
      Say(MSG_NOTICE, from, "Failed to process other servers");
      break;
    }

    if (!validip(s))
    {
      j = 0;
      r = resolve(s, &j);
      //t = DoubleTime();
      if (!r)
      {
        Say(MSG_NOTICE, from, va("Host not found: %s", s));
        continue;
      }
      s = r;
    }
#endif

    qserv = findqueryserver(trig, s, i);
    if (!qserv)
    {
      gserv = NewGameServer();
      if (!gserv)
      {
        RemoveTrigger(trig);
        return NULL;
      }

#ifdef USE_OLDQUERY
      gserv->addr.sin_addr.s_addr = inet_addr(s);
      gserv->addr.sin_port = htons((short)i);
      gserv->game = game;
#else
      strlcpy(gserv->addr, s, sizeof(gserv->addr));
      gserv->port = i;
      strlcpy(gserv->game, prot->name, sizeof(gserv->game));
#endif
      gserv->custom = 1;

      qserv = NewQueryServer(trig);
      if (!qserv)
      { 
        RemoveTrigger(trig);
        return NULL;
      }

      qserv->server = gserv;
    }
  }

  return trig;
}
#endif

network_t *findnetwork(char *name)
{
  network_t *net;

#ifdef SUPERDEBUG
  superdebug("findnetwork()\n");
#endif

  for (net = networks; net; net = net->next)
  {
    if (!strcmp(net->group, name))
      return net;
  }

  return NULL;
}

nick_t *findnick(channel_t *chan, char *name)
{
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("findnick()\n");
#endif

  if (!chan)
    return NULL;

  for (nick = chan->nicks; nick; nick = nick->next)
  {
    if (!nick->user)
    {
#ifdef SUPERDEBUG
      echo(NULL, ECHO_NORMAL, "WARNING: nick->user was NOT init\'d!\n");
#endif
      continue;
    }
    if (!stricmp(nick->user->nick, name))
    {
      //debug(1, "findnick(): matched %s with %s\n", nick->user->nick, name);
      if (strcmp(nick->user->nick, name))
      {
#ifdef SUPERDEBUG
        echo(NULL, ECHO_WRITE, va("findnick(): WARNING: strcmp didnt think it was a match: %s with %s\n", nick->user->nick, name));
#endif
      }
      return nick;
    }
  }

  return NULL;
}

user_t *finduser(network_t *net, char *nick)
{
  user_t *user;

#ifdef SUPERDEBUG
  superdebug("finduser()\n");
#endif

  if (!net)
    return NULL;

  for (user = net->users; user; user = user->next)
  {
    if (!stricmp(user->nick, nick))
      return user;
  }

  return NULL;
}

comrade_t *findcomrade(network_t *net, char *address)
{
  comrade_t *comrade;

#ifdef SUPERDEBUG
  superdebug("findcomrade()\n");
#endif

  if (!net)
    return NULL;

  for (comrade = net->comrades; comrade; comrade = comrade->next)
  {
    if (!match(address, comrade->mask))
    {
      //printf("**** %s %s\n", address, comrade->mask);
      return comrade;
    }
  }

  return NULL;
}

channel_t *findchannel(network_t *net, char *name)
{
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("findchannel()\n");
#endif

  if (!net)
    return NULL;

  for (chan = net->channels; chan; chan = chan->next)
  {
    if (!stricmp(chan->name, name))
      return chan;
  }

  return NULL;
}

#ifdef USE_QUERY
trigger_t *findtrigger(char *name)
{
  trigger_t *trig;

#ifdef SUPERDEBUG
  superdebug("findtrigger()\n");
#endif

  for (trig = triggers; trig; trig = trig->next)
  {
    if (!stricmp(trig->name, name))
      return trig;
  }

  return NULL;
}
#endif

#ifdef USE_QUERY
gameserver_t *findgameserver(char *ip, int port)
{
  gameserver_t *gserv;

#ifdef SUPERDEBUG
  superdebug("findgameserver()\n");
#endif

  for (gserv = gameservers; gserv; gserv = gserv->next)
  {
    if (!strcmp(gserv->addr, ip) && gserv->port == port)
      return gserv;
  }

  return NULL;
}
#endif

#ifdef USE_QUERY
querychannel_t *findquerychannel(trigger_t *trig, char *name)
{
  querychannel_t *qchan;

#ifdef SUPERDEBUG
  superdebug("findquerychannel()\n");
#endif

  for (qchan = trig->channels; qchan; qchan = qchan->next)
  {
    if (!stricmp(qchan->channel->name, name))
      return qchan;
  }

  return NULL;
}
#endif

#ifdef USE_QUERY
queryserver_t *findqueryserver(trigger_t *trig, char *ip, int port)
{
  queryserver_t *qserv;

#ifdef SUPERDEBUG
  superdebug("findqueryserver()\n");
#endif

  for (qserv = trig->servers; qserv; qserv = qserv->next)
  {
    if (!strcmp(qserv->server->addr, ip) && qserv->server->port == port)
      return qserv;
  }

  return NULL;
}
#endif

floodcheck_t *findfloodcheck(network_t *net, char *from)
{
  floodcheck_t *flood;

#ifdef SUPERDEBUG
  superdebug("findfloodcheck()\n");
#endif

  if (!net)
    return NULL;

  for (flood = net->flood; flood; flood = flood->next)
  {
    if (!strcmp(flood->name, from))
      return flood;
  }

  return NULL;
}
