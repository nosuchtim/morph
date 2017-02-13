/*
 TUIO Server Component - part of the reacTIVision project
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

#pragma once

#ifndef WIN32
#include <pthread.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include <iostream>
#include <list>
#include <algorithm>

#include "osc/OscOutboundPacketStream.h"
#include "ip/NetworkingUtils.h"
#include "ip/UdpSocket.h"

class MMTT_SharedMem;

#include "TuioCursor.h"

#define IP_MTU_SIZE 1500
#define MAX_UDP_SIZE 65536
#define MIN_UDP_SIZE 576
#define OBJ_MESSAGE_SIZE 108	// setMessage + seqMessage size
#define CUR_MESSAGE_SIZE 88

namespace TUIO {

	class TuioServer { 
		
	public:

		TuioServer();
		~TuioServer();
		
		TuioCursor* addTuioCursor(float xp, float yp);
		TuioCursor* addTuioCursorId(float x, float y, int uid, int id);
		void updateTuioCursor(TuioCursor *tcur, float xp, float yp);
		void removeTuioCursor(TuioCursor *tcur);
		long getSessionID();
		std::list<TuioCursor*> getTuioCursors();

		void adjustXY(float& x, float& y);
		
		TuioCursor* getClosestTuioCursor(float xp, float yp);
		bool isConnected() { return connected; }

		int verbose;
		int initial_session_id;
		int device_multiplier;
		bool flipX;
		bool flipY;
		
		virtual void update() { }

	protected:
		std::list<TuioCursor*> cursorList;
		long sessionID;
		int maxCursorID;
		bool updateCursor;
		long lastCursorUpdate;
		bool connected;

	private:
		std::list<TuioCursor*> freeCursorList;
		std::list<TuioCursor*> freeCursorBuffer;
		
	};
};