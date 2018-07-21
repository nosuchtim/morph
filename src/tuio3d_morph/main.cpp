/*
	TUIO C++ Server Demo - part of the reacTIVision project
	http://reactivision.sourceforge.net/

	Copyright (c) 2005-2009 Martin Kaltenbrunner <mkalten@iua.upf.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "NosuchUtil.h"

#include "TuioServer.h"
#include "TuioSharedMemServer.h"
#include "TuioUdpServer.h"
#include "TuioCursor.h"

#include <stdio.h>

#include <list>
#include <map>

#include "morph.h"

#include "xgetopt.h"
#include "tchar.h"

#include "sensel.h"
#include "sensel_device.h"

using namespace TUIO;

void
printUsage() {
	fprintf(stdout,"usage: tuio3d_* [-v] [-V #] [-a #] [-h port@host] [-l]\n");
	fprintf(stdout,"  -v               Verbosity level = 1\n");
	fprintf(stdout,"  -V #             Verbosity level = #\n");
	fprintf(stdout,"  -a #             Number of milliseconds between alive messages\n");
	fprintf(stdout,"  -h {port}@{host} Port and hostname for TUIO output\n");
	fprintf(stdout,"  -l               List all Morphs and their serial numbers\n");
	fprintf(stdout,"  -s {serial#}:{initialsid}        Serial# and initial session id\n");
	fprintf(stdout,"\n");
}

float ForceFactor = 1.0;
int Verbose = 0;
int Alive_update_interval = 1000; // milliseconds
bool FlipX = false;
bool FlipY = false;

int main(int argc, char **argv)
{
	const char *host = NULL;
	int port = -1;
	int c;
	bool listdevices = false;
	std::map<unsigned char*, unsigned char*> serialmap;
	bool serialmap_filled = false;

	while ((c = getopt(argc, (const char**)argv, "vxyV:a:f:h:i:lLm:p:s:")) != EOF) {
		switch (c) {
		case _T('f'):
			extern float ForceFactor;
			ForceFactor = (float)atof(optarg);
			break;
		case _T('v'):
			Verbose = 1;
			break;
		case _T('x'):
			FlipX = true;
			break;
		case _T('y'):
			FlipY = true;
			break;
		case _T('V'):
			Verbose = atoi(optarg);
			break;
		case _T('a'):
			Alive_update_interval = atoi(optarg);
			break;
		case _T('h'):
			host = optarg;
			break;
#if 0
		case _T('i'):
			sidinitial = atoi(optarg);
			break;
#endif
		case _T('s'):
		{
			unsigned char* serial = (unsigned char*)_strdup((const char*)optarg);
			unsigned char* pcolon = (unsigned char*)strchr((char*)serial, ':');
			if (pcolon == NULL) {
				printf("Invalid value for -s option\n");
			}
			else {
				*pcolon++ = '\0';
				// the format is -s {serial}:{sid} or
				// -s {serial}:{sid}={x0,y0,x1,y1};{sid}={x0,y0,x1,y1}
				serialmap.insert(std::pair<unsigned char*, unsigned char *>(serial, pcolon));
				serialmap_filled = true;
			}
			break;
		}
		case _T('l'):
			listdevices = true;
			break;
		case _T('?'):
			printUsage();
			return 1;
		}
	}

	if (listdevices) {
		AllMorphs::listdevices();
		return 0;
	}

	int nleft = argc - optind;
	if ( nleft != 0 ) {
		printUsage();
		return 1;
	}

	AllMorphs *allmorphs = new AllMorphs(serialmap);

	if (allmorphs->init()) {
		allmorphs->run();
	}
	// run() may not actually return
	delete(allmorphs);

	return 0;
}