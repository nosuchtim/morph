#Sensel C/C++ API

This API allows users to communicate with a Sensel device through C or C++. This API should be cross-platform, and work across Windows, Mac, and Linux. If you find any incompatibilities, please submit a bug report through Github.

##Setup
Clone this Github project and move all of the sensel* files into your project to build against. Note: if you are on windows, you do not need sensel_serial_linux.c and if you are on Linux/Mac, you do not need sensel_serial_win.cpp.

Make sure to look at the sensel_example_read_contacts project under the examples folder to see how to setup your project. On Linux/Mac, simply navigate to this project and type in "make" to build the executable. On Windows, you will want to open up the read_contacts_win.sln file in Visual Studio and build the project from there.

##Usage
Here's a high-level view of how to use this API:

First, we need to import Sensel:

```c++
#include "sensel.h"
```

Next, we need to properly setup the sensor. We first call `senselOpenConnection()`, which returns true if we successfully connect to a Sensel device. If we connect to a sensor, we need to tell the sensor to send us contacts. We use the method `senselSetFrameContentControl()`, and pass in the `SENSEL_FRAME_CONTACTS_FLAG` constant. After this, we tell the sensor to start scanning by calling `senselStartScanning()`:

```c++
//Open Sensor
bool sensel_sensor_opened = senselOpenConnection(0);

if(!sensel_sensor_opened)
{
  printf("Unable to open Sensel sensor!\n");
  return -1; //exit
}

//Enable contact sending
senselSetFrameContentControl(SENSEL_FRAME_CONTACTS_FLAG);

//Enable scanning
senselStartScanning();
```

Next, you can read out contacts in your program's main event loop with `senselReadContacts()`. This returns an array of contacts.

```c++
//Declare contacts array and variable to hold number of valid contacts
contact_t contacts[MAX_CONTACTS];
int n_contacts = 0;

while(true)
{
  n_contacts = senselReadContacts(contacts);
  if(n_contacts != 0) //Check for contacts
    //USE CONTACT DATA HERE
}
```

Before the applciation exits, make sure to cleanly close the connection to the sensor by calling `senselStopScanning()` and `senselCloseConnection()`

```c++
printf("Done looping, exiting...\n")

if(sensel_sensor_opened)
{
  senselStopScanning();
  senselCloseConnection();
}
```

##Examples

There is an example in this repository that you can use as a starting point for your project:

####sensel_example_read_contacts
This project opens up a Sensel device, reads out contact data, and prints it to the screen.
