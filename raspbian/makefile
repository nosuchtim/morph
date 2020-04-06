CC = g++
INC = -I/usr/include/ -I../src/oscpack -I../src/misc -I../src/tuio -I../src/morph
CFLAGS = -O -DOSC_HOST_LITTLE_ENDIAN $(INC) $(PROFILE) -Wno-psabi
SENSELLIBS = -L/usr/lib/ -lpthread -ldl -lsensel

SRCFILES = \
	../src/morph/main.cpp \
	../src/morph/morph.cpp \
	../src/tuio/TuioServer.cpp \
	../src/tuio/TuioTime.cpp \
	../src/tuio/TuioUdpServer.cpp \
	../src/misc/NosuchDebug.cpp \
	../src/misc/XGetopt.cpp \
	../src/misc/NosuchUtil.cpp \
	../src/oscpack/osc/OscTypes.cpp \
	../src/oscpack/osc/OscOutboundPacketStream.cpp \
	../src/oscpack/osc/OscReceivedElements.cpp \
	../src/oscpack/osc/OscPrintReceivedElements.cpp \
	../src/oscpack/ip/posix/UdpSocket.cpp \
	../src/oscpack/ip/posix/NetworkingUtils.cpp \
	../src/oscpack/ip/IpEndpointName.cpp \

morph : $(SRCFILES)
	$(CC) $(STATIC) $(CFLAGS) $(SRCFILES) $(SENSELLIBS) -o morph

clean :
	rm -f *.o

.cpp.o :
	$(CC) -c $(CFLAGS) $<
