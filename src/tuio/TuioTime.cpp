/*
 TUIO C++ Library - part of the reacTIVision project
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

#include "TuioTime.h"
#include <math.h>

using namespace TUIO;
	
long TuioTime::start_milliseconds = 0;

void TuioTime::initSession() {
	TuioTime startTime = TuioTime::getSystemTime();
	start_milliseconds = startTime.getMilliSeconds();
}

TuioTime TuioTime::getStartTime() {
	return TuioTime(start_milliseconds);
}

TuioTime TuioTime::getSystemTime() {

#ifdef _WIN32
    TuioTime systemTime(timeGetTime());
    return systemTime;
#else
	long ms;
	time_t s;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	s = spec.tv_sec;
	ms = round(spec.tv_nsec / 1.0e6);

	TuioTime systemTime( ms );
    return systemTime;
#endif
}
