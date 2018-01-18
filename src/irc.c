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

extern essential_t es;
extern network_t *network;
extern network_t *networks;
#ifdef USE_QUERY
extern activequery_t *queries;
#endif

void Parse(char *rv, int sz)
{
  char *s, *e;
  char *p[MAX_DELIMITERS];
  int pr;

#ifdef SUPERDEBUG
  superdebug("Parse()\n");
#endif

  if (network->block)
  {
    _free_(network->block);
    network->block = NULL;
  }

  if (network->split)
  {
    pr = strlen(network->split) + sz + 1;
    network->block = (char *)_malloc_(pr);
    if (network->block)
    {
      strlcpy(network->block, network->split, pr);
      strlcat(network->block, rv, pr);
      rv = network->block;
    }
    _free_(network->split);
    network->split = NULL;
  }

  s = rv;
  e = rv;

  do
  {
    while (*e && *e != '\n')
    {
      if (*e == '\r')
        *e = '\0';
      e++;
    }

    if (!*e)
    {
      pr = strlen(s) + 2 + 1;
      network->split = (char *)_malloc_(pr);
      if (network->split)
      {
        strlcpy(network->split, s, pr);
      }
      return;
    }

    *e++ = '\0';
    //*e++ = '\0';

    memset(p, '\0', sizeof(p));
    pr = 0;

    if (*s == ':')
      s++;

    p[pr] = s;
    while (*s++ && pr < MAX_DELIMITERS)
    {
      if (*s == ' ')
      {
        *s++ = '\0';
        if (*s == ' ')
          s++;
        p[++pr] = s;
      }
      if (*s == ':')
      {
        p[pr] = ++s;
        break;
      }
    }

    if (!p[PRE] || !p[CMD])
      goto _skip;

    if (!strcmp(p[PRE], "PING")) //PING :example.com
      Forward(va("PONG :%s\r\n", p[CMD]));

    else if (!strcmp(p[PRE], "NOTICE")) //NOTICE AUTH :*** Looking up your hostname
      echo(NULL, ECHO_NORMAL, va("%s\n", p[PARAM0]));

    else if (!strcmp(p[PRE], "ERROR")) //ERROR :Closing Link: foo[~hi@example.com] by example.com (Ping Timeout)
      ServerDisconnect(p[pr]);

    else if (!strcmp(p[CMD], "NOTICE")) //example.com NOTICE foo :Highest connection count: 2 (2 clients)
      ServerNotice(p[PRE], p[PARAM0], p[pr]);

    else if (!strcmp(p[CMD], "JOIN")) //crank!~spin@example.com JOIN :#bar
      NickJoined(p[pr], p[PRE]);

    else if (!strcmp(p[CMD], "PART")) //Number4!~qwertyuio@example.com PART #foo
      NickParted(p[PARAM0], p[PRE], p[pr]);

    else if (!strcmp(p[CMD], "QUIT")) //Number0!~qwertyuio@example.com QUIT :Quit: HEHEHE HAHAHA HOOHOHO
      NickQuit(p[PRE], p[pr]);

    else if (!strcmp(p[CMD], "KILL")) //bliP!~rad@example.com KILL crank :bliP (heh¤)
      NickKilled(p[PRE], p[PARAM0], p[pr]);

    else if (!strcmp(p[CMD], "PRIVMSG")) //bliP!~rad@example.com PRIVMSG #foo :hello
      NickSaid(p[PRE], p[PARAM0], p[pr]);

    else if (!strcmp(p[CMD], "MODE")) //bliP!~rad@example.com MODE #foo +b *!*dsfs@fdfffff
      NickMode(p, pr);

    else if (!strcmp(p[CMD], "NICK")) //Traveler89!webchat@example.com NICK :Rayburn
      NickChange(p[PRE], p[PARAM0]);

    else if (!strcmp(p[CMD], "KICK")) //bliP!~rad@example.com KICK #foo Chad :boot
      NickKick(p[PRE], p[PARAM0], p[PARAM1], p[pr]);

    else if (!strcmp(p[CMD], "TOPIC")) //X3!X3@X3.AfterNET.Services TOPIC #AfterNET :Website accounts are now authserv accounts. http://afternet.org/accounts/ for details.
      NickTopic(p[PRE], p[PARAM0], p[pr]);

    else
    {
      switch (atoi(p[CMD]))
      {
        case 001: //example.com 001 foo :Welcome to the Internet Relay Network foo
          network->state = SOCKETSTATE_CONNECTED;
          strlcpy(network->hostname, p[PRE], sizeof(network->hostname));
          //forward("MODE %s +iswk", network->nick);
        case 002: //example.com 002 foo :Your host is example.com, running version beware1.5.5
        case 003: //example.com 003 foo :This server was created Wed Nov 26 2003 at 22:06:15 GMT
        case 250: //example.com 250 crank :Highest connection count: 188 (187 clients) (5785 connections received)
        case 251: //example.com 251 foo :There are 1 users and 1 invisible on 1 servers
        case 255: //example.com 255 foo :I have 2 clients and 0 servers
        case 265: //example.com 265 crank :Current local users: 100 Max: 918
        case 266: //example.com 266 crank :Current global users: 698 Max: 1302
        case 372: //example.com 372 foo :- 2004-9-4 17:11
        case 375: //example.com 375 foo :- example.com Message of the day
        case 396: //ign.ie.quakenet.org 396 crank example.users.quakenet.org :is now your hidden host
        case 501: //+x not found
        case 513: //example.com 513 crank___ :Your client may not be compatible with this server.
          if (*p[PARAM1])
            echo(NULL, ECHO_NORMAL, va("* %s\n", p[PARAM1]));
          break;

        case 376: //example.com 376 crank :End of /MOTD command.
        case 422: //example.com 422 crank :MOTD File is missing
          echo(NULL, ECHO_NORMAL, va("* %s\n", p[PARAM1]));
          JoinChannels();
          break;
        
        case 004: //example.com 004 foo example.com beware1.5.5 dgikoswx biklmnoprstv
          echo(NULL, ECHO_NORMAL, va("* %s\n", undil(p, PARAM1, pr)));
          break;
        
        case 005: //example.com 005 foo MAP SILENCE=15 WHOX WALLCHOPS WALLVOICES USERIP CPRIVMSG CNOTICE MODES=6 MAXCHANNELS=30 MAXBANS=45 :are supported by this server
          echo(NULL, ECHO_NORMAL, va("* %s %s\n", undil(p, PARAM1, pr - 1), p[pr]));
          break;

        case 252: //example.com 252 crank 1 :operator(s) online
        case 253: //example.com 253 foo 1 :unknown connection(s)
          echo(NULL, ECHO_NORMAL, va("* %s %s\n", p[PARAM1], p[pr]));
          break;
        
        case 254: //example.com 254 foo 1 :channels formed
          echo(NULL, ECHO_NORMAL, va("* %s %s\n", p[PARAM1], p[PARAM2]));
          break;

        case 302: //example.com 302 crank___ :crank=+rad@jKsfOb.lR6ma6.virtual
          if (network->retake && !*p[PARAM1] /*&& !strncmp(p[PARAM1], network->nick, strlen(network->nick))*/) //nick retake
          {
            strlcpy(network->mynick, network->nick, sizeof(network->mynick));
            Forward(va("NICK %s\r\n", network->mynick));
            network->retake = 0;
            break;
          }
          //AddUserHost(p[pr]);
          break;

        case 315: //example.com 315 crank #query :End of /WHO list.
          //EndOfWho(p[PARAM1]);
          break;

        case 324: //example.com 324 bliP #foo +nt
          if (p[PARAM2])
            undil(p, PARAM2, pr - 1);
          echo(p[PARAM1], ECHO_WRITE, va("* Modes: %s\n", p[PARAM2])); //display
          echo(NULL, ECHO_SHOW, va("%s * Modes: %s\n", p[PARAM1], p[PARAM2])); //display
          SetChannelDefaults(p[PARAM1], p[PARAM2]);
          break;

        case 329: //example.com 329 crank #pri 1106269838
          echo(p[PARAM1], ECHO_WRITE, va("* Created: %s", sctime(p[PARAM2]))); //display
          echo(NULL, ECHO_SHOW, va("%s * Created: %s", p[PARAM1], sctime(p[PARAM2])));  //display
          break;

        case 331: //example.com 331 bliP #bar No topic is set
          echo(p[PARAM1], ECHO_WRITE, va("* Topic: none\n")); //display
          echo(NULL, ECHO_SHOW, va("%s * Topic: none\n", p[PARAM1])); //display
          //ShowTopic(p[PARAM1], "none", NULL, NULL);
          //echo(p[PARAM1], "* [%s] topic- none\n", p[PARAM1]);
          break;

        case 332: //example.com 332 crank #bar :#bar !!!
          echo(p[PARAM1], ECHO_WRITE, va("* Topic: %s\n", p[pr])); //display
          echo(NULL, ECHO_SHOW, va("%s * Topic: %s\n", p[PARAM1], p[pr])); //display
          //ShowTopic(p[PARAM1], p[pr], NULL, NULL);
          SetTopic(findchannel(network, p[PARAM1]), p[pr], NULL);
          //echo(p[PARAM1], "* [%s] topic- %s\n", p[PARAM1], p[pr]);
          break;

        case 333: //example.com 333 crank #bar bliP 1106209031
          //ShowTopic(p[PARAM1], NULL, p[PARAM2], p[PARAM3]);
          echo(p[PARAM1], ECHO_WRITE, va("* Topic set by: %s on %s", p[PARAM2], sctime(p[PARAM3]))); //display
          echo(NULL, ECHO_SHOW, va("%s * Topic set by %s on %s", p[PARAM1], p[PARAM2], sctime(p[PARAM3]))); //display
          SetTopic(findchannel(network, p[PARAM1]), NULL, p[PARAM2]);
          break;

        case 352: //example.com 352 crank #nztf ~spin example.com example.com crank H :0 Insert 30c to cont..
          AddWhoReply(p[PARAM1], p[PARAM5], p[PARAM2], p[PARAM3], p[PARAM6]);
          break;

        case 353: //example.com 353 crank = #bar :crank @bliP
          AddNick(p[PARAM2], p[pr], 1);
          break;

        case 366: //example.com 366 crank #foo :End of /NAMES list.
          break;

        case 404: //example.com 404 crank2 #nztf :Cannot send to channel
          echo(p[PARAM1], ECHO_WRITE, va("* %s\n", p[pr])); //display
          echo(NULL, ECHO_SHOW, va("%s * %s\n", p[PARAM1], p[pr])); //display
          break;

        case 421: //example.com 421 crank 1x :Unknown command
          if (strcmp(p[PARAM1], KEEP_ALIVE))
            echo(NULL, ECHO_NORMAL, va("* %s: %s\n", p[pr], p[PARAM1]));
          break;

        case 433: //example.com 433 * crank :Nickname is already in use
          echo(NULL, ECHO_NORMAL, va("* %s (%s)\n", p[pr], p[PARAM1])); //display
          {
            char t[5];
            strlcpy(network->mynick, va("%s%s", network->nick, randch(t, sizeof(t))), sizeof(network->mynick));
            Forward(va("NICK %s\r\n", network->mynick));
            network->retake = 1;
          }
          break;

        case 461: //example.com 461  USER :Not enough parameters
          echo(NULL, ECHO_NORMAL, va("* %s: %s\n", p[PARAM0], p[pr])); //display
          if (!strcmp(p[PARAM0], "USER"))
            ServerDisconnect("Not enough details to connect");
          break;

        case 471: //example.com 471 crank #query :Cannot join channel (+l)
        case 473: //example.com 474 crank #query :Cannot join channel (+i)
        case 474: //example.com 474 crank #query :Cannot join channel (+b)
        case 475: //example.com 474 crank #query :Cannot join channel (+k)
          TimedJoin(findchannel(network, p[PARAM1]), p[pr], TIME_JOIN_ATTEMPT);
          break;

        case 482: //example.com 482 bliP #nztf You're not channel operator
          StripSelfOpFlag(p[PARAM1], p[pr]);
          break;
        
        default:
          echo(NULL, ECHO_NORMAL, va("[raw] %s\n", undil(p, PRE, pr + 1)));
          break;
      }
    }

_skip:
    s = e;
  } while (*e);
}

void Forward(char *data)
{
  int sz;
  double t;

#ifdef SUPERDEBUG
  superdebug("Forward()\n");
#endif

  if (!network)
    return;

  t = DoubleTime();

  network->lines = ((t - network->lastsend) <= MAX_BURST_SEND) ? network->lines + 1 : 0;

  sz = strlen(data);
  network->transfer += sz;

  if ((network->lines >= MAX_TRANSFER_LINES) || network->buffers || (network->transfer > MAX_TRANSFER))
  {
    //if (net->lines >= MAX_TRANSFER_LINES)
      //echo(NULL, ECHO_NORMAL, "*** WARNING: Forward(): Burst detected\n");
    //if (net->transfer > MAX_TRANSFER)
      //echo(NULL, ECHO_NORMAL, "*** WARNING: Forward(): MAX_TRANSFER exceeded\n");
    AddBuffer(network, data, sz);
    return;
  }

  if ((t - network->lastsend) > MAX_TRANSFER_RESET)
    network->transfer = 0;

  network->lastsend = t;
  network->buffertransfer = 0;

#ifdef _DEBUG
  printf("Forward() --> %s", data);
#endif
  if (SendNetSocket(network, data, sz) == SOCKET_ERROR)
  {
    ServerDisconnect("Connection error");
  }
}

int Say(int type, char *where, char *msg)
{
  channel_t *chan;
  nick_t *nick;
  user_t *user;
  char prefix[MAX_PREFIX+1];
  char *s, *p, *q, *buf, *mt;
  char c;
  int i, maxlen, lines;

#ifdef SUPERDEBUG
  superdebug("Say()\n");
#endif

  if (!network || !where || !msg)
    return 0;

  buf = _strdup_(msg);
  if (!buf)
    return 0;

  mt = (type) ? "PRIVMSG" : "NOTICE";
  maxlen = MAX_TRANSFER - strlen(mt) - strlen(where) - 5;

  msg = buf;
  s = msg;
  lines = 0;

  while (*s)
  {
    i = 0;
    p = NULL;
    q = NULL;
    c = 0;

    while (*s && i < maxlen)
    {
      if (*s == ' ')
        p = s;
      s++;
      i++;
    }

    if (*s)
    {
      if (!p)
      {
        c = *s;
        *s = '\0';
      }
      else
      {
        *p++ = '\0';
        s = p;
      }
      q = s;
    }

    Forward(va("%s %s :%s\r\n", mt, where, msg));
    lines++;

    if (c && q)
      *q = c;
    msg = q;
  }

  chan = findchannel(network, where);
  if (!chan)
  {
    user = finduser(network, where);
    if (!user)
    {
//#ifdef PARANOID
//      echo(NULL, ECHO_NORMAL, va("*** WARNING: Say() user %s not found in %s\n", where, net->group));
//#endif
      //message to channel i'm not on
      echo(NULL, ECHO_NORMAL, va("WIDE -> %s %s\n", where, buf)); //display
      goto _end;
    }

    echo(NULL, ECHO_NORMAL, va("PRIVATE -> *%s!%s@%s* %s\n", user->nick, user->ident, user->address, buf)); //display
    goto _end;
  }

  nick = findnick(chan, network->mynick);
  if (!nick)
  {
//#ifdef PARANOID
    //echo(NULL, ECHO_NORMAL, va("*** WARNING: Say() nick %s not in channel %s, %s list\n", network->mynick, chan->name, network->group));
//#endif
    //can happen with saybf2() messages
    goto _end;
  }

  nickprefix(nick, prefix);

  echo(chan->name, ECHO_WRITE, va("<%s%s> %s\n", prefix, network->mynick, buf)); //display
  echo(NULL, ECHO_SHOW, va("%s <%s%s> %s\n", chan->name, prefix, network->mynick, buf)); //display

_end:
  _free_(buf);

  return lines;
}

//USER spin "example.com" "192.0.34.166" :splended!
void Register(void)
{
#ifdef SUPERDEBUG
  superdebug("Register()\n");
#endif

  if (network->auth.registered)
    return;

  Forward("PASS *\r\n");
  Forward(va("NICK %s\r\n", network->nick));
  Forward(va("USER %s \"example.com\" \"192.0.34.166\" :%s\r\n", network->ident, network->fullname));
  network->auth.registered = 1;
}

void KeepAlive(void)
{
  double t;

  if (network->state != SOCKETSTATE_CONNECTED)
    return;

  if (network->buffers) //buffers will keep us alive
    return;

  if (network->retake) //nick retake will keep us alive
    return;

  t = DoubleTime();
	if ((t - network->lastkeepalive) >= TIME_KEEP_ALIVE)
  {
#ifdef SUPERDEBUG
    superdebug("KeepAlive()\n");
#endif
    network->lastkeepalive = t;
    Forward(KEEP_ALIVE "\r\n");
	}
}

void NickRetake(void)
{
  double t;

  if (network->state < SOCKETSTATE_CONNECTED)
    return;

  if (!network->retake)
    return;

  if (network->buffers) //if there's buffers then we'd just add to the backlog
    return;

#ifdef SUPERDEBUG
  superdebug("NickRetake()\n");
#endif

  t = DoubleTime();
	if ((t - network->lastretake) >= TIME_NICK_RETAKE)
  {
    network->lastretake = t;
    Forward(va("USERHOST %s\r\n", network->nick));
	}
}

void JoinAttempt(void)
{
  channel_t *chan;
  double t;

  if (network->state < SOCKETSTATE_CONNECTED)
    return;

  t = DoubleTime();

  for (chan = network->channels; chan; chan = chan->next)
  {
	  if ((chan->lastjoin > 0.0) && (t >= chan->lastjoin))
    {
#ifdef SUPERDEBUG
      superdebug("JoinAttempt()\n");
#endif
      chan->lastjoin = 0.0;
      if (chan->key)
        Forward(va("JOIN %s %s\r\n", chan->name, chan->key));
      else
        Forward(va("JOIN %s\r\n", chan->name));
    }
  }
}

void JoinChannels(void)
{
  char *s;
  channel_t *chan;
  int b, len;

#ifdef SUPERDEBUG
  superdebug("JoinChannels()\n");
#endif

  if (network->state < SOCKETSTATE_CONNECTED)
    return;

  if (!network->channels)
    return;

  if (!network->auth.authed)
  {
    SendAuth();

    if (network->auth.flags & AUTHFLAG_JOINWAIT)
      return;
  }

  Forward(va("MODE %s %s\r\n", network->mynick, DEFAULT_HIDDEN_MODE));
  
  len = 0;
  
  for (chan = network->channels; chan; chan = chan->next)
  {
    //<channel>{,<channel>} [<key>{,<key>}]
    len += strlen(chan->name) + 1;
    if (chan->key[0])
      len += strlen(chan->key) + 1;
    len++; // ,
  }
  len++;

  s = (char *)_malloc_(len);
  if (!s)
    return;

  //keyless channels
  s[0] = '\0';
  b = 0;
  for (chan = network->channels; chan; chan = chan->next)
  {
    if (chan->key[0])
      continue;
    if (b)
      strlcat(s, ",", len);
    strlcat(s, chan->name, len);
    b = 1;
  }
  Forward(va("JOIN %s\r\n", s));

  //channels with keys
  s[0] = '\0';
  b = 0;
  for (chan = network->channels; chan; chan = chan->next)
  {
    if (!chan->key[0])
      continue;
    if (s[0])
      strlcat(s, ",", len);
    strlcat(s, chan->name, len);
  }
  if (s[0])
    strlcat(s, " ", len);
  b = 0;
  for (chan = network->channels; chan; chan = chan->next)
  {
    if (!chan->key[0])
      continue;
    if (b)
      strlcat(s, ",", len);
    strlcat(s, chan->key, len);
    b = 1;
  }
  if (s[0])
  {
    Forward(va("JOIN %s\r\n", s));
  }

  _free_(s);
}

void SendAuth(void)
{
  if (!network->auth.authserv[0] || !network->auth.authline[0])
    return;
  Forward(va("PRIVMSG %s :%s\r\n", network->auth.authserv, network->auth.authline));
}

void AuthNetwork(char *nick, char *host, char *msg)
{
  if (network->auth.authed)
    return;

  if (strcmp(va("%s@%s", nick, host), network->auth.authserv))
    return;
  
  if (strcmp(msg, network->auth.authreply))
    return;

  network->auth.authed = 1;

  JoinChannels();
}

void TimedJoin(channel_t *chan, char *msg, int time)
{
#ifdef SUPERDEBUG
  superdebug("TimedJoin()\n");
#endif

  if (!chan)
    return;

  if (msg)
  {
    echo(chan->name, ECHO_WRITE, va("* %s\n", msg)); //display
    echo(NULL, ECHO_SHOW, va("%s * %s\n", chan->name, msg)); //display
  }

  chan->lastjoin = DoubleTime() + time;
}

void SetTopic(channel_t *chan, char *topic, char *setby)
{
#ifdef SUPERDEBUG
  superdebug("SetTopic()\n");
#endif

  if (!chan)
    return;

  if (topic)
    strlcpy(chan->topic, topic, sizeof(chan->topic));
  if (setby)
    strlcpy(chan->topicsetby, setby, sizeof(chan->topicsetby));
}

void SetChannelDefaults(char *channel, char *modes)
{
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("SetChannelDefaults()\n");
#endif

  chan = findchannel(network, channel);
  if (!chan)
    return;

  strlcpy(chan->modes, modes, sizeof(chan->modes));
  modifymode(chan, '~', 0, NULL);

  if (!isop(chan, network->mynick))
    return;

  if (chan->defaultmodes[0])
    ChannelMode(chan, chan->defaultmodes);

  if (chan->defaulttopic[0])
    Forward(va("TOPIC %s %s\r\n", chan->name, chan->defaulttopic));
}

void FlushBuffers(void)
{
  double t;
  buffer_t *buffer;
  int r;

  if (!network->buffers)
    return;

#ifdef SUPERDEBUG
  superdebug("FlushBuffers()\n");
#endif

  t = DoubleTime();

  if ((t - network->lastsend) <= MAX_BUFFER_SEND)
    return;

  if (network->buffertransfer >= MAX_TOTAL_TRANSFER && (t - network->lastsend) <= MAX_TRANSFER_DELAY)
  {
#ifdef _DEBUG
    //printf("*** %f >= %f ---- %f <= %f\n", network->transfer, MAX_TOTAL_TRANSFER, (t - network->lastbuffer), MAX_TRANSFER_DELAY);
    //printf("waiting: %f <= %f (%d >= %d)\n", (t - network->lastsend), MAX_TRANSFER_DELAY, network->buffertransfer, MAX_TOTAL_TRANSFER);
#endif
    return;
  }

  //echo(NULL, ECHO_NORMAL, "FlushBuffers(): Sending excess data\n");

  buffer = network->buffers;
  network->buffers = network->buffers->next;

  if (network->buffertransfer <= MAX_TOTAL_TRANSFER)
    network->buffertransfer += buffer->len;
  else
    network->buffertransfer = 0;

  network->lastsend = t;
  network->transfer = 0;

#ifdef _DEBUG
  printf("FlushBuffers() --> %s", buffer->data);
#endif
  r = SendNetSocket(network, buffer->data, buffer->len);

  _free_(buffer->data);
  _free_(buffer);

  if (r == SOCKET_ERROR)
  {
    ServerDisconnect("Connection error");  
  }
}

int FloodCheck(network_t *net, char *name, char *nick)
{
  floodcheck_t *flood;
  comrade_t *comrade;
  user_t *user;
  double t;

#ifdef SUPERDEBUG
  superdebug("FloodCheck()\n");
#endif

  if (!net || !net->flood)
    return 1;

  user = finduser(net, nick);
  if (!user)
    return 0;

  comrade = findcomrade(net, va("%s!%s@%s", user->nick, user->ident, user->address));
  if (comrade)
  {
    if (comrade->flags & COMRADEFLAG_COMMAND)
      return 1;
  }

  t = DoubleTime();

  for (flood = net->flood; flood; flood = flood->next)
  {
    if (!strcmp(flood->name, name))
    {
      if ((t <= flood->expires) && (flood->count >= FLOOD_MAX_TRIGGERS))
      {
        if (!flood->warned)
        {
          Say(MSG_NOTICE, nick, "You are temporarily ignored for flooding");
          flood->warned = 1;
        }
        return 0;
      }
    }
  }

  return 1;
}

void ExpiredFloodCheck(void)
{
  floodcheck_t *flood;
  floodcheck_t *n;
  double t;

  if (!network)
    return;

  if (!network->flood)
    return;

#ifdef SUPERDEBUG
  superdebug("ExpiredFloodCheck()\n");
#endif

  t = DoubleTime();

  flood = network->flood;
  while (flood)
  {
    n = flood->next;
    if (t >= flood->expires)
      RemoveFloodCheck(network, flood);
    flood = n;
  }
}

void CheckBanned(char *where, char *mask)
{
  channel_t *chan;
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("CheckBanned()\n");
#endif

  chan = findchannel(network, where);
  if (!chan)
    return;

  for (nick = chan->nicks; nick; nick = nick->next)
  {
    if (!match(va("%s!%s@%s", nick->user->nick, nick->user->ident, nick->user->address), mask))
    {
      nick->flags |= NICKFLAG_BANNED;
    }
  }
}

void ChannelMode(channel_t *chan, char *mode)
{
  char *s;

#ifdef SUPERDEBUG
  superdebug("ChannelMode()\n");
#endif

  if (!chan)
    return;

  if (!isop(chan, network->mynick))
    return;

  s = _strdup_(mode);
  if (!s)
    return;

  Forward(va("MODE %s %s\r\n", chan->name, s));

  _free_(s);
}

void NickJoined(char *channel, char *host)
{
  char *p[3];
  //char s[MAX_NICKNAME + MAX_IDENT + MAX_ADDRESS + 1];
  channel_t *chan;
  user_t *user;
  nick_t *nick;
  comrade_t *comrade;
  int i;

#ifdef SUPERDEBUG
  superdebug("NickJoined()\n");
#endif

  splithost(host, &p[0], &p[1], &p[2]);

  chan = findchannel(network, channel);
  if (!chan)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickJoined() channel %s not in %s list\n", network->group, channel));
#endif
#endif
    echo(NULL, ECHO_NORMAL, va("%s * Join: %s (%s@%s)\n", channel, p[0], p[1], p[2])); //display
    return;
  }

  //[23:59] Join: crank (spin@example.com)
  //[23:59] Join #AfterNet: crank (spin@example.com)

  echo(chan->name, ECHO_WRITE, va("* Join: %s (%s@%s)\n", p[0], p[1], p[2])); //display
  echo(NULL, ECHO_SHOW, va("%s * Join: %s (%s@%s)\n", chan->name, p[0], p[1], p[2])); //display

  AddNick(chan->name, p[0], 0);

  user = finduser(network, p[0]);
  if (user)
  { //if address differs, add last one we've seen
    //silent host change is lame at least quit them with a "regstered"
    if (strcmp(user->address, p[2]))
      strlcpy(user->address, p[2], sizeof(user->address));

    nick = findnick(chan, p[0]);
    if (user->flags & USERFLAG_IRCOP)
    {
      echo(chan->name, ECHO_WRITE, va("* Note: %s is an IRC Operator\n", user->nick)); //display
      echo(NULL, ECHO_SHOW, va("%s * Note: %s is an IRC Operator\n", chan->name, user->nick)); //display
      linkmsg(chan, va("* Note: %s is an IRC Operator", user->nick)); //link
    }
    else if (nick)
    {
      nick->flags |= NICKFLAG_SHOWOP;
    }
  }

  //friend
  comrade = findcomrade(network, va("%s!%s@%s", p[0], p[1], p[2]));
  if (comrade)
  {
    user->comrade = comrade;

    if (comrade->flags & COMRADEFLAG_AUTOOP)
    {
      ChannelMode(chan, va("%c%c %s", SYMBOL_POSITIVE, SYMBOL_MODECHANOP, p[0]));
    }
  }

  if (!strcmp(network->mynick, p[0])) //only get channel modes if it's us joining
  {
    Forward(va("MODE %s\r\n", chan->name));
    chan->lastjoin = 0.0;

    if (chan->link && (chan->link->network->state == SOCKETSTATE_CONNECTED))
    {
      //need 2 messages here as both networks won't connect at same time
      linkmsg(chan, va("* Link Open"));
      linkmsg(chan->link->channel, va("* Link Open"));
    }

    return;
  }
  else
  {
    linkmsg(chan, va("* Join: %s (%s@%s)", p[0], p[1], p[2])); //link
  }

  //clone scanner
  p[2] = p[2] - 4;
  memcpy(p[2], "*!*@", 4);

  for (i = 0, nick = chan->nicks; nick; nick = nick->next)
  {
    if (user && (nick->user == user)) //me
      continue;

    //_snprintf(s, sizeof(s), "%s!%s@%s", nick->user->nick, nick->user->ident, nick->user->address);
    if (!match(va("%s!%s@%s", nick->user->nick, nick->user->ident, nick->user->address), p[2]))
    {
      if (!i)
      {
        echo(chan->name, ECHO_WRITE, va("* Clones: %s, ", nick->user->nick)); //display
        echo(NULL, ECHO_SHOW, va("%s * Clones: %s, ", chan->name, nick->user->nick)); //display
      }
      else
      {
        echo(chan->name, ECHO_WRITE | ECHO_NOSTAMP, va("%s, ", nick->user->nick));
        echo(NULL, ECHO_SHOW | ECHO_NOSTAMP, va("%s, ", nick->user->nick));
      }
      i++;
    }
  }

  if (i)
  {
    i++;
    echo(chan->name, ECHO_WRITE | ECHO_NOSTAMP, va("%s (%d users)\n", user->nick, i));
    echo(NULL, ECHO_SHOW | ECHO_NOSTAMP, va("%s (%d users)\n", user->nick, i));
  }
}

void NickTopic(char *host, char *channel, char *newtopic)
{
  char *p[3];
  char prefix[MAX_PREFIX+1];
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("NickTopic()\n");
#endif

  splithost(host, &p[0], &p[1], &p[2]);

  chan = findchannel(network, channel);
  if (!chan)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickTopic() channel %s not in %s list\n", channel, network->group));
#endif
#endif
    echo(NULL, ECHO_NORMAL, va("* Topic: %s changes topic: %s\n", p[0], (*newtopic) ? newtopic : "(none)")); //display
    return;
  }

  nickprefix(findnick(chan, p[0]), prefix);
  echo(chan->name, ECHO_WRITE, va("* Topic: %s%s changes topic: %s\n", prefix, p[0], (*newtopic) ? newtopic : "(none)")); //display
  echo(NULL, ECHO_SHOW, va("%s * Topic: %s%s changes topic: %s\n", chan->name, prefix, p[0], (*newtopic) ? newtopic : "(none)")); //display
  linkmsg(chan, va("* Topic: %s%s changes topic: %s", prefix, p[0], (*newtopic) ? newtopic : "(none)")); //link
  SetTopic(chan, newtopic, NULL);
  SetTopic(chan, NULL, p[0]);
}

void NickKick(char *host, char *channel, char *knick, char *reason)
{
//bliP!~rad@example.com KICK #foo Chad :boot
  char *p[3];
  char prefix[MAX_PREFIX+1];
  channel_t *chan;
  time_t long_time;

#ifdef SUPERDEBUG
  superdebug("NickKick()\n");
#endif

  splithost(host, &p[0], &p[1], &p[2]);

  chan = findchannel(network, channel);
  if (!chan)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickKick() channel %s not in %s list\n", channel, network->group));
#endif
#endif
    echo(NULL, ECHO_NORMAL, va("* Kick: %s kicks %s (%s)\n", p[0], knick, (*reason) ? reason : knick)); //display
    RemoveFrom(NULL, knick);
    return;
  }

  nickprefix(findnick(chan, p[0]), prefix);
  echo(chan->name, ECHO_WRITE, va("* Kick: %s%s kicks %s (%s)\n", prefix, p[0], knick, (*reason) ? reason : knick)); //display
  echo(NULL, ECHO_SHOW, va("%s * Kick: %s%s kicks %s (%s)\n", chan->name, prefix, p[0], knick, (*reason) ? reason : knick)); //display
  linkmsg(chan, va("* Kick: %s%s kicks %s (%s)", prefix, p[0], knick, (*reason) ? reason : knick)); //link

  RemoveFrom(chan, knick);

  if (!strcmp(knick, network->mynick))
  {
    //Forward(va("JOIN %s\r\n", chan->name));
    time(&long_time);
#ifdef USE_LOGGING
    SessionClose(&chan->log, &long_time);
#endif
    TimedJoin(chan, NULL, TIME_JOIN_KICKED);
  }
}

void NickChange(char *host, char *newnick)
{
  nick_t *nick;
  user_t *user;
  channel_t *chan;
  char *p[3];

#ifdef SUPERDEBUG
  superdebug("NickChange()\n");
#endif
  //ListOut();

  splithost(host, &p[0], &p[1], &p[2]);

  user = finduser(network, p[0]);
  if (!user)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickChange() user %s not in %s list\n", p[0], network->group));
#endif
#endif
    return;
  }

  for (chan = network->channels; chan; chan = chan->next)
  {
    nick = findnick(chan, p[0]);
    if (!nick)
    {
#ifdef PARANOID
      //echo(NULL, ECHO_NORMAL, va("*** WARNING: NickChange() nick %s not in channel %s, %s\n", p[0], chan->name, network->group));
#endif
      continue;
    }

    echo(chan->name, ECHO_WRITE, va("* Nick: %s now known as %s\n", p[0], newnick)); //display
    echo(NULL, ECHO_SHOW, va("%s * Nick: %s now known as %s\n", chan->name, p[0], newnick)); //display
    linkmsg(chan, va("* Nick: %s now known as %s", p[0], newnick)); //link
  }

  strlcpy(user->nick, newnick, sizeof(user->nick));

  //ListOut();
}

void NickMode(char *p[], int pr)
{
  char *h[3];
  char prefix[MAX_PREFIX+1];
  char *s;
  char m;
  int i;
  nick_t *nick;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("NickMode()\n");
#endif

  splithost(p[PRE], &h[0], &h[1], &h[2]);
  chan = findchannel(network, p[PARAM0]);
  if (!chan)
  {
    if (!strcmp(p[PARAM0], network->mynick))
    {
      echo(NULL, ECHO_NORMAL, va("* Mode: %s sets: %s\n", h[0], undil(p, PARAM1, pr))); //display
      return;
    }
#ifdef PARANOID
    //this happens if main nick is taken and i don't know about it yet
    //echo(NULL, ECHO_NORMAL, va("*** WARNING: NickMode() channel %s not in %s list, not my nick (%s) either\n", p[PARAM0], network->group, network->mynick));
#endif
    return;
  }

  //bliP!~rad@example.com MODE #foo +bo *!*dsfs@fdfffff Ebert

  m = '\0';
  for (i = 0, s = p[PARAM1]; *s; s++)
  {
    switch (*s)
    {
      case '+':
      case '-':
        m = *s;
        break;

      case SYMBOL_MODEBAN:
        if (m == '+')
          CheckBanned(chan->name, p[PARAM2 + i]);
        i++;
        break;

      case SYMBOL_MODEKEY:
      case SYMBOL_MODELIMIT:
        modifymode(chan, m, *s, p[PARAM2 + i]);
        i++;
        break;

      case SYMBOL_MODECHANOP:
      case SYMBOL_MODEHALFOP:
      case SYMBOL_MODEVOICED:
        nick = findnick(chan, p[PARAM2 + i]);
        i++;
        if (!nick)
          continue;
        switch (m)
        {
          case '+':
            switch (*s)
            {
              case SYMBOL_MODECHANOP: nick->flags |= NICKFLAG_CHANOP; break;
              case SYMBOL_MODEHALFOP: nick->flags |= NICKFLAG_HALFOP; break;
              case SYMBOL_MODEVOICED: nick->flags |= NICKFLAG_VOICED; break;
              default: break;
            }
            break;

          case '-':
            switch (*s)
            {
              case SYMBOL_MODECHANOP: nick->flags &= ~NICKFLAG_CHANOP; break;
              case SYMBOL_MODEHALFOP: nick->flags &= ~NICKFLAG_HALFOP; break;
              case SYMBOL_MODEVOICED: nick->flags &= ~NICKFLAG_VOICED; break;
              default: break;
            }
            break;

          default:
            break;
        }
        break;

      default:
        modifymode(chan, m, *s, NULL);
        break;
    }
  }

  //channel message
  undil(p, PARAM1, pr);
  nick = findnick(chan, h[0]);
  if (!nick)
  {
//don't need this warning here: when servers mode nicks, Sync.US.AfterNET.Org +o someone after they auth
//#ifdef PARANOID
//    echo(NULL, ECHO_NORMAL, "*** WARNING: NickMode() nick %s not in %s, %s\n", h[0], chan->name, network->group);
//#endif
    echo(chan->name, ECHO_WRITE, va("* Mode: %s sets: %s\n", h[0], p[PARAM1])); //display
    echo(NULL, ECHO_SHOW, va("%s * Mode: %s sets: %s\n", chan->name, h[0], p[PARAM1])); //display
    linkmsg(chan, va("* Mode: %s sets: %s", h[0], p[PARAM1])); //link
    return;
  }

  nickprefix(nick, prefix);

  echo(chan->name, ECHO_WRITE, va("* Mode: %s%s sets: %s\n", prefix, nick->user->nick, p[PARAM1])); //display
  echo(NULL, ECHO_SHOW, va("%s * Mode: %s%s sets: %s\n", chan->name, prefix, nick->user->nick, p[PARAM1])); //display
  linkmsg(chan, va("* Mode: %s%s sets: %s", prefix, nick->user->nick, p[PARAM1])); //link

  for (i = 0, nick = chan->nicks; nick; nick = nick->next)
  {
    if (nick->flags & NICKFLAG_BANNED)
    {
      if (!i)
      {
        echo(chan->name, ECHO_WRITE, va("* Banned: %s", nick->user->nick)); //display
        echo(NULL, ECHO_SHOW, va("%s * Banned: %s", chan->name, nick->user->nick)); //display
      }
      else
      {
        echo(chan->name, ECHO_WRITE | ECHO_NOSTAMP, va(", %s", nick->user->nick));
        echo(NULL, ECHO_SHOW | ECHO_NOSTAMP, va(", %s", nick->user->nick));
      }

      nick->flags &= ~NICKFLAG_BANNED; 
      i++;
    }
  }

  if (i)
  {
    echo(chan->name, ECHO_WRITE | ECHO_NOSTAMP, va(" (%d user%s)\n", i, (i > 1) ? "s" : ""));
    echo(NULL, ECHO_SHOW | ECHO_NOSTAMP, va(" (%d user%s)\n", i, (i > 1) ? "s" : ""));
  }
}

void NickSaid(char *host, char *where, char *msg)
{
  char *p[3];
  char action[4];
  char prefix[MAX_PREFIX+1];
#ifdef PARANOID
  int i;
#endif
  nick_t *nick;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("NickSaid()\n");
#endif

  //bliP!~rad@example.com PRIVMSG #foo :hello
  //<BaM> woohoo - found some cool houses to buy today
  //[21:16] K|#pri <!@bliP> hello
  splithost(host, &p[0], &p[1], &p[2]);
  action[0] = '\0';

  if (*msg == SYMBOL_CTCP)
  {
    if (!strncmp(msg + 1, "ACTION", 6))
    {
      msg += 7;
#ifdef PARANOID
      i = strlen(msg)-1;
      if (msg[i] == SYMBOL_CTCP)
        msg[i] = '\0';
#else
      msg[strlen(msg)-1] = '\0';
#endif
      strlcpy(action, "/me", sizeof(action));
    }
  }

  chan = findchannel(network, where);
  if (!chan) //private message
  {
    echo(NULL, ECHO_NORMAL, va("PRIVATE <%s!%s@%s> %s%s\n", p[0], p[1], p[2], action, msg)); //display
//#ifdef USE_QUERY
    where = NULL;
    goto _trig;
//#else
//    return;
//#endif
  }

  //channel message
  nick = findnick(chan, p[0]);
  if (!nick)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickSaid() nick %s not in channel %s, %s list\n", p[0], chan->name, network->group));
#endif
#endif
    //echo(chan->name, ECHO_WRITE, va("<%s> %s%s\n", p[0], action, msg)); //display
    //echo(NULL, ECHO_SHOW, va("%s <%s> %s%s\n", chan->name, p[0], action, msg)); //display
    return;
  }

  nickprefix(nick, prefix);

  echo(chan->name, ECHO_WRITE, va("<%s%s> %s%s\n", prefix, nick->user->nick, action, msg)); //display
  echo(NULL, ECHO_SHOW, va("%s <%s%s> %s%s\n", chan->name, prefix, nick->user->nick, action, msg)); //display

  linkmsg(chan, va("<%s%s> %s%s", prefix, nick->user->nick, action, msg)); //link

//#ifdef USE_QUERY
_trig:
//#endif
  TriggerCheck(where, p[0], msg);
}

void NickQuit(char *host, char *msg)
{
  channel_t *chan;
  char *p[3];

#ifdef SUPERDEBUG
  superdebug("NickQuit()\n");
#endif

  splithost(host, &p[0], &p[1], &p[2]);

  for (chan = network->channels; chan; chan = chan->next)
  {
    if (findnick(chan, p[0]))
    {
      echo(chan->name, ECHO_WRITE, va("* Quit: %s (%s@%s) Quit (%s)\n", p[0], p[1], p[2], (*msg) ? msg : "Quit:")); //display
      echo(NULL, ECHO_SHOW, va("%s * Quit: %s (%s@%s) Quit (%s)\n", chan->name, p[0], p[1], p[2], (*msg) ? msg : "Quit:")); //display
      linkmsg(chan, va("* Quit: %s (%s@%s) Quit (%s)", p[0], p[1], p[2], (*msg) ? msg : "Quit:")); //link
    }
  }

  RemoveUser(p[0]);
}

void NickKilled(char *host, char *who, char *msg)
{
  char *p[3];

#ifdef SUPERDEBUG
  superdebug("NickKilled()\n");
#endif

  if (strcmp(network->mynick, who))
  {
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickKilled() not self: %s %s\n", network->mynick, who));
#endif
    return;
  }

  splithost(host, &p[0], &p[1], &p[2]);
  //*** crank (~spin@example.com) Quit (Killed (bliP (heh)))
  echo(NULL, ECHO_NORMAL, va("* You were killed by %s (%s@%s): %s\n", p[0], p[1], p[2], msg)); //display
}

void NickParted(char *channel, char *host, char *msg)
{
  channel_t *chan;
  char *p[3];

#ifdef SUPERDEBUG
  superdebug("NickParted()\n");
#endif

  splithost(host, &p[0], &p[1], &p[2]);
  //[21:10] K|#pri part- Jack (~qwertyuio@example.com)
  chan = findchannel(network, channel);
  if (!chan)
  {
#ifdef PARANOID
#ifdef SUPERDEBUG
    echo(NULL, ECHO_NORMAL, va("*** WARNING: NickParted() channel %s not in %s list\n", network->group, channel));
#endif
#endif
    echo(NULL, ECHO_NORMAL, va("* Part: %s (%s@%s): %s\n", p[0], p[1], p[2], msg)); //display
    RemoveFrom(NULL, p[0]);
    return;
  }

  echo(chan->name, ECHO_WRITE, va("* Part: %s (%s@%s): %s\n", p[0], p[1], p[2], (strcmp(channel, msg)) ? msg : "bye")); //display
  echo(NULL, ECHO_SHOW, va("%s * Part: %s (%s@%s): %s\n", chan->name, p[0], p[1], p[2], (strcmp(channel, msg)) ? msg : "bye")); //display
  linkmsg(chan, va("* Part: %s (%s@%s): %s", p[0], p[1], p[2], (strcmp(channel, msg)) ? msg : "bye")); //link

  RemoveFrom(chan, p[0]);
}

void StripSelfOpFlag(char *channel, char *msg)
{
  nick_t *nick;
  channel_t *chan;
  
#ifdef SUPERDEBUG
  superdebug("RemoveOpFlag()\n");
#endif

  chan = findchannel(network, channel);
  if (!chan)
    return;

  echo(chan->name, ECHO_WRITE, va("* %s\n", msg)); //display
  echo(NULL, ECHO_SHOW, va("%s * %s\n", chan->name, msg)); //display

  nick = findnick(chan, network->mynick);
  if (!nick)
    return;

  //we thought we had ops but we don't, remove flag
  nick->flags &= ~NICKFLAG_CHANOP;
}

void ServerNotice(char *host, char *who, char *msg)
{
  char *p[3];
  char prefix[MAX_PREFIX+1];
#if ((MAX_CHANNEL) > (MAX_NICKNAME))
  char to[MAX_CHANNEL+1];
#else
  char to[MAX_NICKNAME+1];
#endif
  channel_t *chan;
  int serv;

/*
[K@23:50] bliP!~rad@example.com crank (peonnotice/#foo/5) non ops
[K@23:50] bliP!~rad@example.com crank (ovnotice/#foo/4@/5+) ops n voice
[K@23:50] bliP!~rad@example.com @#foo @ (ohnotice/#foo/4@/3%) ops n halfops
[K@23:50] bliP!~rad@example.com crank (ohnotice/#foo/4@/3%) ops n halfops
[K@23:50] bliP!~rad@example.com @#foo @ (opnotice/#foo/5@) ops only

[K@10:46] bliP!~rad@example.com crank (private) direct to crank
[K@10:47] bliP!~rad@example.com #foo (#foo) direct to channel foo

[K@10:48] bliP!~rad@example.com @#foo @ onotice for channel foo
*/
#ifdef SUPERDEBUG
  superdebug("ServiceNotice()\n");
#endif

  strlcpy(to, (*who == '@') ? who + 1 : who, sizeof(to));
  splitnotice(to, sizeof(to), &msg);
  serv = !strcmp(host, network->hostname);

  chan = findchannel(network, to);
  if (!chan || serv) //from server / private
  {
    echo(NULL, ECHO_NORMAL, va("-%s- %s\n", host, msg));

    if (!serv)
    {
      splithost(host, &p[0], &p[1], &p[2]);
      AuthNetwork(p[0], p[2], msg);
    }

    return;
  }

  //channel msg
  splithost(host, &p[0], &p[1], &p[2]);
  nickprefix(findnick(chan, p[0]), prefix);
  echo(chan->name, ECHO_WRITE, va("-%s%s- %s\n", prefix, p[0], msg));
  echo(NULL, ECHO_SHOW, va("%s -%s%s- %s\n", chan->name, prefix, p[0], msg)); 
  linkmsg(chan, va("-%s%s- %s", prefix, p[0], msg)); //link
}

void ServerDisconnect(char *msg)
{
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("ServerDisconnect()\n");
#endif

  if (network->state <= SOCKETSTATE_DISCONNECTED)
    return;

  echo(NULL, ECHO_NORMAL, va("* %s\n", msg)); //display
  echo(NULL, ECHO_NORMAL, "Disconnected\n\n");

  for (chan = network->channels; chan; chan = chan->next)
  {
    echo(chan->name, ECHO_WRITE, va("* %s\n", msg)); //display
    echo(chan->name, ECHO_WRITE, "Disconnected\n\n");
    linkmsg(chan, va("* Link Broken: %s", msg)); //link
    RemoveNicks(chan);
  }

  RemoveUsers(network);
  RemoveBuffers(network);

  network->state = SOCKETSTATE_DISCONNECTED;
  network->lastkeepalive = 0.0;
  network->lastconnect = DoubleTime() + (double)es.delay;
  network->lastsend = 0.0;
  network->transfer = 0;
  network->auth.registered = 0;
  network->auth.authed = 0;

  _close(network->sock);
  network->sock = 0;

  //printf("ServerDisconnect(): %s was terminated (%d)\n", network->group, network->sock);
}

void linkmsg(channel_t *chan, char *str)
{
  char *p;
  nick_t *nick;

  if (!chan)
    return;

  if (!chan->link)
    return;

  if (chan->link->network->state != SOCKETSTATE_CONNECTED)
    return;

#ifdef PARANOID
  if (!chan->link->channel)
    return;
#endif

  for (nick = chan->link->channel->nicks; nick; nick = nick->next)
  {
    //printf("*********checking %s [%s]\n", nick->user->nick, nick->user->comrade?"yer":"nah");
#ifdef PARANOID
    if (!nick->user)
      continue;
#endif
    if (nick->user->comrade && (nick->user->comrade->flags & COMRADEFLAG_OVERRIDE))
    {
      return;
    }
  }

  //this strdup is here otherwise the 2x va() stomp on eachother
  p = _strdup_(str);
  if (!p)
    return;

  AddBuffer(chan->link->network, va("PRIVMSG %s :[%s] %s\r\n", chan->link->channel->name, network->grp, p), -1);

  _free_(p);
}

void modifymode(channel_t *chan, char action, char mode, char *param)
{
  char params[MAX_MODE+1];
  char modes[MAX_MODE+1];
  char *p[MAX_DELIMITERS];
  char *s, *pa;
  int i, j;

  if (!chan)
    return;

  memset(p, 0, sizeof(p));

  s = chan->modes;
  p[0] = s;
  i = 0;
  while (*s)
  {
    if (*s == ' ' && i < MAX_DELIMITERS)
    {
      *s++ = '\0';
      p[++i] = s;
    }
    else
    {
      s++;
    }
  }
  
  params[0] = '\0';
  i = 0;
  j = 0;
  for (s = p[0]; (*s && (i < MAX_DELIMITERS)); s++)
  {
    if (j >= sizeof(modes))
      break;

    switch (*s)
    {
      case SYMBOL_MODEKEY:
        i++;
        pa = "*";
        break;
      case SYMBOL_MODELIMIT:
        i++;
        pa = p[i];
        break;
      default:
        pa = NULL;
        break;
    }

    if (*s == mode)
      continue;
  
    modes[j++] = *s;
    if (pa)
    {
      strlcat(params, " ", sizeof(params));
      strlcat(params, pa, sizeof(params));
    }
  }

  if ((action == '+') && (j < sizeof(modes)))
  {
    modes[j++] = mode;
    if (param)
    {
      if (mode == SYMBOL_MODEKEY)
        strlcat(params, " *", sizeof(params));
      else
      {
        strlcat(params, " ", sizeof(params));
        strlcat(params, param, sizeof(params));
      }
    }
  }

  modes[j] = '\0';
  _strnprintf(chan->modes, sizeof(chan->modes), "%s%s", modes, params);
}

void nickprefix(nick_t *nick, char *prefix)
{
  *prefix = '\0';

#ifdef SUPERDEBUG
  superdebug("nickprefix()\n");
#endif

  if (!nick)
    return;

  if (nick->user->flags & USERFLAG_IRCOP)
    *prefix++ = SYMBOL_IRCOP;

  if (nick->flags & NICKFLAG_CHANOP)
    *prefix++ =  SYMBOL_CHANOP;
  else if (nick->flags & NICKFLAG_HALFOP)
    *prefix++ = SYMBOL_HALFOP;
  else if (nick->flags & NICKFLAG_VOICED)
    *prefix++ = SYMBOL_VOICED;

  *prefix = '\0';
}

int isop(channel_t *chan, char *name)
{
  nick_t *nick;

#ifdef SUPERDEBUG
  superdebug("isop()\n");
#endif

  if (!chan)
    return 0;

  nick = findnick(chan, name);
  if (!nick)
    return 0;

  if (nick->flags & NICKFLAG_CHANOP)
    return 1;

  return 0;
}

void TriggerCheck(char *channel, char *nick, char *msg)
{
  char *p[3];
  char *from;
  channel_t *chan = NULL;
  user_t *user;

#ifdef SUPERDEBUG
  superdebug("TriggerCheck()\n");
#endif

  if (*msg != QUERYSYMBOL_TRIGGER && *msg != SYMBOL_CTCP)
    return;

  splitmsg(++msg, &p[0], &p[1], &p[2]);

  if (!channel
#ifdef USE_QUERY
               || (modifiers(p[1]) & QUERYFLAG_QUERYNICK)
#endif
    )
  {
    user = finduser(network, nick);
    if (!user)
      return;
    from = user->nick;
  }
  else
  {
    chan = findchannel(network, channel);
    if (!chan)
      return;
    from = chan->name;
  }

  if (!FloodCheck(network, from, nick))
  {
    //printf("ignore flood: %s %s\n", from, nick);
    return;
  }
  
  if (!strcmp(p[0], "restart"))
    AdminRestart(nick);
  else if (!strcmp(p[0], "uptime"))
    AdminUptime(from);
  else if (!strncasecmp(p[0], "version", 7) && p[0][7] == SYMBOL_CTCP)
    AdminVersion(from);
#ifdef USE_QUERY    
  else if (!stricmp(p[0], GAMEQUERY_ME)) // !crank
    ShowHelp(nick);
  else if (!strcmp(p[0], "addserver"))
    AdminAddServer(nick, p[1], p[2]);
  else if (!strcmp(p[0], "delserver"))
    AdminRemoveServer(nick, p[1], p[2]);
  else if (!strcmp(p[0], "addchannel"))
    AdminAddChannel(nick, p[1], p[2]);
  else if (!strcmp(p[0], "delchannel"))
    AdminRemoveChannel(nick, p[1], p[2]);
  else if (!strcmp(p[0], "savequery"))
    AdminSaveQuery(nick);
  else if (!strcmp(p[0], "loadquery"))
    AdminLoadQuery(nick);
#endif
#ifdef USE_STATS
  else if (!strcmp(p[0], GAMEQUERY_ME":stats"))
    AdminNetStats(nick, from);
#endif
#ifdef USE_SERVICE
  else if (!strcmp(p[0], "borg"))
    AdminBorg(nick, from);
#endif
#ifdef _DEBUG
  //else if (!strcmp(p[0], "flood"))
  //  FloodMe();
#endif
#ifdef USE_QUERY
  else
  {
    trigger_t *trig = NULL;
    nick_t *nickname;

    if (chan)
    {
      for (nickname = chan->nicks; nickname; nickname = nickname->next)
      {
#ifdef PARANOID
        if (!nickname->user)
          continue;
#endif
        if (nickname->user->comrade && (nickname->user->comrade->flags & COMRADEFLAG_OVERRIDE))
          return;
      }
    }

    if (findchannel(network, p[0])) // !#nztf
      ShowTriggers(p[0], nick);
#ifdef USE_QUERY
    else if (!stricmp(p[0], GAMEQUERY_NAME)) // !query
      trig = NewCustomTrigger(from, nick, p[1], p[2]);
#endif
    else
    {
      trig = allowedtrigger(p[0], from);
      if (!trig)
        return;
    }

    if (trig)
    {
      QueryGameServer(&queries, trig, from, nick, p[1]
#ifdef USE_GENERATE
                     , 0
#endif
                     );
#ifdef USE_STATS
      network->stats.triggered++;
#endif
    }
  }
#else // USE_QUERY
  else
  {
    return;
  }
#endif

  AddFloodCheck(network, from);
}

int AdminValidate(char *nick, int flags)
{
  user_t *user;
  comrade_t *comrade;

#ifdef SUPERDEBUG
  superdebug("AdminValidate()\n");
#endif

  if (!nick)
    return 0;

  user = finduser(network, nick);
  if (!user)
    return 0;

  comrade = findcomrade(network, va("%s!%s@%s", user->nick, user->ident, user->address));
  if (comrade)
  {
    if (comrade->flags & flags)
      return 1;
  }

  return 0;
}

#ifdef USE_QUERY
int AdminValidateTrigger(char *nick, trigger_t *trig)
{
  user_t *user;
  comrade_t *comrade;

#ifdef SUPERDEBUG
  superdebug("AdminValidateTrigger()\n");
#endif

  if (!nick || !trig)
    return 0;

  user = finduser(network, nick);
  if (!user)
    return 0;

  comrade = findcomrade(network, va("%s!%s@%s", user->nick, user->ident, user->address));
  if (comrade)
  {
    if ((comrade->flags & COMRADEFLAG_SERVERS) && trig->id && comrade->access && (comrade->access & trig->id))
      return 1;
  }

  return 0;
}
#endif

#ifdef USE_STATS
void AdminNetStats(char *name, char *from)
{
  network_t *net;
  mergestats_t stats;
  unsigned long trigs;

#ifdef SUPERDEBUG
  superdebug("AdminNetStats()\n");
#endif

  if (!AdminValidate(name, COMRADEFLAG_COMMAND))
    return;

  AdminUptime(from);

  trigs = 0;
  SaveStatistics(1);

  for (net = networks; net; net = net->next)
  {
    InitMergeStats(&stats, net->group);
    LoadConfig(CONFIG_STATS, DELIM_CONFIG, &StatsParse, &stats);

    Say(MSG_PRIVMSG, from,
      va(""IRC_BOLD"%s"IRC_BOLD" (%s:%d) ["IRC_BOLD"Triggered"IRC_BOLD":%u]",
        net->group,
        (net->state == STATE_CONNECTED) ? inet_ntoa(net->connectaddr.sin_addr) : "not connected",
        (net->state == STATE_CONNECTED) ? ntohs(net->connectaddr.sin_port) : net->state,
        stats.triggered
      ));
    trigs += stats.triggered;
  }

  Say(MSG_PRIVMSG, from, va(IRC_BOLD"total"IRC_BOLD"["IRC_BOLD"Triggered"IRC_BOLD":%u (since 25th November 2003)]", trigs));
}
#endif

#ifdef USE_SERVICE
void AdminBorg(char *name, char *from)
{
  int r;

#ifdef SUPERDEBUG
  superdebug("AdminBorg()\n");
#endif

  if (!AdminValidate(name, COMRADEFLAG_COMMAND))
    return;

  r = (int)ShellExecute(NULL, "open", SERVICE_BORG, NULL, es.wdir, SW_HIDE);
  if (r <= 32)
  {
    Say(MSG_PRIVMSG, from, va("Assimilation failed: %d", r));
  }
}
#endif

void AdminVersion(char *from)
{
  Say(MSG_NOTICE, from,
      va(HEADER_TITLE " v" VERSION " " VER_SUFFIX " " __DATE__ " " __TIME__));
}

void AdminUptime(char *from)
{
  extern double starttime;
  //unsigned long trigs;
  char up[64], host[64];
  double d;
#ifdef UNIX
  FILE *fp;
#endif

#ifdef SUPERDEBUG
  superdebug("AdminUptime()\n");
#endif

  //if (!AdminValidate(name))
  //  return;

#ifdef WIN32
  d = GetTickCount();
  timelength(((d - starttime) / 1000.0), up, sizeof(up));
  timelength((d / 1000.0), host, sizeof(host));
#endif
#ifdef UNIX
  d = DoubleTime();
  timelength(((d - starttime) + d / 1000.0), up, sizeof(up));
  fp = fopen("/proc/uptime", "rt");
  if (fp)
  {
    fscanf(fp, "%lf %*f\n", &d);
    timelength(d, host, sizeof(host));
    fclose(fp);
  }
  else
  {
    strlcpy(host, "unknown", sizeof(host));
  }
#endif

  Say(MSG_PRIVMSG, from,
      va(IRC_BOLD HEADER_TITLE IRC_BOLD"["IRC_BOLD"Version"IRC_BOLD": " VERSION " (" VER_SUFFIX " " \
         __DATE__ " " __TIME__ ") " \
         IRC_BOLD"Uptime"IRC_BOLD":%s (host: %s)]", up, host));
}

void AdminRestart(char *name)
{
#ifdef SUPERDEBUG
  superdebug("AdminRestart()\n");
#endif

  if (!AdminValidate(name, COMRADEFLAG_COMMAND))
    return;

  ServerQuit(STATE_RESTART);
}

/* TODO: make work without sessionstart() happening next time log is used
void AdminFlushLogs(char *name)
{
  network_t *net;
  channel_t *chan;

#ifdef SUPERDEBUG
  superdebug("AdminFlushLogs()\n");
#endif

  if (!AdminValidate(name, COMRADEFLAG_COMMAND))
    return;

  for (net = networks; net; net = net->next)
  {
    if (net->log)
      fclose(net->log);

    for (chan = net->channels; chan; chan = chan->next)
    {
      if (chan->log)
        fclose(net->log);
    }
  }
}
*/

char *undil(char **p, int start, int pr)
{
  int i, len;
  char *s;

#ifdef SUPERDEBUG
  superdebug("undil()\n");
#endif

  if (start < 0 || start > MAX_DELIMITERS - 1)
    return NULL;

  for (i = start, len = 0; i < MAX_DELIMITERS && i < pr; i++)
  {
    len += strlen(p[i]);
  }

  len += pr - start;

  for (i = 0, s = p[start]; i < len; i++, s++)
  {
    if (!*s)
      *s = ' ';
  }

  return p[start];
}

void splithost(char *host, char **nick, char **ident, char **address)
{
  // nick!~ident@address
  char *s;

#ifdef SUPERDEBUG
  superdebug("splithost()\n");
#endif

  *nick = NULL;
  *ident = NULL;
  *address = NULL;

  s = host;
  *nick = s;
  for (; *s; s++)
  {
    if (*s == '!')
    {
      *s++ = '\0';
      *ident = s;
    }

    if (*s == '@')
    {
      *s++ = '\0';
      *address = s;
      break;
    }
  }
}

void splitnotice(char *to, int sz, char **msg)
{
  char *s, *p;
  char c;

#ifdef SUPERDEBUG
  superdebug("splitnotice()\n");
#endif

  while (**msg && ((**msg == ' ') || (**msg == '@')))
    *msg = *msg + 1; // *msg++; doesn't work here ---> er obviously, (*msg)++ would though, not risking it!

  s = *msg;
  while (*s && (*s != '#') && (*s != ')'))
    s++;
  if (*s != '#')
    return;

  p = s;
  while (*p && (*p != '/') && (*p != ')'))
    p++;
  c = *p;
  *p = '\0';
  strlcpy(to, s, sz);
  *p = c;
}

