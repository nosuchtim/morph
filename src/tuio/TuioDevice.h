#if 0
#pragma once

#include <list>

namespace TUIO {

	class TuioDevice {

	public:
		TuioDevice(TuioServer* s) : server(s) {
		};
		~TuioDevice() {
		};

		virtual void run() = 0;
		virtual bool init() = 0;

		TuioServer *server;
	};
};
#endif