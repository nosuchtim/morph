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
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur alive ";
	}
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		(*fullPacket) << (int32)((*tuioCursor)->getSessionOrCursorID());	
		if ( verbose > 1 ) {
			std::cout << " " << (int32)((*tuioCursor)->getSessionOrCursorID());	
		}
	}
	(*fullPacket) << osc::EndMessage;	
	if ( verbose > 1 ) {
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
			if ( verbose > 1 ) {
				std::cout << "/tuio/25Dcur fseq -1" << std::endl;
			}

			// prepare the new cursor packet
			fullPacket->Clear();	
			(*fullPacket) << osc::BeginBundleImmediate;
			
			// add the cursor alive message
			(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive";
			if ( verbose > 1 ) {
				std::cout << "/tuio/25Dcur alive";
			}
			for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
				(*fullPacket) << (int32)((*tuioCursor)->getSessionOrCursorID());	
				if ( verbose > 1 ) {
					std::cout << " " << (int32)((*tuioCursor)->getSessionOrCursorID());	
				}
			}
			(*fullPacket) << osc::EndMessage;				
			if ( verbose > 1 ) {
				std::cout << std::endl;
			}
		}

		float x = (*tuioCursor)->getX();
		float y = (*tuioCursor)->getY();
		if (flipX) {
			x = 1.0f - x;
		}
		// For some reason that I'm not sure of, the y is already flipped, so this is reversed.
		if (!flipY) {
			y = 1.0f - y;
		}

		// add the actual cursor set message
		(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "set";
		(*fullPacket) << (int32)((*tuioCursor)->getSessionOrCursorID()) << x << y << (*tuioCursor)->getForce();
		(*fullPacket) << (*tuioCursor)->getXSpeed() << (*tuioCursor)->getYSpeed() << (*tuioCursor)->getForceSpeed() <<(*tuioCursor)->getMotionAccel();	
		(*fullPacket) << osc::EndMessage;	
		if ( verbose ) {
			std::cout << "/tuio/25Dcur set"
				<< " " << (int32)((*tuioCursor)->getSessionOrCursorID()) << " " << x << " " << y << " " << (*tuioCursor)->getForce()
				<< " " << (*tuioCursor)->getXSpeed() << " " << (*tuioCursor)->getYSpeed() << " " << (*tuioCursor)->getForceSpeed() << " " <<(*tuioCursor)->getMotionAccel()
				<< std::endl;
		}
	}
	
	// add the immediate fseq message and send the cursor packet
	(*fullPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << -1 << osc::EndMessage;
	(*fullPacket) << osc::EndBundle;
	socket->Send( fullPacket->Data(), fullPacket->Size() );
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur fseq -1" << std::endl;
	}
	
}

TuioUdpServer::TuioUdpServer(const char *host, int port, int alive_interval) {
	alive_update_interval = alive_interval;
	initialize(host,port);
}

void TuioUdpServer::initialize(const char *host, int port) {
	int size = IP_MTU_SIZE;
	if (size>MAX_UDP_SIZE) size = MAX_UDP_SIZE;
	else if (size<MIN_UDP_SIZE) size = MIN_UDP_SIZE;

	try {
		long unsigned int ip = GetHostByName(host);
		socket = new UdpTransmitSocket(IpEndpointName(ip, port));

		oscBuffer = new char[size];
		oscPacket = new osc::OutboundPacketStream(oscBuffer,size);
		fullBuffer = new char[size];
		fullPacket = new osc::OutboundPacketStream(fullBuffer,size);
	} catch (std::exception &e) { 
		std::cout << "could not create socket" << std::endl;
		socket = NULL;
	}
	
	currentFrame = sessionID = maxCursorID = -1;
	updateCursor = false;
	lastCursorUpdate = timeGetTime();
	
	sendEmptyCursorBundle();

	periodic_update = false;
	connected = true;
}

TuioUdpServer::~TuioUdpServer() {
	connected = false;

	sendEmptyCursorBundle();

	delete oscPacket;
	delete []oscBuffer;
	delete fullPacket;
	delete []fullBuffer;
	delete socket;
}

long TuioUdpServer::getFrameID() {
	return currentFrame;
}

void TuioUdpServer::initFrame() {
	currentFrame++;
}

void TuioUdpServer::commitFrame() {
	
	if(updateCursor) {
		startCursorBundle();
		for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
			
			// start a new packet if we exceed the packet capacity
			if ((oscPacket->Capacity()-oscPacket->Size())<CUR_MESSAGE_SIZE) {
				sendCursorBundle(currentFrame);
				startCursorBundle();
			}

			TuioCursor *tcur = (*tuioCursor);
			addCursorMessage(tcur);				
		}
		sendCursorBundle(currentFrame);
	} else if (!periodic_update) {
		long milli = timeGetTime();
		long dt = milli - lastCursorUpdate;
		if ( dt > alive_update_interval ) {
			lastCursorUpdate = milli;
			startCursorBundle();
			sendCursorBundle(currentFrame);
		}
	}
	updateCursor = false;
}

void TuioUdpServer::sendEmptyCursorBundle() {
	oscPacket->Clear();	
	(*oscPacket) << osc::BeginBundleImmediate;
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive" << osc::EndMessage;	
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur alive" << std::endl;
	}
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << -1 << osc::EndMessage;
	(*oscPacket) << osc::EndBundle;
	socket->Send( oscPacket->Data(), oscPacket->Size() );
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur fseq -1" << std::endl;
	}
}

void TuioUdpServer::startCursorBundle() {	
	oscPacket->Clear();	
	(*oscPacket) << osc::BeginBundleImmediate;
	
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "alive";
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur alive";
	}
	for (std::list<TuioCursor*>::iterator tuioCursor = cursorList.begin(); tuioCursor!=cursorList.end(); tuioCursor++) {
		if ( verbose > 1 ) {
			std::cout << " " << (int32)((*tuioCursor)->getSessionOrCursorID());	
		}
		(*oscPacket) << (int32)((*tuioCursor)->getSessionOrCursorID());	
	}
	(*oscPacket) << osc::EndMessage;	
	if ( verbose > 1 ) {
		std::cout << std::endl;
	}
}

void TuioUdpServer::addCursorMessage(TuioCursor *tcur) {

	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "set";
	float x = tcur->getX();
	float y = tcur->getY();
	if (flipX) {
		x = 1.0 - x;
	}
	// For some reason that I'm not sure of, the y is already flipped, so this is reversed.
	if (!flipY) {
		y = 1.0 - y;
	}
	 (*oscPacket) << (int32)(tcur->getSessionOrCursorID()) << x << y << tcur->getForce();
	 (*oscPacket) << tcur->getXSpeed() << tcur->getYSpeed() << tcur->getForceSpeed() << tcur->getMotionAccel() ;	
	 (*oscPacket) << osc::EndMessage;
	if ( verbose ) {
		std::cout << "/tuio/25Dcur set"
			<< " " << (int32)(tcur->getSessionOrCursorID()) << " " << x << " " << y << " " << tcur->getForce()
			<< " " << tcur->getXSpeed() << " " << tcur->getYSpeed() << " " << tcur->getForceSpeed() << " " << tcur->getMotionAccel()
			<< std::endl;
	}
}

void TuioUdpServer::sendCursorBundle(long fseq) {
	(*oscPacket) << osc::BeginMessage( "/tuio/25Dcur") << "fseq" << (int32)fseq << osc::EndMessage;
	(*oscPacket) << osc::EndBundle;
	socket->Send( oscPacket->Data(), oscPacket->Size() );
	if ( verbose > 1 ) {
		std::cout << "/tuio/25Dcur fseq " << (int32)fseq << std::endl;
	}
}
