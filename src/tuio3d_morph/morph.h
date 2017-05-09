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

#include "TuioServer.h"
#include "TuioCursor.h"
#include <list>
#include <map>
#include <math.h>
#include "TuioDevice.h"
#include "sensel.h"
#include "sensel_protocol.h"

using namespace TUIO;

#define MORPH_MAX_FORCE 1000.0f
#define MORPH_WIDTH 230
#define MORPH_HEIGHT 130
#define MAX_IGESTURE_ID 12

class OneMorph {
public:
	OneMorph(SENSEL_HANDLE h, unsigned char* serial, int sid) : _handle(h), _initialsid(sid), _serialnum(serial) {
	}
	SENSEL_HANDLE _handle;
	int _initialsid;
	unsigned char* _serialnum;
	SenselFrameData* _frame;
};

class Morph : public TuioDevice { 
	
public:
	Morph(TuioServer* s, std::map<unsigned char*,int> );
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
