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

#include <list>
#include <math.h>
#include "TuioPoint.h"
#include <iostream>

#define TUIO_ADDED 0
#define TUIO_ACCELERATING 1
#define TUIO_DECELERATING 2
#define TUIO_STOPPED 3
#define TUIO_REMOVED 4

namespace TUIO {
	
	/**
	 * The abstract TuioContainer class defines common attributes that apply to both subclasses {@link TuioObject} and {@link TuioCursor}.
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.4
	 */ 
	class TuioContainer: public TuioPoint {
		
	protected:
		long session_id;
		float x_speed;
		float y_speed;
		float motion_speed;
		float motion_accel;
		/**
		 * The force (or area) value.   Scaled so that 1.0 is a fairly high force.
		 */
		float force;
		float force_speed;		
		/**
		 * A List of TuioPoints containing all the previous positions of the TUIO component.
		 */ 
		std::list<TuioPoint> path;
		int state;
		
	public:
		/**
		 * This constructor takes the provided Session ID, X and Y coordinate 
		 * and assigs these values to the newly created TuioContainer.
		 *
		 * @param	si	the Session ID to assign
		 * @param	xp	the X coordinate to assign
		 * @param	yp	the Y coordinate to assign
		 */
		TuioContainer (long si, float xp, float yp):TuioPoint(xp,yp) {
			session_id = si;
			x_speed = 0.0f;
			y_speed = 0.0f;
			motion_speed = 0.0f;
			motion_accel = 0.0f;
			force = 0.0f;
			force_speed = 0.0f;
			TuioPoint p(xpos,ypos);
			path.push_back(p);
			
			state = TUIO_ADDED;
		};
		
		/**
		 * This constructor takes the atttibutes of the provided TuioContainer 
		 * and assigs these values to the newly created TuioContainer.
		 *
		 * @param	tcon	the TuioContainer to assign
		 */
		TuioContainer (TuioContainer *tcon):TuioPoint(tcon) {
			session_id = tcon->getSessionID();
			x_speed = 0.0f;
			y_speed = 0.0f;
			motion_speed = 0.0f;
			motion_accel = 0.0f;
			force = 0.0f;
			force_speed = 0.0f;
			TuioPoint p(xpos,ypos);
			path.push_back(p);
			
			state = TUIO_ADDED;
		};
		
		/**
		 * The destructor is doing nothing in particular. 
		 */
		virtual ~TuioContainer(){};
		
		virtual void update (float xp, float yp) {
			// printf("UPDATE xy=%f,%f gettime=%ld\n",xp,yp,timeGetTime());
			TuioPoint lastPoint = path.back();
			TuioPoint::update(xp, yp);
			
			long dt = timeGetTime() - lastPoint.getStartTime().getMilliSeconds();
			if ( dt == 0 ) {
				printf("HEY, dt==0 in update!?\n");
			}
			float dx = xpos - lastPoint.getX();
			float dy = ypos - lastPoint.getY();
			float dist = sqrt(dx*dx+dy*dy);
			float last_motion_speed = motion_speed;
			
			x_speed = dx/dt;
			y_speed = dy/dt;
			motion_speed = dist/dt;
			motion_accel = (motion_speed - last_motion_speed)/dt;
			
			TuioPoint p(xpos,ypos);
			path.push_back(p);
			
			if (motion_accel>0) state = TUIO_ACCELERATING;
			else if (motion_accel<0) state = TUIO_DECELERATING;
			else state = TUIO_STOPPED;
		};

		
		/**
		 * This method is used to calculate the speed and acceleration values of
		 * TuioContainers with unchanged positions.
		 */
		virtual void stop() {
			update(xpos,ypos);
		};

		/**
		 * Assigns the provided X and Y coordinate, X and Y velocity and acceleration
		 * to the private TuioContainer attributes. The TuioTime time stamp remains unchanged.
		 *
		 * @param	xp	the X coordinate to assign
		 * @param	yp	the Y coordinate to assign
		 * @param	xs	the X velocity to assign
		 * @param	ys	the Y velocity to assign
		 * @param	ma	the acceleration to assign
		 */
		virtual void update (float xp, float yp, float xs, float ys, float ma) {
			TuioPoint::update(xp,yp);
			x_speed = xs;
			y_speed = ys;
			motion_speed = (float)sqrt(x_speed*x_speed+y_speed*y_speed);
			motion_accel = ma;
			
			path.pop_back();
			TuioPoint p(xpos,ypos);
			path.push_back(p);
			
			if (motion_accel>0) state = TUIO_ACCELERATING;
			else if (motion_accel<0) state = TUIO_DECELERATING;
			else state = TUIO_STOPPED;
		};
		
		/**
		 * Takes the atttibutes of the provided TuioContainer 
		 * and assigs these values to this TuioContainer.
		 * The TuioTime time stamp of this TuioContainer remains unchanged.
		 *
		 * @param	tcon	the TuioContainer to assign
		 */
		virtual void update (TuioContainer *tcon) {
			TuioPoint::update(tcon);
			x_speed = tcon->getXSpeed();
			y_speed =  tcon->getYSpeed();
			motion_speed =  tcon->getMotionSpeed();
			motion_accel = tcon->getMotionAccel();
			force = tcon->getForce();
			force_speed = tcon->getForceSpeed();
			
			TuioPoint p(xpos,ypos);
			path.push_back(p);
			
			if (motion_accel>0) state = TUIO_ACCELERATING;
			else if (motion_accel<0) state = TUIO_DECELERATING;
			else state = TUIO_STOPPED;
		};
		
		virtual void remove() {
			state = TUIO_REMOVED;
		}

		virtual long getSessionID() { 
			return session_id;
		};

		virtual float getXSpeed() { 
			return x_speed;
		};

		virtual float getYSpeed() { 
			return y_speed;
		};
		
		virtual TuioPoint getPosition() {
			TuioPoint p(xpos,ypos);
			return p;
		};
		
		virtual std::list<TuioPoint> getPath() {
			return path;
		};
		
		virtual float getMotionSpeed() {
			return motion_speed;
		};
		
		virtual float getMotionAccel() {
			return motion_accel;
		};
		
		virtual float getForce() {
			return force;
		};
		
		virtual float getForceSpeed() {
			return force_speed;
		};

		virtual void setForce(float f) {
			force = f;
		};
		
		virtual int getTuioState() { 
			return state;
		};	
		
		virtual bool isMoving() { 
			if ((state==TUIO_ACCELERATING) || (state==TUIO_DECELERATING)) return true;
			else return false;
		};
	};
};
