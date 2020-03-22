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

extern int Verbose;

std::list<TuioServer*> TuioServer::allservers;

TuioServer::TuioServer() {
}

TuioServer::~TuioServer() {
}

TuioCursor* TuioServer::addTuioCursor(float x, float y) {
	sessionID++;
	
	int cursorID = (int)cursorList.size();
	if (((int)(cursorList.size())<=maxCursorID) && ((int)(freeCursorList.size())>0)) {
		std::list<TuioCursor*>::iterator closestCursor = freeCursorList.begin();
		
		for(std::list<TuioCursor*>::iterator iter = freeCursorList.begin();iter!= freeCursorList.end(); iter++) {
			if((*iter)->getDistance(x,y)<(*closestCursor)->getDistance(x,y)) closestCursor = iter;
		}
		
		TuioCursor *freeCursor = (*closestCursor);
		cursorID = (*closestCursor)->getCursorID();
		freeCursorList.erase(closestCursor);
		std::cout << "DELETING freeCursor=" << (long long)freeCursor << std::endl;
		delete freeCursor;
	} else maxCursorID = cursorID;	
	
	TuioCursor *tcur = new TuioCursor(sessionID, cursorID, x, y);

	std::cout << "ADDING TO CURSORLIST!  tcur=" << (long long)tcur << "  sid=" << tcur->getSessionID() << std::endl;
	cursorList.push_back(tcur);

	for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
		TuioCursor *tc = (*t);
		std::cout << "ADDED A tcur=" << (long long)tc << "  sessionid=" << (long)((*t)->getSessionID()) << std::endl;
	}

	// Check for duplicates.  Silly inefficient, just to find bug
	std::list<TuioCursor*>::iterator clist1;
	for (clist1 = cursorList.begin(); clist1 != cursorList.end(); clist1++) {
		TuioCursor *t1 = *clist1;
		std::list<TuioCursor*>::iterator clist2;
		for (clist2 = cursorList.begin(); clist2 != cursorList.end(); clist2++) {
			TuioCursor *t2 = *clist2;
			if (t1 != t2 && t1->getSessionID() == t2->getSessionID()) {
				std::cout << "DUPLICATE SESSION IDS!!" << std::endl;
			}
		}
	}

	setUpdateCursor();

	if (Verbose > 2) 
		std::cout << "add cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;

	return tcur;
}

TuioCursor* TuioServer::addTuioCursorId(float x, float y, int uid, int id) {

	sessionID = uid;
	int cursorID = id;
	
	TuioCursor *tcur = new TuioCursor(sessionID, cursorID, x, y);

    // For some reason, the list may already contain a cursor with the same sessionID.
	// Possible reason - not receiving a CONTACT_END
	for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
		TuioCursor *tc = (*t);
		if ( sessionID == tc->getSessionID() ) {
			std::cout << "===================================================================== FOUND DUPLICATE SESSIONID! sessionid=" << (long)((*t)->getSessionID()) << std::endl;
			removeTuioCursor(tc);
			std::cout << "========================================================================== REMOVED tc= " << (long long)tc << std::endl;
			// Can't continue unless we restart the iterator, or use the iterator methods for removing an entry
			break;
		}
	}

	cursorList.push_back(tcur);

	// for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
	// 	TuioCursor *tc = (*t);
	// 	DWORD dw = GetCurrentThreadId();
	// 	std::cout << "thread="<<dw<<" ADDED B tcur=" << (long long)tc << "  sessionid=" << (long)((*t)->getSessionID()) << std::endl;
	// }

	// Check for duplicates.  Silly inefficient, just to find bug
	std::list<TuioCursor*>::iterator clist1;
	for (clist1 = cursorList.begin(); clist1 != cursorList.end(); clist1++) {
		TuioCursor *t1 = *clist1;
		std::list<TuioCursor*>::iterator clist2;
		for (clist2 = cursorList.begin(); clist2 != cursorList.end(); clist2++) {
			TuioCursor *t2 = *clist2;
			if (t1 != t2 && t1->getSessionID() == t2->getSessionID()) {
				std::cout << "DUPLICATE SESSION IDS!!" << std::endl;
			}
		}
	}

	setUpdateCursor();

	if (Verbose > 2) 
		std::cout << "add cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;

	return tcur;
}

void TuioServer::updateTuioCursor(TuioCursor *tcur,float x, float y) {
	if (tcur == NULL) {
		return;
	}
	tcur->update(x,y);

	setUpdateCursor();

	if (Verbose > 2 && tcur->isMoving())	 	
		std::cout << "set cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() 
		<< " " << tcur->getXSpeed() << " " << tcur->getYSpeed() << " " << tcur->getMotionAccel() << " " << std::endl;
}

void TuioServer::removeTuioCursor(TuioCursor *tcur) {
	if (tcur == NULL) {
		std::cout << "removeTuioCursor tcur=NULL?" << std::endl;
		return;
	}

	// std::cout << "removeTuioCursor tcur=" << (long long)tcur << "  sessionid=" << tcur->getSessionID() << std::endl;

	// for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
	// 	TuioCursor *tc = (*t);
	// 	std::cout << "Before REMOVING tc =" << (long long)tc << "  sessionid=" << (long)((*t)->getSessionID()) << std::endl;
	// }

	cursorList.remove(tcur);
	tcur->remove();
	setUpdateCursor();

	// for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
	// 	TuioCursor *tc = (*t);
	// 	std::cout << "AFTER REMOVING tc =" << (long long)tc << "  sessionid=" << (long)((*t)->getSessionID()) << std::endl;
	// }

	if (Verbose > 2) {
		std::cout << "del cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ")" << std::endl;
	}

	if (tcur->getCursorID()==maxCursorID) {
		maxCursorID = -1;
		// std::cout << "DELETING tcur=" << (long long)tcur << std::endl;
		delete tcur;
		
		if (cursorList.size()>0) {
			std::list<TuioCursor*>::iterator clist;
			for (clist=cursorList.begin(); clist != cursorList.end(); clist++) {
				int cursorID = (*clist)->getCursorID();
				if (cursorID>maxCursorID) maxCursorID=cursorID;
			}
			
			freeCursorBuffer.clear();
			for (std::list<TuioCursor*>::iterator flist = freeCursorList.begin(); flist != freeCursorList.end(); flist++) {
				TuioCursor *freeCursor = (*flist);
				if (freeCursor->getCursorID() > maxCursorID) {
					std::cout << "DELETING freeCursor > maxCursorID =" << (long long)freeCursor << std::endl;
					delete freeCursor;
				}
				else {
					std::cout << "PUSH_BACK freeCursor =" << (long long)freeCursor << std::endl;
					freeCursorBuffer.push_back(freeCursor);
				}
			}
			freeCursorList = freeCursorBuffer;
			
		} else {
			for (std::list<TuioCursor*>::iterator flist=freeCursorList.begin(); flist != freeCursorList.end(); flist++) {
				TuioCursor *freeCursor = (*flist);
				std::cout << "DELETING freeCursor (*flist) =" << (long long)freeCursor << std::endl;
				delete freeCursor;
			}
			freeCursorList.clear();
		} 
	} else if (tcur->getCursorID()<maxCursorID) {
		std::cout << "PUTTING on freeCursorList tcur=" << (long long)tcur << std::endl;
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
