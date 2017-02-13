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
#include <math.h>
#include "TuioDevice.h"
#include "sensel.h"

using namespace TUIO;

#define MORPH_MAX_FORCE 1000.0
#define MAX_IGESTURE_ID 12
#define DEVICE_MULTIPLIER 1000

class Morph : public TuioDevice { 
	
public:
	Morph(TuioServer* s);
	~Morph() {
	};
	
	void run();
	bool init();
	int check() { return 0; };
	void pressed(float x, float y, int uid, int id, float force);
	void released(float x, float y, int uid, int id, float force);
	void dragged(float x, float y, int uid, int id, float force);

	int uid_for_id[MAX_IGESTURE_ID*12*DEVICE_MULTIPLIER];   // 5 devices, 12 fingers per device

private:

	void _mycallback(int devnum, int fingnum, int event, float x, float y, float prox);
	static Morph* _me;

	int verbose, running;
	int wasupdated;
	
	int width, height;
	int s_id;
	contact_t *contacts;
	int n_contacts;
};

#endif /* INCLUDED_Morph_H */
