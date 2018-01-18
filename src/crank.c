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

network_t *networks; //all networks
network_t *network; //current network

#ifdef USE_QUERY
//all triggers
trigger_t *triggers;
//all game servers
gameserver_t *gameservers;
//queries
activequery_t *queries;
//protocols
protocol_t *protocols;
#endif

essential_t es;
double starttime;

#ifdef USE_SERVICE
SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus;
#endif

#ifdef SUPERDEBUG
FILE *superlog = NULL;
#endif

int main(int argc, char *argv[])
{
  int r;

  es.wdir = workingdir(argv[0]);
#ifdef WIN32
  starttime = 0.0;
#endif

#ifdef SUPERDEBUG
  superdebug("main()\n");
#endif

#ifdef USE_SERVICE
  if (!CommandLine(argc, argv))
    return 0;
#endif

#ifdef USE_SERVICE
  if (es.runservice)
  {
    SERVICE_TABLE_ENTRY st[] =
    {
      {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
      {NULL, NULL}
    };

    if (!StartServiceCtrlDispatcher(st))
      return 1;
    return 0;
  }
#endif

  r = Begin();

#ifdef SUPERDEBUG
  superdebug("main finished()\n");
  if (superlog)
    fclose(superlog);
#endif

  return r;
}

int Begin(void)
{
  int r;

#ifdef SUPERDEBUG
  superdebug("Begin()\n");
#endif

  do
  {
    if ((r = Init()))
    {
      Frame();
    }

    Finish();

    if (!r)
    {
#ifdef SUPERDEBUG
      superdebug("Begin(): Init() failed\n");
#endif
      return 1;
    }

#ifdef WIN32
#ifdef _DEBUG
    MemLeakCheckAndFree();
#endif
#endif
  } while (es.state == STATE_RESTART);

#ifdef SUPERDEBUG
  superdebug("Begin() finished\n");
#endif

#ifdef _DEBUG
  {
    printf("\nPress ENTER to exit\n");
    getchar();
  }
#endif

  return 0;
}

int Init(void)
{
#ifdef USE_LOGGING
  channel_t *ch;
#endif
  double d;
  int i;
#ifdef WIN32
  WSADATA wsaData;
#endif

#ifdef SUPERDEBUG
  superdebug("Init()\n");
#endif

#ifdef WIN32
  if (!es.runservice)
#endif
    printf ("%s v%s - %s %s (%s)\n\n", HEADER_TITLE, VERSION, HEADER_COPYRIGHT, HEADER_AUTHOR, HEADER_EMAIL);

  es.state = STATE_ACTIVE;
  networks = NULL;
  network = NULL;
#ifdef USE_QUERY
  triggers = NULL;
  gameservers = NULL;
  queries = NULL;
  protocols = NULL;
#endif
  es.sockwait = 0;
#ifdef USE_STATS
  es.outrx = 0;
  es.outtx = 0;
#endif
#ifdef USE_GENERATE
  es.outrate = 0;
#endif
  //DOUBLETIME() INIT
#ifdef WIN32
  if (starttime == 0.0)
    starttime = GetTickCount();
#endif
  //PUT ALL DOUBLETIME USAGE BELOW HERE
  d = DoubleTime();
#ifdef USE_STATS
  es.lastsave = d;
#endif
#ifdef USE_GENERATE
  es.nextoutput = d + 30.0;
  es.nextnetgen = d + 30.0;
#endif
#ifdef UNIX
  if (starttime == 0.0)
    starttime = d;
  mkdir(va("%s" CONFIG_LOGDIR, es.wdir), CONFIG_LOG_MODE);
#endif
#ifdef UNIX
  signal(SIGTERM, &HandleSignal);
  signal(SIGINT, &HandleSignal);
#endif

#ifdef WIN32
  mkdir(va("%s" CONFIG_LOGDIR, es.wdir));
#ifdef _DEBUG
  SetMemLeakLogProc(&MemLeakLog);
#endif

  if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0)
  {
    echo(NULL, ECHO_SHOW, "Error: WSAStartup() failed\n");
    return 0;
  }
#endif

  LoadConfig(CONFIG_MAIN, DELIM_CONFIG, &MainParse, NULL);
#ifdef USE_QUERY
  LoadConfig(CONFIG_PROTOCOL, DELIM_CONFIG, &ProtocolParse, NULL);
  LoadConfig(CONFIG_QUERY, DELIM_CONFIG, &QueryParse, NULL);
#endif

  if (!networks)
  {
    echo(NULL, ECHO_SHOW, "Error: No networks specified\n");
    return 0;
  }

  for (i = 0, network = networks; network; network = network->next, i++)
  {
    if (!network->servers)
    {
      echo(NULL, ECHO_SHOW, va("Error: Network %s has no servers\n", network->group));
      return 0;
    }

    if (!network->channels)
    {
      echo(NULL, ECHO_SHOW, va("Error: Network %s has no channels\n", network->group));
      return 0;
    }

#ifdef USE_LOGGING
#ifdef UNIX
    mkdir(va("%s" CONFIG_LOGDIR "%s", es.wdir, network->group), CONFIG_LOG_MODE);
#endif
#ifdef WIN32
    mkdir(va("%s" CONFIG_LOGDIR "%s", es.wdir, network->group));
#endif

    for (ch = network->channels; ch; ch = ch->next)
    {
#ifdef UNIX
      mkdir(va("%s" CONFIG_LOGDIR "%s/%s", es.wdir, network->group, ch->name), CONFIG_LOG_MODE);
#endif
#ifdef WIN32
      mkdir(va("%s" CONFIG_LOGDIR "%s/%s" , es.wdir, network->group, ch->name));
#endif
    }
#endif
  }

  es.sockwait = (SOCKWAIT / i);
  if (es.sockwait < 10)
    es.sockwait = 10;
#ifdef PARANOID
  if (es.sockwait > 100)
    es.sockwait = 100;
#endif

#ifdef USE_SERVICE
  if (es.runservice && (es.runservice != SERVICE_DONE_INIT))
  {
    ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
    SetServiceStatus(hStatus, &ServiceStatus);
    es.runservice = SERVICE_DONE_INIT;
  }
#endif

  return 1;
}

void Frame(void)
{
  char rv[MAX_MESSAGE_SIZE+1];
  int i;
  int hsz;

#ifdef SUPERDEBUG
  //superdebug("Frame()\n");
#endif

#ifdef WIN32
  hsz = sizeof(SOCKADDR);
#endif
#ifdef UNIX
  hsz = sizeof(struct sockaddr_in);
#endif

  while (es.state > 0)
  {
    for (network = networks; network; network = network->next)
    {
      if (!ServerConnect())
        continue;
      
      KeepAlive();
      NickRetake();
      JoinAttempt();
      FlushBuffers();
      ExpiredFloodCheck();
      
      if (SocketWait(network->sock, es.sockwait) == -1)
      {
#ifdef PARANOID
        _sleep(10);
#endif
      }

      i = recv(network->sock, rv, MAX_MESSAGE_SIZE, 0);
      if (i == -1)
        continue;
      rv[i] = '\0';
#ifdef USE_STATS
      network->stats.tcprx += i;
#endif
      Parse(rv, i);

    }

#ifdef USE_STATS
    SaveStatistics(0);
#endif

#ifdef USE_QUERY
    ProcessQueries();
#endif

#ifndef _DEBUG

#ifdef USE_GENERATE
    GenerateNetworkPage();
#ifdef USE_QUERY
    QueryGenerateServers();
#endif
#endif

#endif
  }
}

void Finish(void)
{
#ifdef SUPERDEBUG
  superdebug("Finish()\n");
#endif

#ifdef USE_STATS
  SaveStatistics(1);
#endif

  RemoveNetworks();
#ifdef USE_QUERY
  RemoveActiveQueries(&queries);
  RemoveTriggers();
  RemoveGameServers();
  RemoveProtocols(&protocols);
#endif

#ifdef USE_GENERATE
  if (es.outpath)
    _free_(es.outpath);
  if (es.outext)
    _free_(es.outext);
  if (es.outurl)
    _free_(es.outurl);
#endif

#ifdef WIN32
  WSACleanup();
#endif
}

#ifdef USE_SERVICE
int CommandLine(int argc, char **argv)
{
  int i;

#ifdef SUPERDEBUG
  superdebug("CommandLine()\n");
#endif

  for (i = 1; (argc > 1) && (i < argc); i++)
  {
    if (!strcmp(argv[i], SWITCH_RUNSERVICE))
      es.runservice = SERVICE_NO_INIT;
    else if (!strcmp(argv[i], SWITCH_INSTALLSERVICE))
    {
      InstallService();
      return 0;
    }
    else if (!strcmp(argv[i], SWITCH_UNINSTALLSERVICE))
    {
      UninstallService();
      return 0;
    }
  }

  return 1;
}
#endif

int ServerConnect(void)
{
  int r;

  if (network->state == SOCKETSTATE_CONNECTED)
    return 1;

  if (network->state == SOCKETSTATE_FAILEDCONNECT)
  {
    double t = DoubleTime();

    if (network->lastconnect == 0.0)
    {
      echo(NULL, ECHO_NORMAL, va("* Couldn\'t connect to %s, waiting %d seconds\n", network->group, es.delay));
      network->connect = network->connect->next;
      network->lastconnect = t + (double)es.delay;
      return 0;
    }
    else if (network->lastconnect < t)
    {
      network->state = SOCKETSTATE_DISCONNECTED;
    }
    else
    {
      //printf("waiting..%f\n", network->lastconnect - DoubleTime());
      _sleep(20);
      //SocketWait(network->sock, 10); <- no point, socket is dead
      return 0;
    }
  }

  if (network->state == SOCKETSTATE_DISCONNECTED)
  {
    if (!network->connect)
      network->connect = network->servers;

    network->connectaddr.sin_port = htons((short)network->connect->port);
    if (validip(network->connect->addr))
      network->connectaddr.sin_addr.s_addr = inet_addr(network->connect->addr);
    else
    {
      char *s = resolve(network->connect->addr, &network->connect->pos);
      if (!s)
      {
        network->connect = network->connect->next;
        return 0;
      }
      network->connectaddr.sin_addr.s_addr = inet_addr(s);
    }

    echo(NULL, ECHO_NORMAL, va("* Connecting to %s (%s:%d)...\n", network->group, inet_ntoa(network->connectaddr.sin_addr), ntohs(network->connectaddr.sin_port))); //display
  }

  r = ConnectSocket(&network->sock, &network->connectaddr, &network->state, &network->lastconnect);

  if (network->state == SOCKETSTATE_SOCKCONNECTED)
  {
    Register();
    return 1;
  }

  return r;
}

int SendNetSocket(network_t *net, const char *data, int sz)
{
#ifdef SUPERDEBUG
  superdebug("SendTCP()\n");
#endif

#ifdef PARANOID
  if (!net)
    return INVALID_SOCKET;
#endif

  if (sz == -1)
    sz = strlen(data);

  if (!sz || net->sock == INVALID_SOCKET)
    return INVALID_SOCKET;

#ifdef USE_STATS
  net->stats.tcptx += sz;
#endif

  return send(net->sock, data, sz, 0);
}

void Shutdown(void)
{
  ServerQuit(STATE_SHUTDOWN);
}

void ServerQuit(int r) //quits _all_ servers
{
  network_t *net;
	time_t long_time;
#ifdef USE_LOGGING
  channel_t *chan;
#endif
  const char *msg = "QUIT :" QUIT_MESSAGE "\r\n";
  int len = strlen(msg);
	
#ifdef SUPERDEBUG
  superdebug("ServerQuit()\n");
#endif

	time(&long_time);

  for (net = networks; net; net = net->next)
  {
    SendNetSocket(net, msg, len); //send message immediately, doesn't matter if we'll excess flood, we're quitting anyway
  }

#ifdef USE_LOGGING
  for (net = networks; net; net = net->next)
  {
    SessionClose(&net->log, &long_time);

    for (chan = net->channels; chan; chan = chan->next)
      SessionClose(&chan->log, &long_time);
  }
#endif

  es.state = r;
}

void echo(char *name, int flags, char *str)
{
#ifdef USE_LOGGING
  FILE *fp;
#endif
  char stamp[MAX_TIMESTAMP+1];
  struct tm *t;
	time_t long_time;

#ifdef SUPERDEBUG
  superdebug("echo()\n");
#endif

	time(&long_time);
	t = localtime(&long_time);

  if (t)
    _strnprintf(stamp, sizeof(stamp), "%02d:%02d", t->tm_hour, t->tm_min);
  else
    *stamp = '\0';

  if ((flags & ECHO_SHOW) && !es.silent
#ifdef USE_SERVICE
    && !es.runservice
#endif
     )
  {
    {
      if ((flags & ECHO_NOSTAMP) || !network)
        printf("%s", str);
      else
        printf("%s:%s %s", stamp, network->grp,  str);
    }
  }

#ifdef USE_LOGGING
  if (flags & ECHO_WRITE)
  {
    fp = LogFile(name);
    if (!fp)
      return;
    if (flags & ECHO_NOSTAMP)
      fprintf(fp, "%s", str);
    else
      fprintf(fp, "[%s] %s", stamp, str);
    fflush(fp);
  }
#endif
}

#ifdef USE_LOGGING
FILE *LogFile(char *name)
{
  network_t *net;
  channel_t *chan;
  struct tm *t;
  static int day = -1;
  static int month = -1;
  static int year = -1;
	time_t long_time;

#ifdef SUPERDEBUG
  superdebug("LogFile()\n");
#endif
	
	time(&long_time);
	t = localtime(&long_time);

  if (!t)
    return NULL;

  //new day, close all logs
  /*Session Start: Fri Dec 03 14:58:01 2004
Session Ident: #bucket
[14:58] *** Now talking in #bucket
[14:58] * Attempting to rejoin channel #bucket
[14:58] *** Now talking in #bucket
[14:58] *** crank sets mode: +o q9q5f8w3g
Session Close: Fri Dec 03 15:04:22 2004*/
  if (t->tm_mday != day || t->tm_mon != month || t->tm_year != year)
  {
    for (net = networks; net; net = net->next)
    {
      SessionClose(&net->log, &long_time);

      for (chan = net->channels; chan; chan = chan->next)
        SessionClose(&chan->log, &long_time);
    }

    day = t->tm_mday;
    month = t->tm_mon;
    year = t->tm_year;
  }

  if (!name)
  {
    //kaboom_20050121.log | <networkgroup>_<year><month><day>.log
    if (!network->log)
    {
      if (!SessionStart(&network->log, NULL, network->group, NULL, &long_time, t))
        return NULL;
    }

    return network->log;
  }

  chan = findchannel(network, name);
  if (!chan)
    return NULL;

  //#foo_20050121.log | <#channel>_<year><month><day>.log
  if (!chan->log)
  {
    if (!SessionStart(&chan->log, chan->name, network->group, name, &long_time, t))
      return NULL;
  }

  return chan->log;
}

int SessionStart(FILE **fp, char *prefix, char *group, char *name, time_t *long_time, struct tm *t)
{
  char path[MAX_PATH];

#ifdef SUPERDEBUG
  superdebug("SessionStart()\n");
#endif

  if (*fp)
    return 0;

  _strnprintf(path, sizeof(path), "%s" CONFIG_LOGDIR "%s/%s%s" "%s%s%s_%04d%02d%02d.log",
    es.wdir,
    network->group,
    (name) ? name : "",
    (name) ? "/" : "",
    (prefix) ? prefix : "",
    (prefix) ? "_" : "",
    group,
    t->tm_year + 1900,
    t->tm_mon + 1,
    t->tm_mday);

  *fp = fopen(path, "at");
  if (!*fp)
    return 0;

  fprintf(*fp, "Session Start: %s", ctime(long_time));

  return 1;
}

int SessionClose(FILE **fp, time_t *long_time)
{
#ifdef SUPERDEBUG
  superdebug("SessionClose()\n");
#endif

  if (!*fp)
    return 0;

  fprintf(*fp, "Session Close: %s\n", ctime(long_time));
  fclose(*fp);
  *fp = NULL;

  return 1;
}
#endif

int MainParse(char p[][MAX_PARAMSIZE+1], void *data)
{
#ifdef SUPERDEBUG
  superdebug("MainParse()\n");
#endif

  if (!strcmp(p[0], "network"))
    AddNetwork(p);
  else if (!strcmp(p[0], "server"))
    AddNetServer(p);
  else if (!strcmp(p[0], "friend"))
    AddComrade(p);
  else if (!strcmp(p[0], "link"))
    AddLink(p);
  else if (!strcmp(p[0], "user"))
    SetUser(p);
#ifdef USE_GENERATE
  else if (!strcmp(p[0], "generate"))
    SetOutput(p);
#endif
  else if (!strcmp(p[0], "auth"))
    SetAuth(p);
  else if (!strcmp(p[0], "delay"))
    es.delay = atoi(p[1]);
  else if (!strcmp(p[0], "silent"))
    es.silent = atoi(p[1]);
#ifdef USE_QUERY
  else if (!strcmp(p[0], "leetify"))
    es.leetify = atoi(p[1]);
#endif
  else if (p[0][0] == '#')
    AddChannel(p);
  else
    echo(NULL, ECHO_SHOW, va("Unknown: %s\n", p));

  return 1;
}

void SetUser(char p[][MAX_PARAMSIZE+1])
{
  network_t *net;

#ifdef SUPERDEBUG
  superdebug("SetUser()\n");
#endif

  net = findnetwork(p[1]);
  if (!net)
  {
    echo(NULL, ECHO_SHOW, va("Can\'t set user details for group \"%s\"\n", p[1]));
    return;
  }

  strlcpy(net->nick, p[2], sizeof(net->nick));
  strlcpy(net->mynick, p[2], sizeof(net->mynick));
  strlcpy(net->ident, p[3], sizeof(net->ident));
  strlcpy(net->fullname, p[4], sizeof(net->fullname)); 
}

#ifdef USE_GENERATE
void SetOutput(char p[][MAX_PARAMSIZE+1])
{
  int sz;

#ifdef SUPERDEBUG
  superdebug("SetOutput()\n");
#endif

  /* generate:localhost/crank/:php:e:\apache2\htdocs\crank\ */

  if (!p[1] || !p[2] || !p[4])
    return;

  es.outurl = _strdup_(p[1]);
  es.outext = _strdup_(p[2]);

  sz = strlen(p[4]) + 1;
  if (*p[3])
    sz += strlen(p[3]) + 1; // :

  es.outpath = (char *)_malloc_(sz);
  if (es.outpath)
  {
    es.outpath[0] = '\0';
    if (*p[3])
    {
      strlcpy(es.outpath, p[3], sz);
      strlcat(es.outpath, ":", sz);
    }
    strlcat(es.outpath, p[4], sz);
  }
}
#endif

void SetAuth(char p[][MAX_PARAMSIZE+1])
{
  network_t *net;

  net = findnetwork(p[1]);
  if (!net)
  {
    echo(NULL, ECHO_SHOW, va("Can\'t set auth details for group \"%s\"\n", p[1]));
    return;
  }

  strlcpy(net->auth.authserv, p[2], sizeof(net->auth.authserv));
  strlcpy(net->auth.authline, p[3], sizeof(net->auth.authline));
  strlcpy(net->auth.authreply, p[4], sizeof(net->auth.authreply));
  net->auth.flags = atoi(p[5]);
}

#ifdef USE_STATS
int StatsParse(char p[][MAX_PARAMSIZE+1], void *data)
{
  mergestats_t *stats = (mergestats_t *)data;

#ifdef SUPERDEBUG
  superdebug("StatsParse()\n");
#endif

  //triggered:group:val
  //tcpinfo:group:tcptx:tcprx
  //udpinfo:group:udptx:udprx

  if (!strcmp(p[1], stats->network))
  {
    if (!strcmp(p[0], "triggered"))
    {
      stats->triggered = atol(p[2]);
    }
    else if (!strcmp(p[0], "tcpinfo"))
    {
      stats->tcptx = atof(p[2]);
      stats->tcprx = atof(p[3]);
    }
    else if (!strcmp(p[0], "udpinfo"))
    {
      stats->udptx = atof(p[2]);
      stats->udprx = atof(p[3]);
    }
  }

  return 1;
}
#endif

#ifdef USE_STATS
void InitMergeStats(mergestats_t *stats, char *group)
{
  strlcpy(stats->network, group, sizeof(stats->network));
  stats->tcprx = 0.0;
  stats->tcptx = 0.0;
  stats->udprx = 0.0;
  stats->udptx = 0.0;
  stats->triggered = 0;
}
#endif

#ifdef USE_STATS
void SaveStatistics(int now)
{
  network_t *net;
  mergestats_t stats;
  FILE *fp;
  char statfile[MAX_PATH];
  char statbakfile[MAX_PATH];
  double d;
  int i;

  d = DoubleTime();
	if (!now && (!((d - es.lastsave) >= TIME_SAVE_STATS)))
    return;

#ifdef SUPERDEBUG
  superdebug("SaveStatistics()\n");
#endif

  es.lastsave = d;
  _strnprintf(statfile, sizeof(statfile), "%s%s", es.wdir, CONFIG_STATS);
  _strnprintf(statbakfile, sizeof(statbakfile), "%s%s", es.wdir, CONFIG_STATS_BAK);
  
  i = rename(statfile, statbakfile);

  fp = fopen(statfile, "wt");
  if (fp)
  {
    for (net = networks; net; net = net->next)
    {
      InitMergeStats(&stats, net->group);
      if (!i)
        LoadConfig(CONFIG_STATS_BAK, DELIM_CONFIG, &StatsParse, &stats);

      stats.triggered += net->stats.triggered;
      d = net->stats.tcptx;
      stats.tcptx += d / 1024 / 1024;
      d = net->stats.tcprx;
      stats.tcprx += d / 1024 / 1024;
      d = net->stats.udptx;
      stats.udptx += d / 1024 / 1024;
      d = net->stats.udprx;
      stats.udprx += d / 1024 / 1024;

      fprintf(fp, "triggered:%s:%u\n", net->group, stats.triggered);
      fprintf(fp, "tcpinfo:%s:%0.6f:%0.6f\n", net->group, stats.tcptx, stats.tcprx);
      fprintf(fp, "udpinfo:%s:%0.6f:%0.6f\n", net->group, stats.udptx, stats.udprx);

      net->stats.triggered = 0;
      net->stats.tcptx = 0;
      net->stats.tcprx = 0;
      net->stats.udptx = 0;
      net->stats.udprx = 0;
    }

    InitMergeStats(&stats, "generate");
    if (!i)
      LoadConfig(CONFIG_STATS_BAK, DELIM_CONFIG, &StatsParse, &stats);

    d = es.outtx;
    stats.udptx += d / 1024 / 1024;
    d = es.outrx;
    stats.udprx += d / 1024 / 1024;

    fprintf(fp, "udpinfo:generate:%0.6f:%0.6f\n", stats.udptx, stats.udprx);

    es.outtx = 0;
    es.outrx = 0;

    fclose(fp);
  }

  remove(statbakfile);
}
#endif

char *BaseDir(char *file)
{
  return va("%s%s", es.wdir, file);
}

/* -------------------------------------------------------------------------- */

//DoubleTime, from the great game that is Quake

#ifdef WIN32
double DoubleTime(void)
{
	double now;

#ifdef SUPERDEBUG
  //superdebug("DoubleTime()\n");
#endif

	now = GetTickCount();

	if (now < starttime)
		return (now / 1000.0) + (LONG_MAX - starttime / 1000.0);

	if ((now - starttime) == 0)
		return 0.0;

	return (now - starttime) / 1000.0;
}
#endif

#ifdef UNIX
double DoubleTime(void)
{
	struct timeval tp;
	static int secbase = 0;
  
#ifdef SUPERDEBUG
  //superdebug("DoubleTime()\n");
#endif

	gettimeofday(&tp, NULL);
	
	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000000.0;
	}
	
	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}
#endif

void LoadConfig(char *file, char dil, void *func, void *data)
{
  FILE *fp;
  char p[MAX_PARAM][MAX_PARAMSIZE+1];
  char c;
  int len, i, n;
  config_parser parse;

#ifdef SUPERDEBUG
  superdebug("LoadConfig()\n");
#endif

  parse = *(config_parser *)&func;

  fp = fopen(BaseDir(file), "rt");
  if (!fp)
  {
    printf("Failed to read %s\n", file);
    return;
  }

  c = ' ';
  while (c != EOF)
  {
    memset(p, '\0', sizeof(p));
    i = 0;

    while (c == ' ' || c == '\t' || c == '\r' || c == '\n')
      c = fgetc(fp);

    if (c == EOF)
      break;

    while (1)
    {
      if (i > MAX_PARAM - 1)
      {
        while (c != '\n' && c != '\r' && c != EOF)
          c = fgetc(fp);
        goto _proc;
      }

      n = 1;
      len = 0;
      while (n)
      {
        if (c == dil)
          n = 0;
        else if (c == '\n' || c == '\r' || c == EOF)
          goto _proc;
        else if (len < MAX_PARAMSIZE)
        {
          p[i][len] = c; //no need to terminate because it is memset to null above
          len++;
        }
        c = fgetc(fp);
      }

      i++;
    }
_proc:
    //process
    if (!strncmp(p[0], "//", 2))
      continue;

    if (!parse(p, data))
      break;
  }

  fclose(fp);
}

#ifdef USE_SERVICE
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
  int r;

#ifdef SUPERDEBUG
  superdebug("ServiceMain()\n");
#endif

  ServiceStatus.dwServiceType = SERVICE_WIN32;
  ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  ServiceStatus.dwWin32ExitCode = 0;
  ServiceStatus.dwServiceSpecificExitCode = 0;
  ServiceStatus.dwCheckPoint = 0;
  ServiceStatus.dwWaitHint = 0;

  hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ServiceControl);
  if (!hStatus)
  {
    StopService(-1);
    return;
  }

  ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  SetServiceStatus(hStatus, &ServiceStatus);

  r = Begin();
  if (r)
  {
    StopService(r);
  }
}
#endif

#ifdef USE_SERVICE
void WINAPI ServiceControl(DWORD request) 
{
  switch(request) 
  { 
    case SERVICE_CONTROL_PAUSE: 
      ServiceStatus.dwCurrentState = SERVICE_PAUSED;
      break;
    case SERVICE_CONTROL_CONTINUE:
      ServiceStatus.dwCurrentState = SERVICE_RUNNING;
      break;
    case SERVICE_CONTROL_INTERROGATE:
      break;
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
      StopService(0);
      Shutdown();
      break;
    default:
      break;
  } 

  SetServiceStatus(hStatus, &ServiceStatus);
}
#endif

#ifdef USE_SERVICE
void StopService(int code)
{
  ServiceStatus.dwCurrentState = SERVICE_STOPPED;
  ServiceStatus.dwWin32ExitCode = (code) ? ERROR_SERVICE_SPECIFIC_ERROR : 0;
  ServiceStatus.dwServiceSpecificExitCode = (code) ? code : NO_ERROR;
  SetServiceStatus(hStatus, &ServiceStatus); 
}
#endif

#ifdef USE_SERVICE
void InstallService(void)
{
	SC_HANDLE ser, scm;
  char dir[MAX_PATH];
  char path[MAX_PATH];

#ifdef SUPERDEBUG
  superdebug("InstallService()\n");
#endif
  
  if (!GetModuleFileName(NULL, dir, sizeof(dir)))
  {
    printf("Failed to access executable path\n");
    return;
  }

	scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (!scm)
  {
    printf("Failed to access services manager\n");
    return;
  }

  _snprintf(path, sizeof(path), "%s %s", dir, SWITCH_RUNSERVICE);

	ser = CreateService(scm, SERVICE_NAME, SERVICE_DESCRIPTION, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, path, NULL, NULL, NULL, NULL, NULL);
  if (!ser)
	{
    printf("Failed to install service\n");
    goto _re;
	}

	CloseServiceHandle(ser);
_re:
	CloseServiceHandle(scm);
}
#endif

#ifdef USE_SERVICE
void UninstallService(void)
{
	SC_HANDLE ser, scm;

#ifdef SUPERDEBUG
  superdebug("UninstallService()\n");
#endif

	scm = OpenSCManager(0, 0, DELETE);
	if (!scm)
  {
    printf("Failed to access services manager\n");
    return;
  }

  ser = OpenService(scm, SERVICE_NAME, DELETE);
  if (!ser)
	{
    printf("Failed to access service\n");
    goto _re;
	}

  if (!DeleteService(ser)) 
	{
    printf("Failed to uninstall service\n");
    goto _re;
	}

	CloseServiceHandle(ser);
_re:
	CloseServiceHandle(scm);
}
#endif

#ifdef UNIX
void HandleSignal(int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
  {
    ServerQuit(STATE_SHUTDOWN);
  }
}
#endif


/* -------------------------------------------------------------------------- */

#ifdef WIN32
void MemLeakLog(char *szmsg)
{
  FILE *fp;

#ifdef SUPERDEBUG
  superdebug("MemLeakLog()\n");
#endif
 
  fp = fopen(va("%s" CONFIG_LOGDIR "leak.txt", es.wdir), "a");
  if (fp)
  {
    fprintf(fp, "%s", szmsg);
    fclose(fp);
  }
}
#endif

#ifdef SUPERDEBUG
void superdebug(char *msg)
{
  static int foo = 0;

  if (foo > 100)
  {
    if (superlog)
      fclose(superlog);
    superlog = NULL;
    foo = 0;
  }

  if (!superlog)
  {
    char *log = superdebuglogname();
    if (*log)
      superlog = fopen(log, "wt");
  }

  if (!superlog)
    return;

  fprintf(superlog, msg);
  fflush(superlog);

  foo++;
}

char *superdebuglogname(void)
{
  struct tm *t;
	time_t long_time;
  static char superlogname[128];
  static int started = 0;

  if (started)
    return superlogname;

  superlogname[0] = '\0';

	time(&long_time);
	t = localtime(&long_time);

  if (t)
  {
    _strnprintf(superlogname, sizeof(superlogname), "%s%sdebug_%d%d%d_%02d%02d%02d.log",
        es.wdir, CONFIG_LOGDIR,
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec
        );
  }

  started = 1;

  return superlogname;
}

#endif
