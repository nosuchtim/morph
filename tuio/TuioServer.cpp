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

#include "NosuchUtil.h"
#include "TuioServer.h"

using namespace TUIO;

TuioServer::TuioServer() {
	verbose = 0;
	initial_session_id = 10000;
	device_multiplier = 1000;
}

TuioServer::~TuioServer() {
}

void
TuioServer::adjustXY(float& x, float& y) {
	if (flipY) {
		y = 1.0f - y;
	}
	if (flipX) {
		x = 1.0f - x;
	}
}
TuioCursor* TuioServer::addTuioCursor(float x, float y) {
	sessionID++;
	
	adjustXY(x, y);

	if (verbose) {
		printf("Added Cursor sid=%d x=%.4f y=%.4f\n", sessionID,x,y);
	}

	int cursorID = (int)cursorList.size();
	if (((int)(cursorList.size())<=maxCursorID) && ((int)(freeCursorList.size())>0)) {
		std::list<TuioCursor*>::iterator closestCursor = freeCursorList.begin();
		
		for(std::list<TuioCursor*>::iterator iter = freeCursorList.begin();iter!= freeCursorList.end(); iter++) {
			if((*iter)->getDistance(x,y)<(*closestCursor)->getDistance(x,y)) closestCursor = iter;
		}
		
		TuioCursor *freeCursor = (*closestCursor);
		cursorID = (*closestCursor)->getCursorID();
		freeCursorList.erase(closestCursor);
		delete freeCursor;
	} else maxCursorID = cursorID;	
	
	TuioCursor *tcur = new TuioCursor(sessionID, cursorID, x, y);
	cursorList.push_back(tcur);
	updateCursor = true;

	if (verbose > 2) 
		std::cout << "add cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;

	return tcur;
}

TuioCursor* TuioServer::addTuioCursorId(float x, float y, int uid, int id) {

	adjustXY(x, y);

	if (verbose) {
		printf("Added Cursor uid=%d id=%d x=%.4f y=%.4f\n", uid, id,x,y);
	}

	sessionID = uid;
	int cursorID = id;
/*
	if (((int)(cursorList.size())<=maxCursorID) && ((int)(freeCursorList.size())>0)) {
		std::list<TuioCursor*>::iterator closestCursor = freeCursorList.begin();
		
		for(std::list<TuioCursor*>::iterator iter = freeCursorList.begin();iter!= freeCursorList.end(); iter++) {
			if((*iter)->getDistance(x,y)<(*closestCursor)->getDistance(x,y)) closestCursor = iter;
		}
		
		TuioCursor *freeCursor = (*closestCursor);
		cursorID = (*closestCursor)->getCursorID();
		freeCursorList.erase(closestCursor);
		delete freeCursor;
	} else maxCursorID = cursorID;	
*/
	
	TuioCursor *tcur = new TuioCursor(sessionID, cursorID, x, y);
	cursorList.push_back(tcur);
	updateCursor = true;

	if (verbose > 2) 
		std::cout << "add cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;

	return tcur;
}

void TuioServer::updateTuioCursor(TuioCursor *tcur,float x, float y) {
	if (tcur==NULL) return;
	adjustXY(x, y);
	tcur->update(x,y);
	updateCursor = true;

	if (verbose > 2 && tcur->isMoving())	 	
		std::cout << "set cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() 
		<< " " << tcur->getXSpeed() << " " << tcur->getYSpeed() << " " << tcur->getMotionAccel() << " " << std::endl;
}

void TuioServer::removeTuioCursor(TuioCursor *tcur) {
	if (tcur==NULL) return;
	cursorList.remove(tcur);
	tcur->remove();
	updateCursor = true;

	if (verbose > 2)
		std::cout << "del cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ")" << std::endl;

	if (tcur->getCursorID()==maxCursorID) {
		maxCursorID = -1;
		delete tcur;
		
		if (cursorList.size()>0) {
			std::list<TuioCursor*>::iterator clist;
			for (clist=cursorList.begin(); clist != cursorList.end(); clist++) {
				int cursorID = (*clist)->getCursorID();
				if (cursorID>maxCursorID) maxCursorID=cursorID;
			}
			
			freeCursorBuffer.clear();
			for (std::list<TuioCursor*>::iterator flist=freeCursorList.begin(); flist != freeCursorList.end(); flist++) {
				TuioCursor *freeCursor = (*flist);
				if (freeCursor->getCursorID()>maxCursorID) delete freeCursor;
				else freeCursorBuffer.push_back(freeCursor);
			}
			freeCursorList = freeCursorBuffer;
			
		} else {
			for (std::list<TuioCursor*>::iterator flist=freeCursorList.begin(); flist != freeCursorList.end(); flist++) {
				TuioCursor *freeCursor = (*flist);
				delete freeCursor;
			}
			freeCursorList.clear();
		} 
	} else if (tcur->getCursorID()<maxCursorID) {
		freeCursorList.push_back(tcur);
	}
}

long TuioServer::getSessionID() {
	sessionID++;
	return sessionID;
}

TuioCursor* TuioServer::getClosestTuioCursor(float xp, float yp) {

	TuioCursor *closestCursor = NULL;
	float closestDistance = 1.0f;

	for (std::list<TuioCursor*>::iterator iter=cursorList.begin(); iter != cursorList.end(); iter++) {
		float distance = (*iter)->getDistance(xp,yp);
		if(distance<closestDistance) {
			closestCursor = (*iter);
			closestDistance = distance;
		}
	}
	
	return closestCursor;
}

std::list<TuioCursor*> TuioServer::getTuioCursors() {
	return cursorList;
}
