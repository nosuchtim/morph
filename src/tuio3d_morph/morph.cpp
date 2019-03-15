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

#include <map>

using namespace TUIO;
extern int Verbose;

bool PrintList = false;

volatile sig_atomic_t ctrl_c_requested = false;

void handle_ctrl_c(int sig)
{
	ctrl_c_requested = true;
}

AllMorphs::AllMorphs(std::map<unsigned char*,unsigned char*> serialmap) {

	SenselDeviceList devlist;

	senselGetDeviceList(&devlist);

	// If no explicit serialmap is given, we create one.
	if (serialmap.size() == 0) {
		int sidinitial = 10000;
		int sidmultiplier = SIDMULTIPLIER;
		for (int i = 0; i < devlist.num_devices; i++) {
			SenselDeviceID& dev = devlist.devices[i];
			std::string sd = NosuchSnprintf("%d",sidinitial + i*(sidmultiplier));
			serialmap.insert(std::pair<unsigned char*, unsigned char*>(dev.serial_num, (unsigned char*)sd.c_str()));
		}
	}

	int nfound = 0;
	for (auto& x : serialmap) {
		SENSEL_HANDLE h;
		unsigned char* serial = x.first;
		if (devlist.num_devices > 0 && senselOpenDeviceBySerialNum(&h, serial) == SENSEL_OK) {
			_morph.push_back(new OneMorph(h, serial, x.second));
			nfound++;
		}
		else {
			fprintf(stdout, "Unable to find Morph with serial number '%s'\n", serial);
		}
	}

	if (devlist.num_devices == 0) {
		fprintf(stdout, "Error: no Sensel devices found!\n");
	}

}

bool
AllMorphs::init()
{
	signal(SIGINT, handle_ctrl_c);

	if (_morph.size() == 0) {
		fprintf(stdout, "No Morphs have been found!\n");
		return false;
	}
	bool sensel_sensor_opened = false;

	for (auto& morph : _morph) {
		SENSEL_HANDLE h = morph->_handle;
		senselSetFrameContent(h, FRAME_CONTENT_CONTACTS_MASK);
		senselAllocateFrameData(h, &morph->_frame);
		senselStartScanning(h);
		for (int led = 0; led < 16; led++) {
			senselSetLEDBrightness(h, led, 0); //turn off LED
		}
	}

	return true;
}

void OneMorph::pressed(MorphArea* area, float x, float y, int sid, int cid, float force) {
	if (Verbose > 1) {
		fprintf(stdout, "PRESSED sid=%d\n", sid);
	}
	TuioCursor *c = area->server->addTuioCursorId(x,y,sid,cid);
	c->setForce(force);
}

void OneMorph::dragged(MorphArea* area, float x, float y, int sid, int cid, float force) {

	TuioCursor *match = NULL;

	if (Verbose > 1) {
		fprintf(stdout, "DRAGGED sid=%d\n", sid);
	}
	std::list<TuioCursor*> cursorList = area->server->getTuioCursors();

	// XXX - use auto here
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getSessionID()) == sid) {
			match = (*tuioCursor);
			break;
		}
	}
	if ( match == NULL ) {
		if (Verbose > 0) {
			fprintf(stdout, "Drag seen without press? Automatically pressing.  sid=%d\n", sid);
		}
		match = area->server->addTuioCursorId(x,y,sid,cid);
	} else {
		area->server->updateTuioCursor(match,x,y);
		match->setForce(force);
	}
}

void OneMorph::released(MorphArea* area, int sid) {
	// printf("released  uid=%d id=%d\n",uid,id);
	if (Verbose > 1) {
		fprintf(stdout, "RELEASED sid=%d  server=%lld   server->sidInitial=%d\n", sid,  (long long) area->server, area->server->sidInitial);
		// TuioServer::printAllLists();
	}
	std::list<TuioCursor*> cursorList = area->server->getTuioCursors();

	TuioCursor *match = NULL;
	// XXX - use auto here
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if (((*tuioCursor)->getSessionID()) == sid) {
			match = (*tuioCursor);
			break;
		}
	}
	if (match!=NULL) {
		area->server->removeTuioCursor(match);
	}

	// TuioServer::printAllLists();
}

void
AllMorphs::listdevices() {
	SenselDeviceList list;

	senselGetDeviceList(&list);
	if (list.num_devices == 0) {
		fprintf(stdout, "No Sensel devices found!\n");
	}

	for (int i = 0; i < list.num_devices; i++) {
		SenselDeviceID& dev = list.devices[i];
		fprintf(stdout, "Sensel Morph device port=%s idx=%d serialnum=%s\n", dev.com_port, dev.idx, dev.serial_num);
	}
}

#define MAX_MORPHS 16

void AllMorphs::run() {

	while (!ctrl_c_requested) {
		for (auto& morph : _morph) {
			SENSEL_HANDLE h = morph->_handle;
			SenselFrameData* frame = morph->_frame;
			senselReadSensor(h);
			unsigned int num_frames = 0;
			senselGetNumAvailableFrames(h, &num_frames);
			for (unsigned int f = 0; f < num_frames; f++) {
				senselGetFrame(h, frame);
				if (Verbose > 2 && frame->n_contacts > 0) {
					printf("FRAME f=%d n_contacts=%d\n", f, frame->n_contacts);
				}
				for (int i = 0; i < frame->n_contacts; i++) {

					unsigned int state = frame->contacts[i].state;
					char* statestr =
						state == CONTACT_INVALID ? "CONTACT_INVALID" :
						state == CONTACT_START ? "CONTACT_START" :
						state == CONTACT_MOVE ? "CONTACT_MOVE" :
						state == CONTACT_END ? "CONTACT_END" :
						"UNKNOWN_CONTACT_STATE";

					SenselContact& c = frame->contacts[i];
					float force = c.total_force;
					float x_mm = c.x_pos;
					float y_mm = c.y_pos;
					//Read out shape information (ellipses)
					float major = c.major_axis;
					float minor = c.minor_axis;
					float orientation = c.orientation;

					float x_norm = x_mm / MORPH_WIDTH;
					float y_norm = y_mm / MORPH_HEIGHT;
					float f_norm = force / MORPH_MAX_FORCE;

					if (x_norm > 1.0) {
						x_norm = 1.0;
					}
					if (y_norm > 1.0) {
						y_norm = 1.0;
					}
					// leave f_norm alone, let it go higher

					if (Verbose > 2) {
						fprintf(stdout, "Serial: %s   Contact ID: %d   State: %s   xy=%.4f,%.4f\n",
							morph->_serialnum, c.id, statestr, x_mm, y_mm);
					}
		
					// Note: this method figures out which area we're in,
					// and normalizes the coordinates within it to (0,0),(1,1)
					MorphArea* area;
					int sid = morph->mapToSidArea(x_norm,y_norm,area) + c.id;
					if (sid < 0 || area == NULL) {
						fprintf(stdout, "Warning, no sid for that position on that pad! x_mm=%lf x_norm=%lf y_mm=%lf y_norm=%lf\n",x_mm,x_norm,y_mm,y_norm);
						continue;
					}
					int previousSid = morph->previousSidFor(c.id);
					if (previousSid >= 0 && previousSid != sid) {
						if (c.state != CONTACT_MOVE) {
							fprintf(stdout, "Unexpected state=%d for previousSid=%d sid=%d\n",c.state,previousSid,sid);
						}
						// This is what happens when you drag across a region boundary.
						// So, we release the previous sid and press the new one.
						if (Verbose > 1) {
							fprintf(stdout, "======= CROSS-AREA DRAG! state was %d c.id=%d previousSid=%d sid=%d\n", c.state, c.id, previousSid, sid);
						}

						MorphArea* previousArea = morph->areaForSid(previousSid);
						if (previousArea) {
							morph->released(previousArea,previousSid);
						}
						else {
							fprintf(stdout, "UNABLE TO FIND AREA for previousSid=%d ?\n",previousSid);
						}

						morph->unsetPreviousSid(c.id);

						// fprintf(stdout, "AFTER UNSETPREVIOUSSID cid=%d list dump follows\n", c.id);
						// for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
						// 	TuioCursor* t = *tuioCursor;
						// 	fprintf(stdout, "   CURSOR = %d\n",t->getSessionID());
						// }

						// XXXXX - for some reason, if I try to
						// insert a fake CONTACT_START here (as opposed to
						// waiting for the next CONTACT_MOVE), it ends up
						// leaving a cursor around.
						continue;

						// c.state = CONTACT_START;
						// continue on below as if it was a press
					}

					morph->setPreviousSid(c.id, sid);

					if (Verbose > 2) {
						fprintf(stdout, "Serial: %s   Contact ID: %d   Session ID: %d   State: %s   xy=%.4f,%.4f\n",
							morph->_serialnum, c.id, sid, statestr, x_mm, y_mm);
						// TuioServer::printAllLists();
					}
		
					char* event = "unknown";
					switch (c.state)
					{
					case CONTACT_START:
						event = "start";
						morph->pressed(area, x_norm, y_norm, sid, c.id, f_norm);
						break;

					case CONTACT_MOVE:
						event = "move";
						morph->dragged(area, x_norm, y_norm, sid, c.id, f_norm);
						break;

					case CONTACT_END:
						event = "end";
						morph->released(area, sid);
						morph->unsetPreviousSid(c.id);
						break;

					case CONTACT_INVALID:
						event = "invalid";
						break;
					}
		
					// printf("TUIO  Contact ID %d, event=%s, mm coord: (%f, %f), force=%f, " \
					// 	"major=%f, minor=%f, orientation=%f\n",
					// 	id, event, x_mm, y_mm, force, major, minor, orientation);
				}
			}
			TuioServer::updateAllServers();
			// morph->update();
		}

		Sleep(1);  // some sort of throttle is probably needed
	}
}
