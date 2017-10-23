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
#include "TuioCursor.h"
#include <list>
#include <map>
#include <vector>
#include <math.h>
#include "TuioDevice.h"
#include "sensel.h"
#include "sensel_protocol.h"

using namespace TUIO;

#define MORPH_MAX_FORCE 1000.0f
#define MORPH_WIDTH 230
#define MORPH_HEIGHT 130
#define MAX_IGESTURE_ID 12

class MorphArea {
public:
	MorphArea(float x0_, float y0_, float x1_, float y1_) : x0(x0_), y0(y0_), x1(x1_), y1(y1_) {
	}
	float x0;
	float y0;
	float x1;
	float y1;
};

class OneMorph {
public:
	OneMorph(SENSEL_HANDLE h, unsigned char* serial, unsigned char* sids) : _handle(h), _serialnum(serial) {
		std::vector<std::string> sidspecs;
		sidspecs = NosuchSplitOnString(std::string((char*)sids), ";", true);
		int nsidspecs = sidspecs.size();
		float x0, y0, x1, y1;
		for (int n = 0; n < nsidspecs; n++) {
			int sidinit;
			const char* sidspec = sidspecs[n].c_str();
			if (strchr(sidspec, '=') == NULL) {
				if (sscanf(sidspec, "%d", &sidinit) != 1) {
					sidinit = -1;
				} else {
					x0 = y0 = 0.0;
					x1 = y1 = 1.0;
				}
			} else if (sscanf(sidspec, "%d=%f,%f,%f,%f", &sidinit, &x0, &y0, &x1, &y1) != 5) {
					sidinit = -1;
			}
			if (sidinit < 0) {
				printf("Invalid value for -s option");
			} else {
				MorphArea* ma = new MorphArea(x0,y0,x1,y1);
				initialsids.insert(std::pair<int, MorphArea*>(sidinit, ma));
			}
		}
	}
	int initialSid(float x, float y) {
		for (auto& xx : initialsids) {
			MorphArea* ma = xx.second;
			if (x >= ma->x0 && x <= ma->x1 && y >= ma->y0 && y <= ma->y1) {
				return xx.first;
			}
		}
		return -1;
	}
	SENSEL_HANDLE _handle;
	std::map<int, MorphArea*> initialsids;
	// int _initialsid;
	unsigned char* _serialnum;
	SenselFrameData* _frame;
};

class Morph : public TuioDevice { 
	
public:
	Morph(TuioServer* s, std::map<unsigned char*,unsigned char*> );
	~Morph() {
	};
	
	static void listdevices();

	void run();
	bool init();
	void pressed(float x, float y, int uid, int id, float force);
	void released(float x, float y, int uid, int id, float force);
	void dragged(float x, float y, int uid, int id, float force);

private:

	std::list<OneMorph*> _morph;

	// int width, height;
};

#endif /* INCLUDED_Morph_H */
