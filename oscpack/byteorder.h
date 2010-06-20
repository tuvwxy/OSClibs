/*
 *  byteorder.h
 *
 *  Created by Toshiro Yamada on 04/02/10.
 *  Copyright 2010 Calit2, UCSD. All rights reserved.
 *
 *	Functions to convert endianness. Network is thought to be big-endian and
 *	the host is thought to be little-endian.
 */
#ifndef __BYTEORDER__
#define __BYTEORDER__

#include <arpa/inet.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
//	Network to Host 

// Following ntohll() and htonll() code snipits were taken from
// http://www.codeproject.com/KB/cpp/endianness.aspx?msg=1661457
#define ntohll(x) (((int64_t)(ntohl((int32_t)((x << 32) >> 32))) << 32) | \
						(uint32_t)ntohl(((int32_t)(x >> 32)))) //By Runner
//#define ntohf(x) (float)ntohl(x)
inline double ntohf(int32_t x) { x = ntohl(x); return *(float*)&x; }
//#define ntohd(x) (double)ntohll(x)
inline double ntohd(int64_t x) { return (double)ntohll(x); }


//	Host to Network

#define htonll(x) ntohll(x)
//#define htonf(x) (int32_t)htonl(*(int32_t*)&x)
inline int32_t htonf(float x) { return (int32_t)htonl(*(int32_t*)&x); }
//#define htond(x) (int64_t)htonll(*(int64_t*)&x)
inline int64_t htond(double x) { return (int64_t)htonll(*(int64_t*)&x); }

#ifdef __cplusplus
}
#endif

#endif // __BYTEORDER__