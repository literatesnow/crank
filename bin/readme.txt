crank v1.545 - Copyright (C) 2005-2007 bliP
Web: http://nisda.net
Email: spawn [at] nisda [dot] net
Compiled: v1.0: 2005-03-10, v1.545: 2007-07-22

crank is a friendly IRC bot, it can query game servers and say the results
into a channel. Other features include multi server support, auto-op of
friends, the usual anti-flood, running as a service under windows and idling.


Installation

  - Extract the archive with full paths
  - Edit these configuration files
    + crank.conf - main configuration
    + query.conf - query configuration
  - Start crank (either in a console or as a service)
    + Under windows, if the window briefly appears then disappears, run from
      the command prompt to see the error message.


Design

  After crank has been started, nearly all of the settings (aside from query
  servers) cannot be changed, unless the configuration is updated and crank
  is restarted. Because of this, crank cannot randomly join/part channels,
  change modes, kick/ban people or even say anything that isn't a trigger
  response.


Configuration

  These files are plain text, each item is delimited by a ":" and separated
  by a new line. Comments are allowed with "//" at the immediate start of the
  line. The character ":" cannot be escaped. Some items are optional and can
  be omitted.


Main configuration: crank.conf

  - Network: Specifies a network. Multiple networks are supported.
    + network:<name>:<short name>

      network:kaboom:KA

  - Server: Adds a server to a network, more than one server can be used, both
    ip and host name.
    + server:<network>:<ip/host>:<port>

      server:kaboom:irc.example.com:6668
      server:kaboom:10.10.1.1:6667

  - User: crank's nick, user name and real name for this network
    + user:<network>:<nick>:<user name>:<real name>

      user:kaboom:crank__:bot:www.nisda.net/
    
  - Channel: The channels to join on this network. If crank has ops, key,
    modes and topic will be set.
    + <#channel>:<network>:<key>:<default modes>:<default topic>
    + <#channel>:<network>

      #test:kaboom
      #query:kaboom::+nt:Type !crank

  - Friend: Users with special permissions. Flags is a string of characters.
    * a = auto op this user
    * s = add/remove/save query servers
    * c = command - all of 's', no flood check, restart, load/save servers
    + friend:<network>:<host mask>:<flags>

      friend:kaboom:*!*user@127.0.0.1:sca
      friend:kaboom:*!*bob@*.dialin.example.com:a

  - Link: Sends all messages and actions (such as join/part/quits) from one
    channel to the other, even over networks. Messages are sent both ways.
    + link:<network 1>:<#channel 1>:<network 2>:<#channel 2>

      link:kaboom:#query:explode:#relay

  - Auth: Authenticate crank to the network's services. It's a good idea to use
    nick@hostmask to send the auth to, also the messages must match exactly.
    Flags is a bitmask (see notes).
    * 1 = random nick before connect
    * 2 = don't join channels until authenticated
    * 4 = don't join channels until authenticated and +x'd
    + auth:<network>:<auth bot nick>:<auth message>:<okay auth message>:<flags>

      auth:kaboom:NickServ@Services.kaboom.xyz:AUTH user pass:-- Auth OK --:3
      auth:quakenet:Q@CServe.quakenet.org:AUTH user pass:AUTH'd successfully.:2


Query configuration: query.conf

  - Trigger: Text in a channel to watch for, the trigger will apply to all
    networks. Modifiers are a string of characters and change the way queries
    are handled, some override others. For triggers with large amounts of
    servers, "-<~" may be the best, for a few servers, "+<" or "+<~".
    * - = hide players
    * + = show players
    * < = hide if no players
    * > = show if no players
    * ~ = show stats
    * @ = /query the user who typed the trigger
    + trigger:<name>:<modifiers>

      trigger:quake:+<~
      trigger:qwdm:@
      trigger:qwctf

  - Channel: Trigger is only active in these channels.
    + channel:<name>:<#channel>

      channel:quake:#quake
      channel:quake:#query

  - Server: List of games and servers for the trigger. Protocol corresponds to
    the protocol switch of qstat.
    + server:<name>:<protocol>:<ip/host>:<port>

      server:quake:qws:203.96.92.69:27500
      server:quake:qws:203.96.92.69:27501


Games/Protocols: games.conf

  - This is a list of game types which is allowed to be queried, the protocol
    parameter is from qstat (for a list run qstat --help). Usually this list
    will not include the master server queries. Game name and default port are
    for reference only and not used.
    + game:<protocol>:<name>:<default port>

      game:qws:QuakeWorld:27500
      game:eye:All Seeing Eye:0


Map Descriptions: desc.conf

  - Provides a friendly name for maps.
    + <map name>:<friendly name>:<size in MB>

    2fort5r:Two Forts:0.9
    nmtrees:No more trees:1.5


Triggers

  These can be either typed in a channel or a private message to crank, the
  trigger prefix is the ! character. (The friend flags required are in brackets
  or none if anyone can action the trigger.)

 - <trigger>: Actions a trigger, if flags are specified they will override the
   flags in query.conf.
   + <trigger> [<flags>]

     !quake
     !quake @~

 - query: Query any server. As with the query configuration, protocol
   corresponds to the protocol switch of qstat. Note that there is no space
   between protocol and the modifiers.
   + query <protocol and modifiers> <ip/host[:port]> [[<ip/host[:port]>] ..]

     !query qws 203.96.92.69
     !query qws@<~ 203.96.92.69:27500 quake.example.com

 - crank: Shows brief help on crank
 
     !crank

 - uptime: Displays crank's version, build date, uptime and uptime of the host.

     !uptime

 - restart (c): Acts as if crank has been closed and opened again, used to apply
   recent changes configuration. Does not effect uptime.

     !restart

 - addserver (sc): Adds a server to a trigger, save changes with !savequery.
   + addserver <trigger> <protocol> <ip/host:port> [[<ip/host:port>] ..]

     !addserver quake qws 203.96.92.69:27500 quake.example.com:27505

 - delserver (sc): Remove a server from a trigger, save changes with !savequery.
   + delserver <trigger> <ip/host:port> [[<ip/host:port>] ..]

     !delserver quake quake.example.com:27505 203.96.92.69:27500

 - savequery (sc): Saves all triggers, flags and servers to query.conf.

     !savequery

 - loadquery (c): Reload all triggers from query.conf, existing changes
   will be lost.

     !loadquery

 - borg (c): Used when crank is running as a service under windows, see the
   service section for more details.

     !borg


Service

  Under windows, crank can run as a service.

    To register the service, run: crank.exe --register
    To unregister the service, run: crank.exe --unregister

  Aside from starting when windows boots, another advantage is that crank can be
  remotely updated by a friend with the command flag, this is useful if you only
  have FTP access to crank's directory and wish to use a new version. Rename the
  new version of crank to crank.up and drop the file in the same directory that
  crank.exe is in then send crank the !borg command to initiate the update. How
  it works: crank calls borg, borg stops the service, backs up the old crank,
  renames new crank and restarts the service.


Notes

  - Bitmasks: To use more than one flag at once, add them to together. For
    example flags of 1 (A), 2 (B), 4 (C), to specify A and C use 5, 1+4
    (Also see the wikipedia article for more information).

  - Game support: If qstat does not appear to support a game, chances are that
    the game is supported using another engine or protocol.
    + Call of Duty - Quake 3 (q3s)
    + Battlefield 1942 - All Seeing Eye (eye)
    + Battlefield Vietnam - Gamespy V2 (gs2)
    + Battlefield 2 - Gamespy V3 (gs3)
    + Red Orchestra - Unreal Tournament 2 (ut2s)
    + Enemy Territory - Quake 3 (q3s)
    + Enemy Territory Quake Wars - QuakeWars (etqws)

  - Flood protection: If a trigger is said too many times, crank will ignore
    the user for a period of time. Friend users with the command flag do not
    activate this.

  - Logging: All channel chat, server status text and private messages are
    logged into files in the logs/ directory. This cannot be disabled.

  - Clone scanner: Users with the same host will be highlighted when they join
    the channel.

  - Startup join lag: On startup, if crank is in many channels then it may take
    a while for crank to respond to requests, wait for the backlog to clear.

  - Keep alive: To keep the connection alive and detect a time out, crank will
    periodically send a simple packet.

  - RFC 1459: While crank implements most of the specifications some of the
    newer protocol changes won't be covered, these will appear in the logs
    as [raw] and the entire message.


Please send all questions, bug reports, comments and suggestions
to the email address provided above.

Disclaimer:
ANY USE BY YOU OF THE SOFTWARE IS AT YOUR OWN RISK. THE SOFTWARE IS
PROVIDED FOR USE "AS IS" WITHOUT WARRANTY OF ANY KIND. THIS SOFTWARE
IS RELEASED AS "FREEWARE". REDISTRIBUTION IS ONLY ALLOWED IF THE
SOFTWARE IS UNMODIFIED, NO FEE IS CHARGED FOR THE SOFTWARE, DIRECTLY
OR INDIRECTLY WITHOUT THE EXPRESS PERMISSION OF THE AUTHOR.

