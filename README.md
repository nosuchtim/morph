# morph
A TUIO 3D server for the Sensel Morph

The code here runs on Windows and the Raspberry Pi.

In order to use this code, you need to install the Sensel libraries, which are
found in this GitHub repository: https://github.com/sensel/sensel-api .
The directory https://github.com/sensel/sensel-api/blob/master/sensel-install
contains installers, and you can install the libraries on Windows by executing this file:
https://github.com/sensel/sensel-api/blob/master/sensel-install/SenselLibWin.exe

====================================================================

```
usage: morph [-v] [-V #] [-a #] [-h port@host] [-l]
  -v               Verbosity level = 1
  -V #             Verbosity level = #
  -a #             Number of milliseconds between alive messages
  -f #             force scale factor
  -h {port}@{host} Port and hostname for TUIO output
  -l               List all Morphs and their serial numbers
  -s {serial#}:{initialsid}        Serial# and initial session id
```

The -v makes it verbose, so it will show you the messages that it's sending out.  The -h argument is the host to which the OSC messages are sent.  The default port is 3333; if you want to use a different port, specify it by using {port}@{hostname} for the -h argument.

After invoking that program, it will wait (forever until killed) for input from the Morph and send it out as TUIO over OSC.  It uses the /tuio/25Dcur profile for the messages.

====================================================================
