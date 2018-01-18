@echo off
rem Used by crank to query servers
rem Resist the urge to modify
cd query
qstat.exe -showgameport -sort nh -f %1.in -of %1.out -P -R -Ts server.tpl -Tp player.tpl -Tr rule.tpl
