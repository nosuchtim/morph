/******************************************************************************************
* SENSEL CONFIDENTIAL
*
* Copyright 2013-2017 Sensel, Inc
* All Rights Reserved.
*
* NOTICE:  All information contained herein is, and remains the property of Sensel, Inc.
* The intellectual and technical concepts contained herein are proprietary to Sensel, Inc.
* and may be covered by U.S. and Foreign Patents, patents in process, and are protected
* by trade secret or copyright law. Dissemination of this information or reproduction of
* this material is strictly forbidden unless prior written permission is obtained from
* Sensel, Inc.
******************************************************************************************/

#ifndef __SENSEL_LIB_H__
#define __SENSEL_LIB_H__

#ifdef _WIN32
#include <windows.h>
#define SENSEL_LIB __declspec(dllexport)
#else
#define SENSEL_LIB
#define WINAPI
#endif
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct
	{
		int decompressed_ncols;
		int decompressed_nrows;
	} sensel_decompress_info;

	SENSEL_LIB void WINAPI senselDecompressInit(unsigned char* metadata, sensel_decompress_info* info);

	SENSEL_LIB void WINAPI senselDecompressFrame(unsigned char* frame_data, int data_size, float* force_data, unsigned char* contact_id_data);
#ifdef __cplusplus
}
#endif

#endif // __SENSEL_LIB_H__
