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

#include <nats.h>

#include "NosuchUtil.h"

#include "TuioServer.h"
#include "TuioSharedMemServer.h"
#include "TuioUdpServer.h"
#include "TuioCursor.h"

#include <stdio.h>
#include <string.h>

#include <list>
#include <map>

#include "morph.h"

#include "xgetopt.h"
// #include "tchar.h"

#include "sensel.h"
#include "sensel_device.h"

natsConnection* NatsConn = NULL;

using namespace TUIO;

void
printUsage() {
	fprintf(stdout,"usage: tuio3d_* [-v] [-V #] [-a #] [-h port@host] [-l]\n");
	fprintf(stdout,"  -v               Verbosity level = 1\n");
	fprintf(stdout,"  -V #             Verbosity level = #\n");
	fprintf(stdout,"  -a #             Number of milliseconds between alive messages\n");
	fprintf(stdout,"  -f #             force scale factor\n");
	fprintf(stdout,"  -h {port}@{host} Port and hostname for TUIO output\n");
	fprintf(stdout,"  -n {host}        Hostname for NATS output\n");
	fprintf(stdout,"  -l               List all Morphs and their serial numbers\n");
	fprintf(stdout,"  -s {serial#}:{initialsid}        Serial# and initial session id\n");
	fprintf(stdout,"\n");
}

extern "C" {
float ForceFactor = 1.0;
}

int Verbose = 0;
int Alive_update_interval = 1000; // milliseconds
bool FlipX = false;
bool FlipY = false;

int main(int argc, char **argv)
{
	const char* nats = NULL;
	const char *host = NULL;
	int port = -1;
	int c;
	bool listdevices = false;
	std::map<unsigned char*, unsigned char*> serialmap;
	bool serialmap_filled = false;

	while ((c = MYgetopt(argc, (char**)argv, "vxyV:a:f:h:i:lLm:n:p:s:")) != EOF) {
		switch (c) {
		case ('f'):
			ForceFactor = (float)atof(optarg);
			break;
		case ('v'):
			Verbose = 1;
			break;
		case ('x'):
			FlipX = true;
			break;
		case ('y'):
			FlipY = true;
			break;
		case ('V'):
			Verbose = atoi(optarg);
			break;
		case ('a'):
			Alive_update_interval = atoi(optarg);
			break;
		case ('n'):
			nats = optarg;
			break;
		case ('h'):
			host = optarg;
			break;
#if 0
		case ('i'):
			sidinitial = atoi(optarg);
			break;
#endif
		case ('s'):
		{
			unsigned char* serial = (unsigned char*)strdup((const char*)optarg);
			unsigned char* pcolon = (unsigned char*)strchr((char*)serial, ':');
			if (pcolon == NULL) {
				printf("Invalid value for -s option\n");
			}
			else {
				*pcolon++ = '\0';
				// the format is one of these
				// -s {serial}:{sid}
				// -s {serial}:{sid}={x0,y0,x1,y1};{sid}={x0,y0,x1,y1}
				// -s {serial}:{sid}={x0,y0,x1,y1};{sid}={x0,y0,x1,y1}
				serialmap.insert(std::pair<unsigned char*, unsigned char *>(serial, pcolon));
				serialmap_filled = true;
			}
			break;
		}
		case ('l'):
			listdevices = true;
			break;
		case ('?'):
			printUsage();
			return 1;
		}
	}

	SenselDeviceList list;

	senselGetDeviceList(&list);
	if (list.num_devices == 0) {
		fprintf(stdout, "No Sensel devices found!\n");
		return 1;
	}

	if (listdevices) {
		AllMorphs::listdevices(list);
		return 0;
	}

	if (nats != NULL) {

		natsStatus s;


		// Creates a connection to the default NATS URL
		s = natsConnection_ConnectTo(&NatsConn, nats);
		if (s == NATS_OK) {
			// This is a convenient function to send a message on "foo"
			// as a string.
			printf("Publishing a morph message on NATS\n");
			s = natsConnection_PublishString(NatsConn, "morph", "OINK");
		}
		else {
			printf("Unable to connect to Nats server\n");
			return 1;
		}
	}

	int nleft = argc - optind;
	if ( nleft != 0 || serialmap_filled == false ) {
		printUsage();
		return 1;
	}

	AllMorphs *allmorphs = new AllMorphs(list, serialmap);

	if (allmorphs->init()) {
		allmorphs->run();
	}
	// run() may not actually return
	delete(allmorphs);

	return 0;
}
