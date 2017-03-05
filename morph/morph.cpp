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

#include <signal.h>

#include "TuioServer.h"
#include "TuioCursor.h"
#include "Morph.h"
#include "xgetopt.h"
#include "tchar.h"

using namespace TUIO;

volatile sig_atomic_t ctrl_c_requested = false;

void handle_ctrl_c(int sig)
{
	ctrl_c_requested = true;
}

void Morph::pressed(float x, float y, int uid, int id, float force) {
	// printf("clicked %f %f   uid=%d id=%d\n",x,y,uid,id);

	uid = uid_for_id[id];
	if ( uid == 0 ) {
		uid = ++s_id;
		uid_for_id[id] = uid;
	}
	TuioCursor *c = server->addTuioCursorId(x,y,uid,id);
	c->setForce(force);

	std::list<TuioCursor*> cursorList = server->getTuioCursors();
	wasupdated++;
}

void Morph::dragged(float x, float y, int uid, int id, float force) {
	// printf("dragged %f %f   uid=%d id=%d\n",x,y,uid,id);
	TuioCursor *match = NULL;
	uid = uid_for_id[id];
	if ( uid == 0 ) {
		printf("UNEXPECTED!  uid==0 in dragged!?\n");
		uid = ++s_id;
		uid_for_id[id] = uid;
	}
	std::list<TuioCursor*> cursorList = server->getTuioCursors();
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getCursorID()) == (id)) {
			// printf("dragged found match of cursor id=%d !!\n",id);
			match = (*tuioCursor);
			break;
		}
	}
	if ( match == NULL ) {
		printf("Hey, didn't find existing cursor with id=%d !?\n",id);
	} else {
		// if (cursor->getTuioTime()==currentTime) return;
		server->updateTuioCursor(match,x,y);
		// printf("UPDATING TuioCursor with xy=%f,%f\n",x,y);
		match->setForce(force);
		wasupdated++;
	}
}

void Morph::released(float x, float y, int uid, int id, float force) {
	// printf("released  uid=%d id=%d\n",uid,id);
	uid_for_id[id] = 0;
	std::list<TuioCursor*> cursorList = server->getTuioCursors();
	TuioCursor *match = NULL;
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getCursorID()) == (id)) {
			// printf("released found cursor id=%d !!\n",id);
			match = (*tuioCursor);
			break;
		}
	}
	if (match!=NULL) {
		server->removeTuioCursor(match);
		wasupdated++;
	}
	if ((int)(server->getTuioCursors()).size() == 0) {
		// printf("Should be resetting sid?\n");
		s_id = 0;
	}
}

Morph::Morph(TuioServer* s) : TuioDevice(s) {

	s_id = 0;
	wasupdated = 0;
	_me = this;

	memset(uid_for_id,0,sizeof(uid_for_id));

	contacts = NULL;
	n_contacts = 0;
}

Morph* Morph::_me = 0;
#define MAX_MORPHS 16

void Morph::_mycallback(int devnum, int fingnum, int event, float x, float y, float prox)
{
	// We want the first device (whatever its devnum is) to start with the initial_session_id,
	// so, we create a map of devnum to the initial_session_id for that devnum
	static int devnum_initial_session_id[MAX_MORPHS];
	static bool initialized = false;
	static int next_initial_session_id;

	if (!initialized) {
		for (int i = 0; i < MAX_MORPHS; i++) {
			devnum_initial_session_id[i] = -1;
		}
		next_initial_session_id = _me->server->initial_session_id;
		initialized = true;
	}

	if (devnum_initial_session_id[devnum] < 0) {
		devnum_initial_session_id[devnum] = next_initial_session_id;
		next_initial_session_id += _me->server->device_multiplier;
	}

	int id = devnum_initial_session_id[devnum] + fingnum;
	// printf("MYCALLBACK!! fing=%d xy=%f,%f\n",fingnum,x,y);
	switch(event) {

		// case FINGER_DRAG:  // UPDATE
		case SENSEL_EVENT_CONTACT_MOVE:
			_me->dragged(x, y, id, id, prox);
			break;

		// case FINGER_DOWN:  // START
		case SENSEL_EVENT_CONTACT_START:
			_me->pressed(x, y, id, id, prox);
			break;

		// case FINGER_UP:  // END
		case SENSEL_EVENT_CONTACT_END:
			_me->released(x, y, id, id, prox);
			break;
	}	
	// printf("end MYCALLBACK\n");
}

bool
Morph::init()
{
	signal(SIGINT, handle_ctrl_c);

	bool sensel_sensor_opened = false;

	sensel_sensor_opened = senselOpenConnection(0);
	if ( !sensel_sensor_opened ) {
		return false;
	}
	//Enable contact sending
	senselSetFrameContentControl(SENSEL_FRAME_CONTENT_CONTACTS_MASK);

	//Enable scanning
	senselStartScanning();

	return true;
}

void Morph::run() {

	int devnum = 0;

	while (!ctrl_c_requested)
	{
		senselReadFrame(&contacts, &n_contacts, NULL, NULL);

		for (int i = 0; i < n_contacts; i++)
		{
			float force = contacts[i].total_force;
			float x_mm = contacts[i].x_pos_mm;
			float y_mm = contacts[i].y_pos_mm;
			//Read out shape information (ellipses)
			float major = contacts[i].major_axis_mm;
			float minor = contacts[i].minor_axis_mm;
			float orientation = contacts[i].orientation_degrees;

#define MORPH_WIDTH 230
#define MORPH_HEIGHT 130
			float x = x_mm / MORPH_WIDTH;
			float y = y_mm / MORPH_HEIGHT;
			float f = force / MORPH_MAX_FORCE;

			int id = contacts[i].id;
			int event_type = contacts[i].type;

			char* event;
			switch (event_type)
			{
			case SENSEL_EVENT_CONTACT_INVALID:
				event = "invalid";
				break;
			case SENSEL_EVENT_CONTACT_START:
				senselSetLEDBrightness(id, 100); //turn on LED
				_mycallback(devnum, id, event_type, x, y, f);
				event = "start";
				break;
			case SENSEL_EVENT_CONTACT_MOVE:
				event = "move";
				_mycallback(devnum, id, event_type, x, y, f);
				break;
			case SENSEL_EVENT_CONTACT_END:
				senselSetLEDBrightness(id, 0); //turn LED off
				event = "end";
				_mycallback(devnum, id, event_type, x, y, f);
				break;
			default:
				event = "error";
			}

			// printf("Contact ID %d, event=%s, mm coord: (%f, %f), force=%f, " \
			// 	"major=%f, minor=%f, orientation=%f\n",
			// 	id, event, x_mm, y_mm, force, major, minor, orientation);
		}

		server->update();
		Sleep(1);  // some sort of throttle is probably needed
	}
}
