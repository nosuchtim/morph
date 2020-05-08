/*
 TUIO Demo - part of the reacTIVision project
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

#ifndef INCLUDED_Morph_H
#define INCLUDED_Morph_H

#include "NosuchUtil.h"
#include "TuioServer.h"
#include "TuioUdpServer.h"
#include "TuioCursor.h"
#include <list>
#include <map>
#include <vector>
#include <math.h>
#include "sensel.h"
#include "sensel_protocol.h"

extern int Alive_update_interval;

using namespace TUIO;

#define SIDMULTIPLIER 1000

#define MORPH_MAX_FORCE 1000.0f

#define MAX_IGESTURE_ID 12

class MorphArea {
public:
	MorphArea(TuioServer* server_, float x0_, float y0_, float x1_, float y1_) : server(server_), x0(x0_), y0(y0_), x1(x1_), y1(y1_) {
	}
	float x0;
	float y0;
	float x1;
	float y1;
	TuioServer* server;
};

class OneMorph {
public:
	OneMorph(SENSEL_HANDLE h, unsigned char* serial, unsigned char* sids) : _handle(h), _serialnum(serial) {
		std::vector<std::string> sidspecs;
		sidspecs = NosuchSplitOnString(std::string((char*)sids), ";", true);
		int nsidspecs = sidspecs.size();
		for (int n = 0; n < nsidspecs; n++) {
			int sidinit;
			int port;
			std::string host;
			float x0, y0, x1, y1;

			if (!parseSidSpec(sidspecs[n], sidinit, port, host, x0, y0, x1, y1)) {
				fprintf(stdout, "Unable to parse Sid Spec: %s\n", sidspecs[n].c_str());
				continue;
			}

			TuioServer* server = TuioServer::findServer(host, port);
			if (!server) {
				fprintf(stdout, "new TuioServer on port %d, host %s  sidinit %d\n", port, host.c_str(), sidinit);
				server = new TuioUdpServer(host, port, Alive_update_interval, sidinit);
				TuioServer::addServerToList(server);
			}

			MorphArea* ma = new MorphArea(server, x0, y0, x1, y1);

			initialsids.insert(std::pair<int, MorphArea*>(sidinit, ma));
		}
	}

	bool parseSidSpec(std::string spec, int& sidinit, int& port, std::string& host, float& x0, float& y0, float& x1, float& y1) {

		// Format is {sidinit}[/{port}[@{host}]][={x0},{y0},{x1},{y1}]

		sidinit = 10000;
		port = 3333;
		host = "127.0.0.1";
		x0 = y0 = 0.0;
		x1 = y1 = 1.0;
		sscanf(spec.c_str(), "%d", &sidinit);
		size_t i;
		i = spec.find("=");
		if (i != spec.npos) {
			std::string s = spec.substr(i + 1);
			spec = spec.substr(0, i);
			sscanf(s.c_str(), "%f,%f,%f,%f", &x0, &y0, &x1, &y1);
		}
		i = spec.find("/");
		if (i != spec.npos) {
#define HOST_BUFFSIZE 256
			spec = spec.substr(i);
			char hostbuff[HOST_BUFFSIZE];
			int n = sscanf(spec.c_str(), "/%d@%128s", &port, hostbuff);
			if (n != 2) {
				fprintf(stdout, "Bad format of sid spec!?\n");
				return false;
			}
			host = std::string(hostbuff);
		}
		return true;
	}

	int mapToSidArea(float& x, float& y, MorphArea*& area) {
		for (auto& xx : initialsids) {
			MorphArea* ma = xx.second;
			if (x >= ma->x0 && x <= ma->x1 && y >= ma->y0 && y <= ma->y1) {
				// Normalize x and y so that the area
				// is mapped to (0,0),(1,1)
				x = (x - ma->x0) * (1.0f / (ma->x1 - ma->x0));
				y = (y - ma->y0) * (1.0f / (ma->y1 - ma->y0));
				area = ma;
				return xx.first;
			}
		}
		area = NULL;
		return -1;
	}

	MorphArea* areaForSid(int sid) {
		for (auto& xx : initialsids) {
			MorphArea* ma = xx.second;
			int areasid = xx.first;
			if (sid >= areasid && sid < (areasid + SIDMULTIPLIER)) {
				return ma;
			}
		}
		return NULL;
	}

	void update() {
		for (auto& xx : initialsids) {
			MorphArea* ma = xx.second;
			ma->server->update();
		}
	}

	SENSEL_HANDLE _handle;
	std::map<int, MorphArea*> initialsids;
	// int _initialsid;
	unsigned char* _serialnum;
	SenselFrameData* _frame;
	std::map<unsigned char, int> _previousSidFor; // map of cid to sid
	void unsetPreviousSid(unsigned char cid) {
		if (_previousSidFor.find(cid) != _previousSidFor.end()) {
			_previousSidFor.erase(cid);
		}
		else {
			fprintf(stdout, "Hey, unsetePreviousSid called for cid=%d but nothing in _previousSidFor?",cid);
		}
	}
	void setPreviousSid(unsigned char cid, int sid) {
		_previousSidFor[cid] = sid;
	}
	int previousSidFor(unsigned char cid) {
		if (_previousSidFor.find(cid) != _previousSidFor.end()) {
			return _previousSidFor[cid];
		}
		else {
			return -1;
		}
	}

	void pressed(MorphArea* area, float x, float y, int uid, int id, float force);
	void released(MorphArea* area, int uid);
	void dragged(MorphArea* area, float x, float y, int uid, int id, float force);
};

class AllMorphs { 
	
public:
	AllMorphs(SenselDeviceList list, std::map<unsigned char*,unsigned char*> );
	~AllMorphs() {
	};
	
	static void listdevices(SenselDeviceList list);

	void run();
	bool init();
#if 0
	void pressed(float x, float y, int uid, int id, float force);
	void released(int uid);
	void dragged(float x, float y, int uid, int id, float force);
#endif

private:

	std::list<OneMorph*> _morph;

	// int width, height;
};

#endif /* INCLUDED_Morph_H */
