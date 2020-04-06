# morph
A TUIO 3D server for the Sensel Morph

The code here only runs on Windows.

In order to use this code, you need to install the Sensel libraries, which are
found in this GitHub repository: https://github.com/sensel/sensel-api .
The directory https://github.com/sensel/sensel-api/blob/master/sensel-install
contains installers, and you can install the libraries on Windows by executing this file:
https://github.com/sensel/sensel-api/blob/master/sensel-install/SenselLibWin.exe

====================================================================

TUIO3D_* is a collection of Github repositories with code that takes input from 3D devices
(the ancient FingerWorks iGesture and the new Sensel Morph) and sends out TUIO messages to transmit 3-Dimensional cursor information.  See http://tuio.org for information about TUIO.

These are the Github repositories:

   https://github.com/nosuchtim/TUIO3D_Sensel

   https://github.com/nosuchtim/TUIO3D_FingerWorks

   https://github.com/nosuchtim/TUIO3D_oscutil

All of this stuff only works on Windows.

Repo TUIO3D_Sensel is for the Morph, and it compiles a program named "tuio3d_morph.exe" using VS2015.

Repo TUIO3D_FingerWorks is for the iGesture, and compiles a program named "tuio3d_igesture.exe". It only compiles with VS2013.

Repo TUIO3D_oscutil is an OSC utility that compiles a program named "oscutil.exe".  It uses Python, so it needs a bunch of auxilary files to work, and they are all in one directory - "dist".  To recompile it, you use "nmake", not Visual Studio.  The github repo contains a "dist" directory with it pre-compiled, so you don't need to compile it.

Usage of the the two tuio3d_*.exe programs is identical, here's how you invoke it to do TUIO:

      tuio3d_morph -v -h 127.0.0.1

The -v makes it verbose, so it will show you the messages that it's sending out.  The -h argument is the host to which the OSC messages are sent.  The default port is 3333; if you want to use a different port, specify it by using {port}@{hostname} for the -h argument.

After invoking that program, it will wait (forever until killed) for input from the Morph and send it out as TUIO over OSC.  It uses the /tuio/25Dcur profile for the messages.

To see that it's working properly, you can use the oscutil.exe utility (from the TUIO3D_oscutil repo) like this:

     oscutil listen 3333

You can also use the {port}@{hostname} syntax to specify the port and IP address on which to listen.

To send TUIO between machines, use the external IP address for the host, not 127.0.0.1.

These programs can also send 3D cursor information using shared memory, which in my experience works much faster and smoother.  If you're interested in that, email me@timthompson.com.

====================================================================
