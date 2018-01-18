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

#include <windows.h>
#include <sys/stat.h>
#include <io.h>
#include "../crank.h"

#define NUM_ARGS 1

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  FILE *fp;
	SC_HANDLE ser, scm;
  SERVICE_STATUS st;
  struct _stat buf;
  char *args[NUM_ARGS] = {SWITCH_RUNSERVICE};
  char name[12];

  fp = fopen("borg.log", "wt");
  if (!fp)
    return 1;

  fprintf(fp, "Borg\n");

  if (_stat(SERVICE_UPGRADE_FILE, &buf) == -1)
  {
    fprintf(fp, "Can't find file %s to upgrade\n", SERVICE_UPGRADE_FILE);
    goto _efp;
  }
 
	scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (!scm)
  {
    fprintf(fp, "Failed to access services manager\n");
    goto _efp;
  }

  ser = OpenService(scm, SERVICE_NAME, SERVICE_STOP | SERVICE_START);
  if (!ser)
  {
    fprintf(fp, "Failed to open service: %d\n", GetLastError());
    goto _escm;
  }

  while (1)
  {
    if (!ControlService(ser, SERVICE_CONTROL_STOP, &st))
    {
      if (GetLastError() == ERROR_SERVICE_REQUEST_TIMEOUT)
        continue;
      fprintf(fp, "Failed to control service: %d\n", GetLastError());
      goto _eser;
    }
    else
    {
      break;
    }
  }

  lstrcpy(name, "crank-XXXX");
  _mktemp(name);

  if (rename(SERVICE_EXE_FILE, name))
  {
    fprintf(fp, "Failed to rename %s to %s\n", SERVICE_EXE_FILE, name);
  }
  else
  {
    if (rename(SERVICE_UPGRADE_FILE, SERVICE_EXE_FILE))
      fprintf(fp, "Failed to rename %s to %s\n", SERVICE_UPGRADE_FILE, SERVICE_EXE_FILE);
    else
      fprintf(fp, "Previous saved to %s\n", name);
  }

  if (!StartService(ser, NUM_ARGS, args))
  {
    fprintf(fp, "Failed to start service: %d\n", GetLastError());
  }

_eser:
	CloseServiceHandle(ser);
_escm:
	CloseServiceHandle(scm);
_efp:
  fclose(fp);

  return 1;
}
