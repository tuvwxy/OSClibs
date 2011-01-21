/******************************************************************************
 *  oscraw
 *
 *  Copyright (C) 2010-2011 The Regents of the University of California. 
 *  All Rights Reserved.
 *
 *  Sonic Arts Research and Development Group
 *  California Institute for Telocommunications and Information Technology
 *  University of California,
 *  La Jolla, CA 92093
 *
 *  Commercial use of this program without express permission of the
 *  University of California, San Diego, is strictly prohibited. Information
 *  about usage and redistribution, and a disclaimer of all warrenties are
 *  available in the Copyright file provided with this code.
 *
 *  Author: Toshiro Yamada
 *  Contact: toyamada [at] ucsd.edu
 *
 ******************************************************************************/
#ifndef __BYTEORDER__
#define __BYTEORDER__

#include <arpa/inet.h>
#include <stdint.h>

//	Network to Host 

// Following ntohll() and htonll() code snipits were taken from
// http://www.codeproject.com/KB/cpp/endianness.aspx?msg=1661457
#define ntohll(x) (((int64_t)(ntohl((int32_t)((x << 32) >> 32))) << 32) | \
						(uint32_t)ntohl(((int32_t)(x >> 32)))) //By Runner
//#define ntohf(x) *(float*)&(ntohl(x))
inline double ntohf(int32_t x) { x = ntohl(x); return *(float*)&x; }
//#define ntohd(x) (double)ntohll(x)
inline double ntohd(int64_t x) { return (double)ntohll(x); }


//	Host to Network

#define htonll(x) ntohll(x)
//#define htonf(x) (int32_t)htonl(*(int32_t*)&x)
inline int32_t htonf(float x) { return (int32_t)htonl(*(int32_t*)&x); }
//#define htond(x) (int64_t)htonll(*(int64_t*)&x)
inline int64_t htond(double x) { return (int64_t)htonll(*(int64_t*)&x); }


#endif // __BYTEORDER__
