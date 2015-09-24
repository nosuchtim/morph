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


#ifndef SENSEL_REGISTER_MAP_H
#define SENSEL_REGISTER_MAP_H

#define SENSEL_REG_MAGIC                                    0x00
#define SENSEL_REG_FW_PROTOCOL_VERSION                      0x06
#define SENSEL_REG_FW_VERSION_MAJOR                         0x07
#define SENSEL_REG_FW_VERSION_MINOR                         0x08
#define SENSEL_REG_FW_VERSION_BUILD                         0x09
#define SENSEL_REG_FW_VERSION_RELEASE                       0x0B
#define SENSEL_REG_DEVICE_ID                                0x0C
#define SENSEL_REG_DEVICE_REVISION                          0x0E
#define SENSEL_REG_DEVICE_SERIAL_NUMBER                     0x0F
#define SENSEL_REG_SENSOR_ACTIVE_AREA_WIDTH_UM              0x14
#define SENSEL_REG_SENSOR_ACTIVE_AREA_HEIGHT_UM             0x18
#define SENSEL_REG_SCAN_FRAME_RATE                          0x20
#define SENSEL_REG_SCAN_CONTENT_CONTROL                     0x24
#define SENSEL_REG_SCAN_ENABLED                             0x25
#define SENSEL_REG_SCAN_READ_FRAME                          0x26
#define SENSEL_REG_CONTACTS_MAX_COUNT                       0x40
#define SENSEL_REG_ACCEL_X                                  0x60
#define SENSEL_REG_ACCEL_Y                                  0x62
#define SENSEL_REG_ACCEL_Z                                  0x64
#define SENSEL_REG_BATTERY_STATUS                           0x70
#define SENSEL_REG_BATTERY_PERCENTAGE                       0x71
#define SENSEL_REG_POWER_BUTTON_PRESSED                     0x72
#define SENSEL_REG_LED_BRIGHTNESS                           0x80
#define SENSEL_REG_SOFT_RESET                               0xE0
#define SENSEL_REG_ERROR_CODE                               0xEC
#define SENSEL_REG_BATTERY_VOLTAGE_MV                       0xFE

#endif
