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
#include "morph.h"
#include "xgetopt.h"
#include "tchar.h"

using namespace TUIO;

void
printUsage() {
	std::cout << "usage: tuio3d_* [-v] [-V #] [-a #] [-h host] [-p port] [-m name]\n";
	std::cout << "  -V #         Verbosity level\n";
	std::cout << "  -a #         Number of milliseconds between alive messages\n";
	std::cout << "  -i #         Initial session id\n";
	std::cout << "  -h {host}    Hostname for TUIO output\n";
	std::cout << "  -p #         Port number for TUIO output\n";
	std::cout << "  -m {name}    Shared memory name\n";
}

int main(int argc, const char* argv[])
{
	const char *host = NULL;
	const char *memname = NULL;
	int port = -1;
	int verbose = 0;
	int c;
	int alive_update_interval = 1000; // milliseconds
	int initial_session_id = 11000;   // session id space
	int device_multiplier =   1000;   // in session id's.  I.e. the second device starts at 12000
	bool flipx = false;
	bool flipy = false;

	while ((c = getopt(argc, argv, "vxyV:a:h:i:m:p:")) != EOF) {
		switch (c) {
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
				initial_session_id = atoi(optarg);
				break;
			case _T('m'):
				memname = optarg;
				break;
			case _T('p'):
				port = atoi(optarg);
				break;
			case _T('?'):
				printUsage();
        		return 0;
		}
	}
	int nleft = argc - optind;
	// std::cout << "nleft = " << nleft << " verbose = " << verbose << "\n";
	if ( nleft != 0 ) {
		printUsage();
		return 0;
	}

	if (memname != NULL && host != NULL) {
		NosuchDebug("You can't do both shared-memory and TUIO at the same time!\n");
		return 1;
	}
	if (memname == NULL && host == NULL) {
		std::cout << "Defaulting to TUIO on host 127.0.0.1\n";
		host = "127.0.0.1";
	}

	TuioServer* server = NULL;

	if (host) {
		if (port < 0) {
			port = 3333;
			std::cout << "Assuming port=" << port << "\n";
		}
		server = new TuioUdpServer(host, port, alive_update_interval);
	}
	else if (memname) {
		server = new TuioSharedMemServer(memname);
	}

	server->flipX = flipx;
	server->flipY = flipy;
	server->initial_session_id = initial_session_id;
	server->device_multiplier = device_multiplier;
	server->verbose = verbose;

	TuioDevice *device = new Morph(server);
	if (device->init()) {
		device->run();
	}
	// run() may not actually return
	delete(device);
	delete(server);

	return 0;
}