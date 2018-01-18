# crank

crank is a friendly IRC bot, it can query game servers and say the results
into a channel. Other features include multi server support, auto-op of
friends, the usual anti-flood, running as a service under windows and idling.

* [Home page](https://www.nisda.net/crank.html)
* [readme.txt](bin/readme.txt)

## Features

* Triggers: Query game servers and message results to a channel.
* Network: Multiple IRC server support.
* Friend: Auto-op of friends and other commands.
* Flood: Message flood protection.
* Auth: Authenticate crank to the network's services.
* Link: Sends all messages and actions (such as join/part/quits) from one channel to the other, even over networks.
* Chat: ~~Ability to message Battlefield 2/2142 server chat to a channel (uses rcon protocol).~~
* Service: Remote updating of crank if run as a windows service.

## Releases

Version|Date|Download
---|---|---
v1.0|10th March 2005|
v1.546|30th July 2007|[win32 i386](https://www.nisda.net/files/crank-v1.546.zip)

## Source

The source code was released on 28th January 2008.

### Windows

Compile with Microsoft Visual Studio 6.

### Linux

Compile with GCC using the ``Makefile`` in the [src/](src/) directory.

```bash
  cd src
  make                  # Build debug version
  make BUILD=release    # Build release version
```

Thanks to [icepik-au](https://github.com/icepik-au) for patching the Makefile to work in this decade.

### Third Party Software

Name|Description
---|---
[qstat](https://github.com/multiplay/qstat)|Best software for querying game servers.
[MemLeakCheck](http://tunesmithy.co.uk/memleakcheck/index.htm)|To stop comparing this program to a sieve.
[strlcpy/strlcat](http://www.gratisoft.us/todd/papers/strlcpy.html)|There's probably a buffer overflow in there somewhere.
