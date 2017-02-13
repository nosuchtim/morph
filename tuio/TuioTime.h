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

#pragma once

#ifndef WIN32
#include <pthread.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

#define MSEC_SECOND 1000
#define USEC_SECOND 1000000
#define USEC_MILLISECOND 1000

namespace TUIO {
	
	class TuioTime {
		
	private:
		long milliseconds;
		static long start_milliseconds;
		
	public:

		TuioTime () {
			milliseconds = 0;
		};

		~TuioTime() {}
		
		TuioTime (long milli) {
			milliseconds = milli;
		};
		
		TuioTime operator+(long milli) {
			long m = milliseconds + milli;
			return TuioTime(m);
		};
		
		TuioTime operator+(TuioTime ttime) {
			long m = milliseconds + ttime.getMilliSeconds();
			return TuioTime(m);
		};

		TuioTime operator-(long us) {
			long m = milliseconds - us;
			return TuioTime(m);
		};

		TuioTime operator-(TuioTime ttime) {
			long m = milliseconds - ttime.getMilliSeconds();
			return TuioTime(m);
		};

		void operator=(TuioTime ttime) {
			milliseconds = ttime.getMilliSeconds();
		};
		
		bool operator==(TuioTime ttime) {
			return (milliseconds==(long)ttime.getMilliSeconds());
		};

		bool operator!=(TuioTime ttime) {
			return ((milliseconds!=ttime.getMilliSeconds()));
		};
		
		void reset() {
			milliseconds = 0;
		};
		
		long getMilliSeconds() {
			return milliseconds;
		};
		
		static void initSession();
#if 0
		static TuioTime getSessionTime();
#endif
		static TuioTime getStartTime();
		static TuioTime getSystemTime();
	};
};