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

#ifndef _H_QUERY
#define _H_QUERY

#define QUERY_HELP_USAGE "Usage: !#<channel> | !<trigger> [<modifiers>] | !query <protocol>[<modifiers>] <server> [<server2> [...]]"
#define QUERY_HELP_MODIFIERS "Modifiers: +/- show/hide players, >/< show/hide servers with no players or didn't respond, ~/$ show/hide statistics footer, @ private message reply, *<match>* only show players which match"
#define QUERY_HELP_INFO "For more information: http://www.nisda.net/crank.php"

//having all here, show overrides hide
#define QUERYFLAG_SHOWPLAYERS 1
#define QUERYFLAG_HIDEPLAYERS 2
#define QUERYFLAG_SHOWIFNOPLAYERS 4
#define QUERYFLAG_HIDEIFNOPLAYERS 8
#define QUERYFLAG_SHOWSTATS 16
#define QUERYFLAG_HIDESTATS 32
#define QUERYFLAG_QUERYNICK 64
#define QUERYFLAG_WILDMATCH 128
#define QUERYFLAG_SHOWONLYTIMEOUT 256
#define QUERYFLAG_SHOWQUERYPORTS 512
//#define QUERYFLAG_HIDEIFNOWILDMATCH 512
#define QUERYFLAG_MATCHCAST 1024
#define QUERYFLAG_GENERATEPAGE 2048

#define QUERYFLAG_DEFAULT (QUERYFLAG_SHOWPLAYERS | QUERYFLAG_SHOWIFNOPLAYERS)

//don't use : as a symbol because it messes with config parser
#define QUERYSYMBOL_HIDEPLAYERS '-'
#define QUERYSYMBOL_SHOWPLAYERS '+'
#define QUERYSYMBOL_HIDEIFNOPLAYERS '<'
#define QUERYSYMBOL_SHOWIFNOPLAYERS '>'
#define QUERYSYMBOL_HIDESTATS '$'
#define QUERYSYMBOL_SHOWSTATS '~'
#define QUERYSYMBOL_QUERYNICK '@'
#define QUERYSYMBOL_WILDMATCH '*'
#define QUERYSYMBOL_SHOWONLYTIMEOUT '.'
#define QUERYSYMBOL_SHOWQUERYPORTS ';'
#define QUERYSYMBOL_GENERATEPAGE '%'
//#define QUERYSYMBOL_HIDEIFNOWILDMATCH '&'

#define GAME_FLAG_PASSWORD 1
#define GAME_FLAG_RANKED 2
#define GAME_FLAG_OS_UNIX 4
#define GAME_FLAG_OS_WIN32 8

#define MAX_GAME_HOSTNAME 96
#define MAX_GAME_MAP 32
#define MAX_GAME_STATUS 16
#define MAX_GAME_PLAYER_NAME 32
#define MAX_GAME_NUMPLAYERS 4
#define MAX_GAME_MAXPLAYERS 4
#define MAX_GAME_PLAYER_FRAGS 4
#define MAX_GAME_TYPE 16
#define MAX_GAME_MOD 16
#define MAX_GAME_VERSION 16
#define MAX_GAME_PID 8

#define MAX_GAME_UID 16
#define MAX_GAME_TAG 6

#define MAX_PROTOCOL_NAME 12

#define MAX_TRIGGER_NAME 12
#define MAX_RETRY_TIME 1.0
#define MAX_TIMEOUT 4.0
#define MAX_THREADWAIT 60000

#define MAX_MAP_DESCRIPTION 32
#define MAX_MAP_SIZE 12

#define NAN_INT -32767

#define MAX_HASH_SIZE 64
#define MAX_HIGH_SCORE 3
#define MAX_TEAM_NAME 32

#define MAX_PLAYERS 64

#define MAX_LINEOUT 8

typedef struct trigger_s
{
  char name[MAX_TRIGGER_NAME+1];
  int id;
  int flags;
  int custom;
  struct queryserver_s *servers;
  struct querychannel_s *channels;
  struct trigger_s *next;
  struct trigger_s *prev;
} trigger_t;

typedef struct queryserver_s
{
  struct gameserver_s *server;
  struct queryserver_s *next;
  struct queryserver_s *prev;
} queryserver_t;

typedef struct querychannel_s
{
  struct channel_s *channel;
  struct querychannel_s *next;
  struct querychannel_s *prev;
} querychannel_t;

typedef struct gameserver_s
{
  char addr[MAX_ADDRESS+1];
  int port;
  char game[MAX_GAME_TAG+1];
  int custom;
  struct gameserver_s *next;
  struct gameserver_s *prev;
} gameserver_t;

typedef struct serverinfo_s
{
  char hostname[MAX_GAME_HOSTNAME+1];
  char map[MAX_GAME_MAP+1];
  char type[MAX_GAME_TYPE+1]; //conquest
  char mod[MAX_GAME_MOD+1]; //dc_final
  char status[MAX_GAME_STATUS+1];
  char version[MAX_GAME_VERSION+1];
  char team1[MAX_TEAM_NAME+1];
  char team2[MAX_TEAM_NAME+1];
  char ip[MAX_IP+1];
  int queryport;
  int gameport;
  int numplayers;
  int maxplayers;
  int tickets1;
  int tickets2;
  int flags;
  struct player_s *players[MAX_PLAYERS];
} serverinfo_t;

typedef struct player_s
{
  char name[MAX_GAME_PLAYER_NAME+1];
  char displayname[MAX_GAME_PLAYER_NAME+6]; //X00<name>X
  char pid[MAX_GAME_PID+1];
  char team[MAX_TEAM_NAME+1];
  int score;
  //int frags;
  int deaths;
  int id;
  int ping;
  int displayed;
  struct player_s *next;
  struct player_s *prev;
} player_t;

typedef struct rules_s
{
  int roundtime;
  int roundremain;
  int timelimit;
  int startdelay;
} rules_t;

typedef struct change_s
{
  struct player_s *player;
  struct change_s *prev;
  struct change_s *next;
} change_t;

typedef struct activequery_s
{
  struct trigger_s *trigger;
  char uid[MAX_GAME_UID+1];
  char *from;
  char *wildmatch;
  //double timeout;
  //double retrytime;
  int flags;
  //int replys;
  int numservers;
  int totalservers;
  int numplayers;
  int totalplayers;
  int matched;
  int lineout;
  int done;
  int id;
  int errorno;
#ifdef USE_GENERATE
  int generate;
#endif
#ifdef UNIX
  pthread_t thread;
#endif
#ifdef WIN32
  HANDLE thread;
#endif
  _mutex mutex;
  struct result_s *results;
  struct result_s *lastresult;
  struct network_s *network;
  struct activequery_s *next;
  struct activequery_s *prev;
} activequery_t;

typedef struct result_s
{
  int up;
  int matched;
  int processed;
  int playerix;
  //int sentretry;
  struct serverinfo_s info;
  struct packet_s *packets;
  struct result_s *next;
} result_t;

typedef int (*game_parser)(result_t *res, serverinfo_t *info);
typedef int (*game_final)(result_t *res);

typedef struct protocol_s
{
  char name[MAX_PROTOCOL_NAME+1];
  int len;
  struct protocol_s *next;
} protocol_t;

typedef struct game_s
{
  char *desc;
  char *id;
  int port;
  char *query;
  int qlen;
  int hlen;
  game_parser parser;
  game_final final;
} game_t;

typedef struct mapdesc_s
{
  char name[MAX_MAP_DESCRIPTION+1];
  char size[MAX_MAP_SIZE+1];
  int match;
} mapdesc_t;

void TriggerCheck(char *channel, char *nick, char *msg);
void TriggerMatch(struct sockaddr_in *addr, char *rv, int sz);

int GameTriggerCheck(char *text, char *game, char *from);
void ShowHelp(char *nick);
void ShowQueries(void);
void ShowTriggers(char *channel, char *from);

int QueryParse(char p[][MAX_PARAMSIZE+1], void *data);
#ifdef USE_GENERATE
void QueryGameServer(activequery_t **queries, trigger_t *trig, char *from, char *nick, char *flags, int gen);
#else
void QueryGameServer(activequery_t **queries, trigger_t *trig, char *from, char *nick, char *flags);
#endif

#ifdef USE_GENERATE
int GeneratePage(activequery_t *query);
#endif

void ProcessQueries(void);
void *RunQuery(__threadvoid *data);
void ShowServerInfo(result_t *res, activequery_t *query);
void ShowFooter(activequery_t *query);
void MapDescription(mapdesc_t *desc, char *mapname);
int ProtocolParse(char p[][MAX_PARAMSIZE+1], void *data);
int ResultParse(char p[][MAX_PARAMSIZE+1], void *data);
int DescriptionParse(char p[][MAX_PARAMSIZE+1], void *data);
protocol_t *FindGame(char *id);

protocol_t *NewProtocol(protocol_t **prots);
void RemoveProtocols(protocol_t **prots);

void AddTrigger(char p[][MAX_PARAMSIZE+1]);
void AddGameServer(char p[][MAX_PARAMSIZE+1]);
void AddGameChannel(char p[][MAX_PARAMSIZE+1]);
void AddChange(change_t **parent, player_t *player);

int AdminValidateTrigger(char *nick, trigger_t *trig);

void AdminSaveQuery(char *name);
void AdminLoadQuery(char *name);
void AdminAddServer(char *name, char *trigger, char *list);
void AdminRemoveServer(char *name, char *trigger, char *list);
void AdminAddChannel(char *name, char *trigger, char *channel);
void AdminRemoveChannel(char *name, char *trigger, char *channel);
void AdminMatchCast(char *nick, char *from, char *channel, char *address);

trigger_t *NewTrigger(void);
gameserver_t *NewGameServer(void);
queryserver_t *NewQueryServer(trigger_t *trig);
querychannel_t *NewQueryChannel(trigger_t *trig);
trigger_t *NewCustomTrigger(char *from, char *nick, char *p, char *list);
player_t *NewPlayer(void);
activequery_t *NewActiveQuery(activequery_t **parent);
result_t *NewResult(result_t **parent);
change_t *NewChange(change_t **parent);

void InitServerInfo(serverinfo_t *info);

void RemoveQueryServer(trigger_t *trig, queryserver_t *qserv);
void RemoveQueryServers(trigger_t *trig);
void RemoveQueryChannel(trigger_t *trig, querychannel_t *qchan);
void RemoveQueryChannels(trigger_t *trig);
void RemoveTrigger(trigger_t *trig);
void RemoveTriggers(void);
void RemoveGameServer(gameserver_t *gserv);
void RemoveGameServers(void);
void RemovePlayers(serverinfo_t *info);
void RemoveActiveQuery(activequery_t **parent, activequery_t *query);
void RemoveActiveQueries(activequery_t **parent);
void RemoveUnusedGameServer(gameserver_t *gserv);
void RemoveResults(result_t **parent);
void RemoveResult(result_t **parent);
void RemovePackets(result_t *res);
void RemoveChanges(change_t **parent);
void RemoveChange(change_t **parent, change_t *change);

player_t *findplayer(serverinfo_t *d, char *name);
trigger_t *findtrigger(char *name);
queryserver_t *findqueryserver(trigger_t *trig, char *ip, int port);
gameserver_t *findgameserver(char *ip, int port);
querychannel_t *findquerychannel(trigger_t *trig, char *name);

trigger_t *allowedtrigger(char *name, char *from);
int countservers(trigger_t *trig);
int istimeout(result_t *results);

void leetifyname(char *s);
void obfuscate(char *name);
int modifiers(char *s);
int mergemodifiers(int a, int b);
void cleanname(char *name);
void writehtmlchars(FILE **fp, char *str);

#ifdef _DEBUG
void playerout(serverinfo_t *info);
void showflags(int f);
void GameOut(void);
#endif

#endif

