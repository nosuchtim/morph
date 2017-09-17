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
#include "TuioDevice.h"

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
	fprintf(stdout,"usage: tuio3d_* [-v] [-V #] [-a #] [-h port@host] [-l] [-m name]\n");
	fprintf(stdout,"  -v               Verbosity level = 1\n");
	fprintf(stdout,"  -V #             Verbosity level = #\n");
	fprintf(stdout,"  -a #             Number of milliseconds between alive messages\n");
	fprintf(stdout,"  -h {port}@{host} Port and hostname for TUIO output\n");
	fprintf(stdout,"  -l               List all Morphs and their serial numbers\n");
	fprintf(stdout,"  -m {name}        Shared memory name\n");
	fprintf(stdout,"  -s {serial#}:{initialsid}        Serial# and initial session id\n");
	fprintf(stdout,"\n");
}

float ForceFactor = 1.0;
bool UseLEDs = false;

int main(int argc, char **argv)
{
	const char *host = NULL;
	const char *memname = NULL;
	int port = -1;
	int verbose = 0;
	int c;
	int alive_update_interval = 1000; // milliseconds
	bool flipx = false;
	bool flipy = false;
	bool listdevices = false;
	std::map<unsigned char*, int> serialmap;
	bool serialmap_filled = false;
	int sidinitial = 10000;
	int sidincrement = 1000;  // if there are multiple morphs

	while ((c = getopt(argc, (const char**)argv, "vxyV:a:f:h:i:lLm:p:s:")) != EOF) {
		printf("Arg = %s\n",optarg);
		switch (c) {
		case _T('f'):
			extern float ForceFactor;
			ForceFactor = (float)atof(optarg);
			break;
		case _T('v'):
			verbose = 1;
			break;
		case _T('x'):
			flipx = true;
			break;
		case _T('y'):
			flipy = true;
			break;
		case _T('V'):
			verbose = atoi(optarg);
			break;
		case _T('a'):
			alive_update_interval = atoi(optarg);
			break;
		case _T('h'):
			host = optarg;
			break;
		case _T('i'):
			sidinitial = atoi(optarg);
			break;
		case _T('s'):
		{
			unsigned char* arg = (unsigned char*)_strdup((const char*)optarg);
			unsigned char* p = (unsigned char*)strchr((char*)arg, ':');
			if (p == NULL) {
				printf("Invalid value for -s option\n");
			}
			else {
				*p++ = '\0';
				serialmap.insert(std::pair<unsigned char*, int>(arg, atoi((const char*)p)));
				serialmap_filled = true;
			}
			break;
		}
		case _T('m'):
			memname = optarg;
			break;
		case _T('l'):
			listdevices = true;
			break;
		case _T('L'):
			UseLEDs = true;
			break;
		case _T('?'):
			printUsage();
			return 1;
		}
	}

	if (listdevices) {
		Morph::listdevices();
		return 0;
	}

	int nleft = argc - optind;
	// fprintf(stdout,"nleft = " << nleft << " verbose = " << verbose << "\n");
	if ( nleft != 0 ) {
		printUsage();
		return 1;
	}

	if (memname != NULL && host != NULL) {
		fprintf(stdout,"You can't do both shared-memory and TUIO at the same time!\n");
		return 1;
	}

	if (memname == NULL && host == NULL) {
		host = "3333@127.0.0.1";
	}

	TuioServer* server = NULL;

	if (host) {
		const char* amp = strchr(host, '@');
		if (amp) {
			port = atoi(host);
			host = amp + 1;
		} else {
			port = 3333;
		}
		fprintf(stdout, "Sending TUIO output to port %d of host %s\n", port, host);
		server = new TuioUdpServer(host, port, alive_update_interval);
	}
	else if (memname) {
		server = new TuioSharedMemServer(memname);
	}

	server->flipX = flipx;
	server->flipY = flipy;
	server->sidInitial = sidinitial;
	server->sidDeviceMultiplier = sidincrement;
	server->verbose = verbose;

	TuioDevice *device = new Morph(server, serialmap);

	if (device->init()) {
		device->run();
	}
	// run() may not actually return
	delete(device);
	delete(server);

	return 0;
}