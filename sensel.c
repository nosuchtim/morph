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

#include "sensel.h"
#include "sensel_serial.h"
#include "sensel_register_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifndef WIN32
// TODO: These includes shouldn't be necessary once linux-specific stuff is moved into sensel_serial.h
#include <unistd.h>
#include <termios.h>
#endif

#define PT_FRAME              1
#define PT_READ_ACK           6
#define PT_WRITE_ACK          10

bool _readReg(uint8 reg, uint8 size, uint8 *buf);
bool _writeReg(uint8 reg, uint8 size, uint8 *buf);

static sensel_protocol_cmd read_cmd = { (DEFAULT_BOARD_ADDR | (1 << 7)), 0x00, 0x00, 0x00};
static sensel_protocol_cmd write_cmd = {DEFAULT_BOARD_ADDR, 0x00, 0x00, 0x00};
static sensel_serial_data serial_data;

#define FRAME_BUFFER_INITIAL_CAPACITY 256
static uint8* frame_buffer = NULL;
static int frame_buffer_capacity = 0;

static int    sensor_max_x;
static int    sensor_max_y;
static float  sensor_width_mm;
static float  sensor_height_mm;
static float  sensor_x_to_mm_factor;
static float  sensor_y_to_mm_factor;

contact_raw_t contacts_raw[MAX_CONTACTS];

bool _frameBufferEnsureCapacity(int capacity)
{
  // If the read buffer is too small, make it bigger
  if(frame_buffer_capacity < capacity)
  {
    // We allocate a buffer twice as big as what we need to avoid
    // very numerous re-allocation. By allocating a buffer
    // twice as big as what is needed, we will be able to accomodate
    // small increases in the data size, and the maximum number of times
    // we'll need to re-allocate is ln2(max_buffer_size)
    frame_buffer_capacity = capacity*2;
    frame_buffer = (uint8*)realloc(frame_buffer, frame_buffer_capacity);

    if(frame_buffer == NULL)
    {
      printf("Unable to allocate temporary buffer!\n");
      return false;
    }
  }

  return true;
}

bool _readReg(uint8 reg, uint8 size, uint8 *buf)
{
  uint8 ack;
  uint8 resp_checksum; uint16 resp_size;
  uint8 checksum = 0;
  int i;

  read_cmd.reg = reg;
  read_cmd.size = size;

  if(!senselSerialWrite(&serial_data, (uint8 *)&read_cmd, 3))
    return false;

  //usleep(50000); // TODO: Test if this is necessary in Linux

  if(!senselSerialReadBytes(&serial_data, (uint8 *)&ack, 1))
    return false;

  if(ack != PT_READ_ACK)
  {
    return false;
  }

  if(!senselSerialReadBytes(&serial_data, (uint8*)&resp_size, 2))
    return false;

  if(resp_size != size)
    return false;

  if(!senselSerialReadBytes(&serial_data, buf, size))
    return false;

  if(!senselSerialReadBytes(&serial_data, &resp_checksum, 1))
    return false;

  for(i = 0; i < size; i++)
    checksum += buf[i];

  //printf("Checksums: (%d == %d)\n", checksum, resp_checksum);

  return (checksum == resp_checksum);
}

bool _writeReg(uint8 reg, uint8 size, uint8 *buf)
{
  uint8 ack; 
  uint8 checksum = 0;
  int i;

  write_cmd.reg = reg;
  write_cmd.size = size;

  for(i = 0; i < size; i++)
    checksum += buf[i];

  //Send write header
  if(!senselSerialWrite(&serial_data, (uint8 *)&write_cmd, 3))
    return false;

  //Send data
  if(!senselSerialWrite(&serial_data, buf, size))
    return false;

  //Send checksum
  if(!senselSerialWrite(&serial_data, &checksum, 1))
    return false;

  if(!senselSerialReadBytes(&serial_data, &ack, 1))
    return false;

  return (ack == PT_WRITE_ACK);
}

bool _senselReadContactFrame()
{
  uint16 payload_size;
  uint8 checksum;
  uint8 received_checksum;

  if(!senselSerialReadBytes(&serial_data, (uint8 *)&payload_size, 2))
  {
    printf("SENSEL ERROR: Unable to read packet size\n");
    return false;
  }

  // Allocate enough space for the size, the data and the checksum
  // Note: This may reallocate the buffer so the pointer to it may change
  if(!_frameBufferEnsureCapacity(((int)payload_size)+3))
  {
    printf("SENSEL ERROR: Unable to allocate buffer\n");
    return false;
  }

  if(!senselSerialReadBytes(&serial_data, frame_buffer, payload_size + 1)) //read checksum as well
  {
    printf("SENSEL ERROR: Unable to read frame!\n");
    return false;
  }

  checksum = 0;
  for(int i = 0; i < payload_size; i++)
  {
    checksum += frame_buffer[i];
  }

  received_checksum = frame_buffer[payload_size];
  if(checksum != received_checksum)
  {
    printf("SENSEL ERROR: Checksum failed! (%d != %d) Dumping the buffer.\n", checksum, received_checksum);
    return false;
  }

  return true;
}


int senselReadContacts(contact_t * contacts)
{
  read_cmd.reg = SENSEL_REG_SCAN_READ_FRAME;
  read_cmd.size = 0;
  senselSerialWrite(&serial_data, (uint8 *)&read_cmd, 3);

  uint8 ack;
  if(!senselSerialReadBytes(&serial_data, &ack, 1))
  {
    printf("Failed to receive ack from sensor\n");
    return false;
  }

  if(ack == PT_FRAME) // Non-buffered frame
  {
    if(!_senselReadContactFrame()) //Read contact frame into frame_buffer
      return false;
  }
  else
  {
    printf("SENSEL ERROR: Received %d when expecting PT_FRAME.\n", ack);
    return false;
  }

  //copy from frame_buffer into contacts
  int num_contacts = frame_buffer[2];
  int contact_buffer_size = num_contacts * sizeof(contact_raw_t);

  memcpy(contacts_raw, &(frame_buffer[3]), contact_buffer_size);

  for(int i = 0; i < num_contacts; i++)
  {
    contacts[i].total_force =         contacts_raw[i].total_force;
    contacts[i].uid =                 contacts_raw[i].uid;
    contacts[i].area =                contacts_raw[i].area;
    contacts[i].x_pos_mm =            contacts_raw[i].x_pos * sensor_x_to_mm_factor;
    contacts[i].y_pos_mm =            contacts_raw[i].y_pos * sensor_y_to_mm_factor;
    contacts[i].dx =                  contacts_raw[i].dx;
    contacts[i].dy =                  contacts_raw[i].dy;
    contacts[i].orientation_degrees = contacts_raw[i].orientation / 256.0f;
    contacts[i].major_axis_mm =       contacts_raw[i].major_axis * sensor_x_to_mm_factor;
    contacts[i].minor_axis_mm =       contacts_raw[i].minor_axis * sensor_x_to_mm_factor;
    contacts[i].peak_x =              contacts_raw[i].peak_x;
    contacts[i].peak_y =              contacts_raw[i].peak_y;
    contacts[i].id =                  contacts_raw[i].id;
    contacts[i].type =                contacts_raw[i].type;
  }
  
  return num_contacts;
}

bool senselStartScanning()
{
  uint8 val = 1;
  return _writeReg(SENSEL_REG_SCAN_ENABLED, 1, &val);
}

bool senselStopScanning(void)
{
  uint8 val = 0;
  return _writeReg(SENSEL_REG_SCAN_ENABLED, 1, &val);
}

bool senselSetFrameContentControl(uint8 content)
{
  return _writeReg(SENSEL_REG_SCAN_CONTENT_CONTROL, 1, &content);
}

bool senselOpenConnection(char* com_port)
{
  if(senselSerialOpen(&serial_data, com_port))
  {
    frame_buffer = (uint8*)malloc(FRAME_BUFFER_INITIAL_CAPACITY*sizeof(uint8));
    frame_buffer_capacity = FRAME_BUFFER_INITIAL_CAPACITY*sizeof(uint8);

    uint8 buf[2];

    //Read in max X and max Y values
    _readReg(0x10, 2, buf);
    sensor_max_x = (256 * (buf[0] - 1));
    sensor_max_y = (256 * (buf[1] - 1));

    uint32 sensor_width_um;
    uint32 sensor_height_um;

    //Read in active area dimensions
    _readReg(SENSEL_REG_SENSOR_ACTIVE_AREA_WIDTH_UM,  4, (uint8 *)&sensor_width_um);
    _readReg(SENSEL_REG_SENSOR_ACTIVE_AREA_HEIGHT_UM, 4, (uint8 *)&sensor_height_um);

    sensor_width_mm = sensor_width_um / 1000.0;
    sensor_height_mm = sensor_height_um / 1000.0;

    sensor_x_to_mm_factor = sensor_width_mm / sensor_max_x;
    sensor_y_to_mm_factor = sensor_height_mm / sensor_max_y;

    return true;
  }

  return false;
}

void senselSetLEDBrightness(int idx, uint8 brightness)
{
  _writeReg(SENSEL_REG_LED_BRIGHTNESS + idx, 1, &brightness);
}

void senselSetLEDBrightnessAll(uint8 brightness)
{
  int i;
  uint8 led_buf[16];

  for(i = 0; i < 16; i++)
    led_buf[i] = brightness;

  _writeReg(SENSEL_REG_LED_BRIGHTNESS, 16, led_buf);
}

  // Returns (x,y,z) acceleration in G's using the following coordinate system:
  //
  //          ---------------------------
  //        /   Z /\  _                 /
  //       /       |  /| Y             /
  //      /        | /                /
  //     /         |/                /
  //    /           -----> X        /
  //   /                           /
  //   ----------------------------
  //
  // Assumes accelerometer is configured to the default +/- 2G range
bool senselReadAccelerometerData(accel_data_t *accel_data)
{
  accel_data_raw_t raw_data;

  // Read accelerometer data bytes for X, Y and Z
  if(!_readReg(SENSEL_REG_ACCEL_X, sizeof(accel_data_raw_t), (uint8 *)&raw_data))
    return false;

  // Rescale to G's (at a range of +/- 2G, accelerometer returns 0x4000 for 1G acceleration)

  accel_data->x = ((float)raw_data.x) / 0x4000;
  accel_data->y = ((float)raw_data.y) / 0x4000;    
  accel_data->z = ((float)raw_data.z) / 0x4000;
    
  return true;
}

void senselCloseConnection()
{
  free(frame_buffer);
  senselSerialClose(&serial_data);
}
