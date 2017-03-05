#include "NosuchUtil.h"
#include "NosuchException.h"
#include "TuioServer.h"
#include "TuioSharedMemServer.h"

TuioSharedMemServer::TuioSharedMemServer(const char* memname) {
	_sharedmem_outlines = setup_shmem_for_outlines(memname);
	if (_sharedmem_outlines == NULL) {
		NosuchErrorOutput("Unable to create shared memory!?");
		exit(1);
	}
}

MMTT_SharedMem*
TuioSharedMemServer::setup_shmem_for_outlines(const char* memname) {

	// Create Shared memory
	unsigned int headersize = sizeof(Outlines_SharedMemHeader);
	unsigned int outlinesize = NUM_BUFFS * MMTT_OUTLINES_MAX * sizeof(OutlineMem);
	unsigned int pointsize = NUM_BUFFS * MMTT_POINTS_MAX * sizeof(PointMem);
	unsigned int totalsize = headersize + outlinesize + pointsize;

	MMTT_SharedMem* mem = new MMTT_SharedMem(memname, totalsize); // figure out the size based on the OP

	MMTT_SharedMemError shmerr = mem->getErrorState();
	if (shmerr != MMTT_SHM_ERR_NONE) {
		NosuchErrorOutput("Error when creating SharedMem?? err=%d", shmerr);
		throw NosuchException("Error when creating SharedMem?? err=%d", shmerr);
		// return NULL;
	}

	mem->lock();
	void *data = mem->getMemory();
	if (!data) {
		NosuchDebug("NULL returned from getMemory of Shared Memory!?  Disabling shmem");
		return NULL;
	}
	Outlines_SharedMemHeader* h = (Outlines_SharedMemHeader*)data;
	h->xinit();

	mem->unlock();
	return mem;
}

void TuioSharedMemServer::update() {
	static int last_ncursors = -1;
	int ncursors = (int)(getTuioCursors()).size();
	if (ncursors != last_ncursors) {
		last_ncursors = ncursors;
	}
	shmem_lock_and_update_outlines();
}

void
TuioSharedMemServer::shmem_lock_and_update_outlines()
{
	if (_sharedmem_outlines == NULL)
		return;

	bool doupdate = true;

	_sharedmem_outlines->lock();

	void *data = _sharedmem_outlines->getMemory();
	if (!data) {
		DEBUGPRINT(("NULL returned from getMemory of Shared Memory!?  Disabling sharedmem_outlines"));
		_sharedmem_outlines->unlock();
		_sharedmem_outlines = NULL;
		return;
	}
	Outlines_SharedMemHeader* h = (Outlines_SharedMemHeader*)data;

	if (h->buff_being_constructed != BUFF_UNSET) {
		if (h->buff_to_display_next != BUFF_UNSET) {
			// Both buffers are full.  buff_being_constructed is the newer one,
			// and buff_to_display_next is the older one.
			// Swap them, so the newer one is going to be displayed next, and
			// we're going to build a new one in the older buffer.
			buff_index t = h->buff_to_display_next;
			h->buff_to_display_next = h->buff_being_constructed;
			h->buff_being_constructed = t;
			// Both buffers are still in use.
		}
		else {
			// The to_display_next one was used and is now UNSET,
			// so shift the being_constructed into it so it'll be displayed next,
			// and find an empty buffer for buff_being_constructed. 
			h->buff_to_display_next = h->buff_being_constructed;
			h->buff_being_constructed = h->grab_unused_buffer();
		}
	}
	else {
		h->buff_being_constructed = h->grab_unused_buffer();
	}

	if (h->buff_being_constructed == BUFF_UNSET) {
		DEBUGPRINT(("Hey! No buffer available in shmem_update_outlines!"));
		doupdate = false;
	}

	// XXX - Do we need to have the memory locked when
	// updating the outlines/etc?  Not sure.  Might only be needed when changing the
	// buff_* pointers?

	shmem_update_outlines(h);

	_sharedmem_outlines->unlock();

	if (!doupdate) {
		DEBUGPRINT(("NOT UPDATING in shmem_update_outlines because !doupdate"));
		return;
	}

	_sharedmem_outlines->lock();

	if (h->buff_to_display_next == BUFF_UNSET) {
		h->buff_to_display_next = h->buff_being_constructed;
		h->buff_being_constructed = BUFF_UNSET;
	}
	else {
	}

	// The h->buff_inuse value for the buffer remains set to true.
	if (h->buff_inuse[h->buff_to_display_next] != true) {
		DEBUGPRINT(("UNEXPECTED in shmem_update_outlines!! h->buff_inuse isn't true?"));
	}

	_sharedmem_outlines->unlock();
}

void
TuioSharedMemServer::shmem_update_outlines(Outlines_SharedMemHeader* h)
{
	buff_index buff = h->buff_being_constructed;
	h->clear_lists(buff);

	// int ncursors = (int)(getTuioCursors()).size();
	for (std::list<TuioCursor*>::iterator iter = cursorList.begin(); iter != cursorList.end(); iter++) {
		TuioCursor* c = *iter;
		int rid = 1;
		float area = 0.5;
		int npoints = 0;
		// int sid = initial_session_id + c->getSessionOrCursorID();
		int sid = c->getSessionOrCursorID();
		h->addOutline(buff, rid, sid, c->getX(), c->getY(), c->getForce(), area, npoints);
	}

	h->lastUpdateTime = timeGetTime();
	h->active = 1;
}