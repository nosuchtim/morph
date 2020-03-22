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
#include "TuioUdpServer.h"

using namespace TUIO;
using namespace osc;

extern int Verbose;

#ifndef WIN32
static void* ThreadFunc( void* obj )
#else
static DWORD WINAPI ThreadFunc( LPVOID obj )
#endif
{
	TuioUdpServer *server = static_cast<TuioUdpServer*>(obj);
	while ((server->isConnected()) && (server->periodicMessagesEnabled())) {
		server->sendFullMessages();
#ifndef WIN32
		usleep(USEC_SECOND*tuioServer->getUpdateInterval());
#else
		Sleep(MSEC_SECOND*server->getUpdateInterval());
#endif
	}	
	return 0;
};

void TuioUdpServer::enablePeriodicMessages(int interval) {
	if (periodic_update) return;
	
	update_interval = interval;
	periodic_update = true;
	
#ifndef WIN32
	pthread_create(&thread , NULL, ThreadFunc, this);
#else
	DWORD threadId;
	thread = CreateThread( 0, 0, ThreadFunc, this, 0, &threadId );
	std::cout << "CREATED THREAD TheadFunc, threadId = " << threadId << std::endl;
#endif
}

void TuioUdpServer::disablePeriodicMessages() {
	if (!periodic_update) return;
	periodic_update = false;
	
#ifdef WIN32
	if( thread ) CloseHandle( thread );
#endif
	thread = NULL;	
}

void TuioUdpServer::sendFullMessages() {
	
	// prepare the cursor packet
	fullPacket->Clear();
	(*fullPacket) << osc::BeginBundleImmediate;
	
	// add the cursor alive message
	(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive";
	if ( Verbose > 2 ) {
		std::cout << "/tuio/25Dcur alive ";
	}
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		(*fullPacket) << (int32)((*tuioCursor)->getSessionID());	
		if ( Verbose > 2 ) {
			std::cout << " " << (int32)((*tuioCursor)->getSessionID());	
		}
	}
	(*fullPacket) << osc::EndMessage;	
	if ( Verbose > 2 ) {
		std::cout << std::endl;
	}

	// add all current cursor set messages
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		
		// start a new packet if we exceed the packet capacity
		if ((fullPacket->Capacity()-fullPacket->Size())<CUR_MESSAGE_SIZE) {
			
			// add the immediate fseq message and send the cursor packet
			(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << -1 << osc::EndMessage;
			(*fullPacket) << osc::EndBundle;
			socket->Send( fullPacket->Data(), fullPacket->Size() );
			if ( Verbose > 2 ) {
				std::cout << "/tuio/25Dcur fseq -1" << std::endl;
			}

			// prepare the new cursor packet
			fullPacket->Clear();	
			(*fullPacket) << osc::BeginBundleImmediate;
			
			// add the cursor alive message
			(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive";
			if ( Verbose > 2 ) {
				std::cout << "/tuio/25Dcur alive";
			}
			for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
				(*fullPacket) << (int32)((*tuioCursor)->getSessionID());	
				if ( Verbose > 2 ) {
					std::cout << " " << (int32)((*tuioCursor)->getSessionID());	
				}
			}
			(*fullPacket) << osc::EndMessage;				
			if ( Verbose > 2 ) {
				std::cout << std::endl;
			}
		}

		float x = (*tuioCursor)->getX();
		float y = (*tuioCursor)->getY();

		// add the actual cursor set message
		(*fullPacket) << osc::BeginMessage("/tuio/25Dcur") << "set";
		(*fullPacket) << (int32)((*tuioCursor)->getSessionID()) << x << y << (*tuioCursor)->getForce();
		(*fullPacket) << (*tuioCursor)->getXSpeed() << (*tuioCursor)->getYSpeed() << (*tuioCursor)->getForceSpeed() << (*tuioCursor)->getMotionAccel();
		(*fullPacket) << osc::EndMessage;
		if (Verbose) {
			std::cout << "/tuio/25Dcur set"
				<< " " << (int32)((*tuioCursor)->getSessionID()) << " " << x << " " << y << " " << (*tuioCursor)->getForce()
				<< " " << (*tuioCursor)->getXSpeed() << " " << (*tuioCursor)->getYSpeed() << " " << (*tuioCursor)->getForceSpeed() << " " << (*tuioCursor)->getMotionAccel()
				<< std::endl;
		}
	}

	// add the immediate fseq message and send the cursor packet
	(*fullPacket) << osc::BeginMessage("/tuio/25Dcur") << "fseq" << -1 << osc::EndMessage;
	(*fullPacket) << osc::EndBundle;
	socket->Send(fullPacket->Data(), fullPacket->Size());
	if (Verbose > 2) {
		std::cout << "/tuio/25Dcur fseq -1" << std::endl;
	}

}

TuioUdpServer::TuioUdpServer(std::string host, int port, int alive_interval, int si) {
	alive_update_interval = alive_interval;
	// If the alive_update_interval is 0, there won't be any periodic update
	periodic_update = (alive_update_interval > 0);
	initialize(host, port);
	sidInitial = si;
}

void TuioUdpServer::initialize(std::string host, int port) {
	hostname = host;
	portnum = port;
	int size = IP_MTU_SIZE;
	if (size > MAX_UDP_SIZE) size = MAX_UDP_SIZE;
	else if (size < MIN_UDP_SIZE) size = MIN_UDP_SIZE;

	try {
		long unsigned int ip = GetHostByName(host.c_str());
		socket = new UdpTransmitSocket(IpEndpointName(ip, port));

		oscBuffer = new char[size];
		oscPacket = new osc::OutboundPacketStream(oscBuffer, size);
		fullBuffer = new char[size];
		fullPacket = new osc::OutboundPacketStream(fullBuffer, size);
	}
	catch (std::exception &e) {
		std::cout << "could not create socket: " << e.what() << std::endl;
		socket = NULL;
	}

	currentFrame = sessionID = maxCursorID = -1;

	clearUpdateCursor();

	lastCursorUpdate = timeGetTime();

	sendEmptyCursorBundle();

	connected = true;
}

TuioUdpServer::~TuioUdpServer() {
	connected = false;

	sendEmptyCursorBundle();

	delete oscPacket;
	delete[]oscBuffer;
	delete fullPacket;
	delete[]fullBuffer;
	delete socket;
}

void TuioUdpServer::commitFrame() {

	if (updateCursor) {
		startCursorBundle();
		for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor != cursorList.end(); ) {

			// start a new packet if we exceed the packet capacity
			if ((oscPacket->Capacity() - oscPacket->Size()) < CUR_MESSAGE_SIZE) {
				sendCursorBundle(++currentFrame);
				startCursorBundle();
			}

			TuioCursor *tcur = (*tuioCursor);

			long tm = timeGetTime();

			// If it's too old, remove it.  There's an intermittent bug somewhere
			// that occasionally leaves TuioCursors around, so this works around that.
#define TOO_OLD 8000
			if ( (tm - tcur->getCreationTime()) > TOO_OLD ) {
				std::cout << "================== REMOVING TOO_OLD CURSOR!!!  tcur=" << (long long) tcur << std::endl;
				tuioCursor++;
				cursorList.remove(tcur);
				tcur->remove();
				std::cout << "DELETING tcur=" << (long long)tcur << std::endl;
				delete tcur;
				std::cout << "================== AFTER REMOVING TOO_OLD CURSOR!!!" << std::endl;
			} else {
#if 0
				std::cout << "===== commitFrame addCursorMessage"
					<< " id=" << tcur->getCursorID()
					<< " time=" << tcur->getCreationTime()
					<< " currtime=" << tm
					<< " age=" << (tm - tcur->getCreationTime())
					<< std::endl;
#endif
				addCursorMessage(tcur);				
				tuioCursor++;
			}
		}
		sendCursorBundle(++currentFrame);
	} else if (periodic_update) {
		long milli = timeGetTime();
		long dt = milli - lastCursorUpdate;
		if ( dt > alive_update_interval ) {
			lastCursorUpdate = milli;
			startCursorBundle();
			sendCursorBundle(++currentFrame);
		}
	}
}

void TuioUdpServer::sendEmptyCursorBundle() {
	oscPacket->Clear();	
	(*oscPacket) << osc::BeginBundleImmediate;
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive" << osc::EndMessage;	
	if ( Verbose > 2 ) {
		std::cout << "/tuio/25Dcur alive" << std::endl;
	}
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << -1 << osc::EndMessage;
	(*oscPacket) << osc::EndBundle;
	socket->Send( oscPacket->Data(), oscPacket->Size() );
	if ( Verbose > 2 ) {
		std::cout << "/tuio/25Dcur fseq -1" << std::endl;
	}
}

void TuioUdpServer::startCursorBundle() {	
	oscPacket->Clear();	
	(*oscPacket) << osc::BeginBundleImmediate;
	
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive";
	if ( Verbose > 2 ) {
		std::cout << "/tuio/25Dcur alive";
	}
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if ( Verbose > 2 ) {
			std::cout << " " << (int32)((*tuioCursor)->getSessionID());	
		}
		(*oscPacket) << (int32)((*tuioCursor)->getSessionID());	
	}
	(*oscPacket) << osc::EndMessage;	
	if ( Verbose > 2 ) {
		std::cout << std::endl;
	}
}

bool isZero(float f) {
	if (f < 0.0000001 && f > -0.0000001)
		return true;
	else
		return false;
}

void TuioUdpServer::addCursorMessage(TuioCursor *tcur) {

	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "set";
	float x = tcur->getX();
	float y = tcur->getY();
	 (*oscPacket) << (int32)(tcur->getSessionID()) << x << y << tcur->getForce();
	 (*oscPacket) << tcur->getXSpeed() << tcur->getYSpeed() << tcur->getForceSpeed() << tcur->getMotionAccel() ;	
	 (*oscPacket) << osc::EndMessage;
	if ( Verbose ) {
		std::cout << "/tuio/25Dcur set"
			<< " " << (int32)(tcur->getSessionID()) << " " << x << " " << y << " " << tcur->getForce()
			<< " " << tcur->getXSpeed() << " " << tcur->getYSpeed() << " " << tcur->getForceSpeed() << " " << tcur->getMotionAccel()
			<< std::endl;
#if 0
		if (isZero(tcur->getXSpeed() ) && isZero(tcur->getYSpeed() ) && isZero(tcur->getForceSpeed() ) && isZero(tcur->getMotionAccel() )) {
			static TuioCursor *lastbug = NULL;
			std::cout << "HEY!!  BUG??  tcur=" << (long long)(tcur) << "  length=" << cursorList.size() << std::endl;
			for (std::list<TuioCursor*>::iterator t = cursorList.begin(); t != cursorList.end(); t++) {
				TuioCursor *tcur = (*t);
				std::cout << "tcur=" << (long long)tcur << "  sessionid=" << (int32)((*t)->getSessionID()) << std::endl;
			}
			if (tcur == lastbug) {
				std::cout << "HEY!!  REPEATED CURSOR BUG??  tcur=" << (long long)(tcur) << std::endl;
			}
			lastbug = tcur;
		}
#endif
	}
}

void TuioUdpServer::sendCursorBundle(long fseq) {
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << (int32)fseq << osc::EndMessage;
	(*oscPacket) << osc::EndBundle;
	socket->Send( oscPacket->Data(), oscPacket->Size() );
	if ( Verbose > 2 ) {
		std::cout << "/tuio/25Dcur fseq " << (int32)fseq << std::endl;
	}
}
