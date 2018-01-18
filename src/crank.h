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

#ifndef _H_CRANK
#define _H_CRANK

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#include <direct.h>
#include <limits.h>
//http://www.tunesmithy.co.uk/memleakcheck/index.htm
#include "MemLeakCheck.h"
#endif

#ifdef UNIX
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#endif

#include "sockets.h"
#include "util.h"
#include "hash.h"

#define HEADER_TITLE "crank"
#define HEADER_AUTHOR "bliP"
#define HEADER_EMAIL "spawn[at]nisda.net"
#define HEADER_COPYRIGHT "Copyright 2005-2007"
#define HEADER_URL "http://nisda.net/"
#define VERSION "1.545"
#ifdef WIN32
#define VER_SUFFIX "Win32"
#endif
#ifdef UNIX
#define VER_SUFFIX "Unix"
#endif

#define STR(s) DISPSTR(s)
#define DISPSTR(s) #s

#define DEFAULT_IRC_PORT 6667
#define DEFAULT_TIMEOUT 30.0

#define CONFIG_LOGDIR "logs/"
#define CONFIG_TEMPLATEDIR "tpl/"
#define CONFIG_QSTATDIR "query/"
#define CONFIG_MAIN "crank.conf"
#define CONFIG_QUERY "query.conf"
#define CONFIG_PROTOCOL "games.conf"
#define CONFIG_STATS "stats.conf"
#define CONFIG_STATS_BAK "stats.bak"
#define CONFIG_DESC "desc.conf"
#ifdef WIN32
#define CONFIG_QSTAT "query.bat"
#endif
#ifdef UNIX
#define CONFIG_QSTAT "query.sh"
#endif
#define CONFIG_LOG_MODE 0700

#define SERVICE_EXE_FILE "crank.exe"
#define SERVICE_UPGRADE_FILE "crank.up"
#define SERVICE_BORG "borg.exe"
#define SERVICE_NAME "crank"
#define SERVICE_DESCRIPTION HEADER_URL
#define SERVICE_NO_INIT 1
#define SERVICE_DONE_INIT 2

#define SWITCH_RUNSERVICE "--service"
#define SWITCH_INSTALLSERVICE "--register"
#define SWITCH_UNINSTALLSERVICE "--unregister"

#define DELIM_CONFIG ':'
#define DELIM_QSTAT '\t'

#define MSG_NOTICE 0
#define MSG_PRIVMSG 1

#define QUIT_MESSAGE HEADER_TITLE " v" VERSION " - " HEADER_URL

#define KEEP_ALIVE "1§"

#define IRC_BOLD "\x02"
#define IRC_COLOUR "\x03"
#define IRC_UNDER "\x1F"

#define DEFAULT_CHANNEL_MODE "+nt"
#define DEFAULT_HIDDEN_MODE "+x"

#define TIME_NICK_RETAKE 15.0
#define TIME_KEEP_ALIVE 30.0
#define TIME_JOIN_ATTEMPT 30
#define TIME_JOIN_KICKED 120
#define TIME_SAVE_STATS 300.0 //5 mins
#define TIME_NETWORK_GENERATE 120.0

#define MAX_IP 15
#define MAX_PORT 5
#define MAX_SERVER_ADDR 48
#define MAX_PASSWORD 16
#define MAX_GROUP 16
#define MAX_SHORTGROUP 4
#define MAX_HOSTNAME 64

#define MAX_NICKNAME 30
#define MAX_FULLNAME 24
#define MAX_IDENT 9
#define MAX_ADDRESS 64

#define MAX_TIMESTAMP 5
#define MAX_PREFIX 2

#define LINE_CHUNK 32
#define SECTION_CHUNK 512

#define MAX_HASH_SIZE 64
#define MAX_HASH_NAME 32

#define MAX_MESSAGE_SIZE 1408
#define MAX_TRANSFER 500
#define MAX_TOTAL_TRANSFER 3000
#define MAX_TRANSFER_DELAY 20.0
#define MAX_TRANSFER_LINES 5
#define MAX_TRANSFER_RESET 32.0

#define MAX_BURST_SEND 3.0
#define MAX_BUFFER_SEND 2.0

#define SOCKWAIT 50 //ms

//
//#define CHANLEN 50
//#define FLAGLEN 64
//#define HOSTLEN 63
//#define USERLEN 16
//#define NICKLEN 20
//#define USERHOSTLEN HOSTLEN + USERLEN + NICKLEN + 2
//

#define MAX_CHANNEL 32
#define MAX_KEY 24
#define MAX_TOPIC 320
#define MAX_MODE 24
#define MAX_DEFAULTTOPIC 32
#define MAX_DEFAULTMODE 12

#define MAX_QUERYUSERHOST 5

#define MAX_DELIMITERS 17
#define PRE 0
#define CMD 1
#define PARAM0 2
#define PARAM1 3
#define PARAM2 4
#define PARAM3 5
#define PARAM4 6
#define PARAM5 7
#define PARAM6 8
#define PARAM7 9
#define PARAM8 10
#define PARAM9 11
#define PARAM10 12
#define PARAM11 13
#define PARAM12 14
#define PARAM13 15
#define PARAM14 16

#define QUERYSYMBOL_TRIGGER '!'
#define GAMEQUERY_ME "crank"
#define GAMEQUERY_NAME "query"

#define SYMBOL_POSITIVE '+'
#define SYMBOL_NEGATIVE '-'

#define SYMBOL_IRCOP '!'
#define SYMBOL_VOICED '+'
#define SYMBOL_HALFOP '%'
#define SYMBOL_CHANOP '@'

#define SYMBOL_MODECHANOP 'o'
#define SYMBOL_MODEHALFOP 'h'
#define SYMBOL_MODEVOICED 'v'
#define SYMBOL_MODEKEY 'k'
#define SYMBOL_MODEBAN 'b'
#define SYMBOL_MODELIMIT 'l'

#define SYMBOL_CTCP 0x01

//#define CHANNELFLAG_NONE 0
//#define CHANNELFLAG_SEENTOPIC 1
//#define CHANNELFLAG_SEENSETBY 2

#define USERFLAG_IRCOP 1
#define USERFLAG_SENTUSERHOST 2
#define USERFLAG_RECVUSERHOST 4
#define USERFLAG_SHOWIRCOP 8
#define USERFLAG_IGNORED 16

#define NICKFLAG_NORMAL 0
#define NICKFLAG_VOICED 1
#define NICKFLAG_HALFOP 2
#define NICKFLAG_CHANOP 4
#define NICKFLAG_BANNED 8
#define NICKFLAG_SHOWOP 16

#define COMRADESYMBOL_AUTOOP 'a'
#define COMRADESYMBOL_SERVERS 's'
#define COMRADESYMBOL_MATCHCAST 'm'
#define COMRADESYMBOL_COMMAND 'c'
#define COMRADESYMBOL_OVERRIDE 'o'

#define COMRADEFLAG_AUTOOP 1
#define COMRADEFLAG_SERVERS 2
#define COMRADEFLAG_MATCHCAST 4
#define COMRADEFLAG_OVERRIDE 8
#define COMRADEFLAG_COMMAND 16

#define AUTHFLAG_JOINWAIT 2

#define ECHO_NONE 0
#define ECHO_WRITE 1
#define ECHO_SHOW 2
#define ECHO_NOSTAMP 4

#define ECHO_NORMAL (ECHO_WRITE | ECHO_SHOW)

#define RAND_NUMBER 0
#define RAND_CHARACTER 1
#define RAND_NONE 2

#define FLOOD_MAX_TRIGGERS 3
#define FLOOD_MAX_TIME 120

#define DEFAULT_PAGE_UPDATE 5 //5 mins
#define MAX_TEMPLATE_NAME 8
#define MAX_SECTION_NAME 32
#define MAX_SECTION_CHUNK 128

#define MAX_AUTHLINE 32
#define MAX_AUTHREPLY 32

#define STATE_ACTIVE 1
#define STATE_RESTART 0
#define STATE_SHUTDOWN -1

#define MAX_PARAMSIZE 128
#define MAX_PARAM 8

#ifdef WIN32
#define ECONNREFUSED WSAECONNREFUSED
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTSOCK WSAENOTSOCK
#define EISCONN WSAEISCONN
#endif

#ifdef UNIX
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

typedef struct essential_s
{
#ifdef USE_STATS
  double lastsave;
#endif
#ifdef USE_GENERATE
  double nextoutput; //next out update time
  double nextnetgen;
#endif
  //double starttime;
  int delay;
  int state;
  int silent;
  int sockwait;
#ifdef USE_QUERY
  int leetify;
#endif
#ifdef USE_STATS
  int outtx;
  int outrx;
#endif
#ifdef USE_GENERATE
  int outrate;
#endif
#ifdef WIN32
  int runservice;
#endif
  char *wdir;
#ifdef USE_GENERATE
  char *outurl; //url
  char *outpath; //html output file
  char *outext; //extension
#endif
} essential_t;

#ifdef USE_STATS
typedef struct netstats_s
{
  int triggered;
  int tcptx;
  int tcprx;
  int udptx;
  int udprx;
} netstats_t;
#endif

typedef struct auth_s
{
  int registered;
  int authed;
  int flags;
  char authserv[MAX_NICKNAME + 1 + MAX_ADDRESS + 1];
  char authline[MAX_AUTHLINE+1];
  char authreply[MAX_AUTHREPLY+1];
} auth_t;

typedef struct network_s
{
  int state;
  int sock;
  struct server_s *servers; //all servers in list
  struct server_s *connect; //server i'm connected to
  struct sockaddr_in connectaddr; //real server i'm connected to
  char group[MAX_GROUP+1]; //networks name
  char grp[MAX_SHORTGROUP+1];
  char hostname[MAX_HOSTNAME+1];
  struct channel_s *channels;
  struct user_s *users;
  char nick[MAX_NICKNAME+1]; //main nick
  char mynick[MAX_NICKNAME+1]; //guess
  char ident[MAX_IDENT+1];
  char fullname[MAX_FULLNAME+1];
  int retake; //retake nick ?
  double lastretake;
  double lastconnect;
  double lastkeepalive;
  double lastsend;
#ifdef USE_STATS
  struct netstats_s stats;
#endif
  int buffertransfer;
  int transfer;
  int lines;
  char *split; //parse
  char *block;
  struct auth_s auth;
  struct comrade_s *comrades;
  struct floodcheck_s *flood;
  struct buffer_s *buffers;
#ifdef USE_LOGGING
  FILE *log;
#endif
  struct network_s *next;
} network_t;

typedef struct server_s
{
  //struct sockaddr_in addr;
  char addr[MAX_SERVER_ADDR+1];
  char pass[MAX_PASSWORD+1];
  int pos;
  int port;
  struct server_s *next;
} server_t;

typedef struct user_s
{
  char nick[MAX_NICKNAME+1];
  char ident[MAX_IDENT+1];
  char address[MAX_ADDRESS+1];
  int flags;
  struct comrade_s *comrade;
  struct user_s *prev;
  struct user_s *next;
} user_t;

typedef struct channel_s
{
  char name[MAX_CHANNEL+1];
  char topic[MAX_TOPIC+1];
  char topicsetby[MAX_NICKNAME+1];
  char modes[MAX_MODE+1];
  char defaulttopic[MAX_DEFAULTTOPIC+1]; //default topic
  char defaultmodes[MAX_DEFAULTMODE+1]; //modes we always set
  char key[MAX_KEY+1];
  double lastjoin;
  FILE *log;
  struct nick_s *nicks; //users on the channel
  struct link_s *link;
  //...
  struct channel_s *prev;
  struct channel_s *next;
} channel_t;

typedef struct nick_s
{
  struct user_s *user; //pointer to user details
  int flags;
  struct nick_s *prev;
  struct nick_s *next;
} nick_t;

typedef struct buffer_s
{
  char *data;
  int len;
  int lines;
  struct buffer_s *next;
} buffer_t;

typedef struct comrade_s
{
  char mask[MAX_ADDRESS+1];
  int access; //triggers i can modify
  int flags;
  struct comrade_s *next;
} comrade_t;

#ifdef USE_STATS
typedef struct mergestats_s
{
  char network[MAX_GROUP+1];
  unsigned long triggered;
  double tcptx;
  double tcprx;
  double udptx;
  double udprx;
} mergestats_t;
#endif

typedef struct link_s
{
  struct network_s *network;
  struct channel_s *channel;
} link_t;

typedef struct floodcheck_s
{
  char *name;
  int warned;
  int count;
  double expires;
  struct floodcheck_s *next;
  struct floodcheck_s *prev;
} floodcheck_t;

typedef struct vital_s
{
  int pos;
  int sz;
} vital_t;

#ifdef USE_SERVICE
int CommandLine(int argc, char **argv);
#ifdef _DEBUG
void SvcDebug(char *s, int r);
#endif
#endif

int Begin(void);
int Init(void);
void Finish(void);
void Frame(void);
void Shutdown(void);

void JoinAttempt(void);
void KeepAlive(void);
void SaveStatistics(int now);
void NickRetake(void);
void JoinChannels(void);
void Register(void);

char *BaseDir(char *file);
int MainParse(char p[][MAX_PARAMSIZE+1], void *data);
int StatsParse(char p[][MAX_PARAMSIZE+1], void *data);

void AddNick(char *channel, char *names, int multi);
void AddNetwork(char p[][MAX_PARAMSIZE+1]);
void AddNetServer(char p[][MAX_PARAMSIZE+1]);
void AddChannel(char p[][MAX_PARAMSIZE+1]);
void AddComrade(char p[][MAX_PARAMSIZE+1]);
void AddLink(char p[][MAX_PARAMSIZE+1]);
//void AddUserHost(char *info);
void AddWhoReply(char *channel, char *name, char *ident, char *address, char *info);
void AddServer(network_t *net, char *ip, char *port, char *pass);
void AddBuffer(network_t *net, char *data, int sz);
void AddFloodCheck(network_t *net, char *from);
void SetUser(char p[][MAX_PARAMSIZE+1]);
void SetOutput(char p[][MAX_PARAMSIZE+1]);
void SetAuth(char p[][MAX_PARAMSIZE+1]);

int ServerConnect(void);
void ServerDisconnect(char *msg);
void ServerNotice(char *host, char *who, char *msg);
void ServerQuit(int r);

#ifdef USE_STATS
void InitMergeStats(mergestats_t *stats, char *group);
#endif

#ifdef USE_GENERATE
void QueryGenerateServers(void);
void GenerateNetworkPage(void);
char *ReadTemplate(char *tpl);
void AddValue(hashtable_t *hash, char *name, char *val, int html);
void ProcessTemplate(char **section, vital_t *vital, char *tpl, hashtable_t *values);
void ClearContent(char **content, vital_t *vital);
#endif

essential_t *NewEssential(void);
user_t *NewUser(char *nick);
nick_t *NewNick(channel_t *chan, user_t *user);
floodcheck_t *NewFloodCheck(network_t *net);

void RemoveUser(char *name);
void RemoveNick(channel_t *chan, char *name);
void RemoveFrom(channel_t *where, char *nick);
void RemoveLists(void);
void RemoveUsers(network_t *net);
void RemoveNicks(channel_t *chan);
void RemoveBuffers(network_t *net);
void RemoveComrades(network_t *net);
void RemoveFloodCheck(network_t *net, floodcheck_t *flood);
void RemoveFloodChecks(network_t *net);
void RemoveNetworks(void);

void NickChange(char *host, char *newnick);
void NickSaid(char *host, char *where, char *msg);
void NickJoined(char *channel, char *host);
void NickParted(char *channel, char *host, char *msg);
void NickKick(char *host, char *channel, char *knick, char *reason);
void NickKilled(char *host, char *who, char *msg);
void NickQuit(char *host, char *msg);
void NickMode(char *p[], int pr);
void NickTopic(char *host, char *channel, char *newtopic);

void CheckBanned(char *where, char *mask);
void SetChannelDefaults(char *channel, char *modes);
void SetTopic(channel_t *chan, char *topic, char *setby);
void TimedJoin(channel_t *chan, char *msg, int time);
void GenerateNetworkPage(void);
void ChannelMode(channel_t *chan, char *mode);
void StripSelfOpFlag(char *channel, char *msg);
void TriggerCheck(char *channel, char *nick, char *msg);
void TriggerMatch(struct sockaddr_in *addr, char *rv, int sz);
int FloodCheck(network_t *net, char *name, char *nick);
void ExpiredFloodCheck(void);
void SendAuth(void);
void AuthNetwork(char *nick, char *host, char *msg);

FILE *LogFile(char *name);

int SendNetSocket(network_t *net, const char *data, int sz);
void Parse(char *rv, int sz);

int AdminValidate(char *nick, int flags);
void AdminBorg(char *name, char *from);
void AdminVersion(char *from);
void AdminUptime(char *from);
void AdminRestart(char *name);
void AdminNetStats(char *name, char *from);
void AdminBF2Status(char *name, char *from, char *group);

int isop(channel_t *chan, char *name);
void nickprefix(nick_t *nick, char *prefix);
user_t *finduser(network_t *net, char *nick);
nick_t *findnick(channel_t *chan, char *name);
channel_t *findchannel(network_t *net, char *name);
network_t *findnetwork(char *name);
comrade_t *findcomrade(network_t *net, char *address);
floodcheck_t *findfloodcheck(network_t *net, char *from);

int Say(int type, char *where, char *msg);
void Forward(char *data);
void FlushBuffers(void);

char *undil(char **p, int start, int pr);
void echo(char *name, int flags, char *s);
void linkmsg(channel_t *chan, char *str);
void append(char **src, int *srcpos, int *srcmax, char *dest, int destlen);

int SessionStart(FILE **fp, char *prefix, char *group, char *name, time_t *long_time, struct tm *t);
int SessionClose(FILE **fp, time_t *long_time);

void modifymode(channel_t *chan, char action, char mode, char *param);
void splithost(char *host, char **nick, char **ident, char **address);
void splitnotice(char *to, int sz, char **msg);

void timestamp(char *stamp, int sz);

typedef int (*config_parser)(char p[][MAX_PARAMSIZE+1], void *data);

extern int Begin(void);
extern void Shutdown(void);

void LoadConfig(char *file, char dil, void *func, void *data);

char *ConsoleInput(void);
double DoubleTime(void);

#ifdef WIN32
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceControl(DWORD request);
void StopService(int code);
int CommandLine(int argc, char **argv);
void InstallService(void);
void UninstallService(void);
#endif

#ifdef UNIX
void HandleSignal(int signal);
#endif

#ifdef _DEBUG
void dump(char *s, int sz);
void debug(char *s);
void FloodMe(void);
#endif

#ifdef SUPERDEBUG
void superdebug(char *msg);
//void superdebug2(char *msg);
char *superdebuglogname(void);
#endif

#endif

