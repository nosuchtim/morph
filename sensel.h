/**************************************************************************
 * Copyright 2015 Sensel, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************/
#ifndef SENSEL_H
#define SENSEL_H

#include "sensel_types.h"

#define DEFAULT_BOARD_ADDR 0x01

#define MAX_CONTACTS 16 // // TODO: Read this from device

#define SENSEL_EVENT_CONTACT_INVALID 0
#define SENSEL_EVENT_CONTACT_START   1
#define SENSEL_EVENT_CONTACT_MOVE    2
#define SENSEL_EVENT_CONTACT_END     3

#define SENSEL_FRAME_CONTACTS_FLAG  0x04

#define SENSEL_MAGIC      "S3NS31"
#define SENSEL_MAGIC_LEN  6

#ifdef __cplusplus
extern "C" {
#endif

  #ifdef _MSC_VER
    #define PACK( __Declaration__, __Name__ ) __pragma( pack(push, 1) )     \
                                              __Declaration__ __Name__ __pragma( pack(pop) )
  #else
    #define PACK( __Declaration__, __Name__ ) __Declaration__ __attribute__((__packed__)) __Name__
  #endif

  PACK(
  typedef struct
  {
    uint8 r_w_addr;
    uint8 reg;
    uint8 size;
    uint8 padding;
  }, 
  sensel_protocol_cmd);

  PACK(
  typedef struct
  {
    uint8  fw_protocol_version;
    uint8  fw_version_major;
    uint8  fw_version_minor;
    uint16 fw_version_build;
    uint8  fw_version_release;
    uint16 device_id;
    uint8  device_revision;
  },
  sensel_device_info);

  typedef struct
  {
    #if WIN32
      void* serial_handle;
    #else
      int   serial_fd;
    #endif
  } sensel_serial_data;

  ////////////////////////////////////////////////////////////////////////////////
  // Type declarations
  typedef uint16 pressure_t;
  typedef uint16 force_t;
  typedef uint8 grid_coord_t;
  typedef uint8 label_t;
  typedef uint8 blobid_t;
  typedef uint32 uid_t;
  typedef uint8 contact_type_t;
  typedef int16 vel_t;


  // This type is for storing contact information
  // NOTE: I use unsigned types for some of these fields. I may want to consider signed types for faster processing.

  PACK(
  typedef struct
  {
    uint32 total_force;
    uid_t  uid;
    uint32 area;  // area multiplied by 65536
    uint16 x_pos; // x position multiplied by 256
    uint16 y_pos; // y position multiplied by 256
    vel_t dx; // change in x from last frame
    vel_t dy; // change in y from last frame
    int16 orientation; // angle from -90 to 90 multiplied by 256
    uint16 major_axis; // length of the major axis multiplied by 256
    uint16 minor_axis; // length of the minor axis multiplied by 256
    grid_coord_t peak_x;
    grid_coord_t peak_y;
    label_t id; // TODO: The type of this should be something like contact_id
    contact_type_t type;
  },
  contact_t);

  PACK(
  typedef struct
  {
    uint8 content_bit_mask;
    uint32 lost_frame_count;
  },
  frame_info_t);

  PACK(
  typedef struct
  {
    int16 x;
    int16 y;
    int16 z;
  },
  accel_data_t);


  bool senselOpenConnection(char* com_port); // Pass in NULL to do auto-detection
  bool senselSetFrameContentControl(uint8 content);
  bool senselStartScanning();
  int  senselReadContacts(contact_t *contacts);
  bool senselStopScanning();
  void senselCloseConnection();


#ifdef __cplusplus
}
#endif

#endif //SENSEL_H
