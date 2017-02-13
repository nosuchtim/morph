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
		virtual void pressed(float x, float y, int uid, int id, float force) = 0;
		virtual void released(float x, float y, int uid, int id, float force) = 0;
		virtual void dragged(float x, float y, int uid, int id, float force) = 0;

		// int device_multiplier;
		// int initial_session_id;
		TuioServer *server;
	};
};