/*
 *  oscsend.c
 *
 *  Created by Toshiro Yamada on 04/05/10.
 *  Copyright 2010 Calit2, UCSD. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define OSCSEND "oscsend"
const char usage[] = 
"usage: oscsend ip port tcp|udp /osc/address -type value ...\n" \
"    Support type/value pairs:\n" \
"        -i      32-bit integer\n" \
"        -h      64-bit integer\n" \
"        -f      32-bit floating point\n" \
"        -d      64-bit double floating point\n" \
"        -c      a single character\n" \
"        -s      string (array of characters)\n" \
"                string should be surround by \"\" (quotes)\n" \
"        -T      True (no value)\n" \
"        -F      False (no value)\n" \
"        -N      Nil (no value)\n" \
"        -I      Infinitum (no value)\n" \
"\n";

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


int32_t oscraw(uint8_t** buf, int argc, char* const argv[])
{
	char* net;
	char* osc_addr;
	int32_t osc_size = 0;
	int32_t addr_size = 0;
	int32_t type_size = 0;
	int32_t mess_size = 0;
	uint8_t* osc_data = NULL;
	uint8_t* addr_ptr = NULL;
	uint8_t* type_ptr = NULL;
	uint8_t* mess_ptr = NULL;
	int32_t bit32;
	int64_t bit64;
	union {
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		float f32;
		double f64;
	} atom;

	int i;
	int32_t len;
	char bad_mess = 0;
	
	// Check first argument
	net = (char*)argv[0];
	
	// Check OSC Address Pattern
	osc_addr = (char*)argv[1];
	addr_size = strlen(osc_addr);
	addr_size += (4 - addr_size % 4);
	
	// +1 for ',' of type specifier
	type_size = 1;
	
	// Iterate once to check OSC type and messages, and calculate size
	for (i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "-i") == 0) {
			i++;
			type_size++;
			mess_size += 4;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			i++;
			type_size++;
			mess_size += 8;
		}
		else if (strcmp(argv[i], "-f") == 0) {
			i++;
			type_size++;
			mess_size += 4;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			i++;
			type_size++;
			mess_size += 8;
		}
		else if (strcmp(argv[i], "-s") == 0) {
			i++;
			if (i >= argc) {
				bad_mess = 1;
				break;
			}
			
			type_size++;
			len = strlen(argv[i]);
			len += (4 - len % 4);
			mess_size += len;
		}
		else if (strcmp(argv[i], "-c") == 0) {
			i++;
			if (i >= argc) {
				bad_mess = 1;
				break;
			}
			
			type_size++;
			if (strlen(argv[i]) > 1) {
				bad_mess = 1;
				break;
			}
			mess_size += 4;
		}
		else if (strcmp(argv[i], "-T") == 0) {
			i++;
			type_size++;
		}
		else if (strcmp(argv[i], "-F") == 0) {
			i++;
			type_size++;
		}
		else if (strcmp(argv[i], "-N") == 0) {
			i++;
			type_size++;
		}
		else if (strcmp(argv[i], "-I") == 0) {
			i++;
			type_size++;
		}
		else {
			bad_mess = 1;
			break;
		}
		
		if (i >= argc) {
			bad_mess = 1;
			break;
		}
	}
	
	if (bad_mess) {
		printf("%s: Bad OSC Type or Message\n", OSCSEND);
		return 0;
	}
	
	type_size += (4 - type_size % 4);
	
	osc_size = addr_size + type_size + mess_size;
	
	// Create an array
	if (strcmp(net, "tcp") == 0) {
		*buf = (uint8_t*)calloc(1, osc_size+4);
		if (*buf == NULL) {
			fprintf(stderr, "%s: Critical memory error...\n", OSCSEND);
			return 0;
		}
		osc_data = *buf;
		bit32 = htonl(osc_size);
		memcpy(osc_data, (uint8_t*)&bit32, 4);
		osc_size += 4;
		addr_ptr = osc_data+4;
	}
	else {
		*buf = (uint8_t*)calloc(1, osc_size);
		if (*buf == NULL) {
			fprintf(stderr, "%s: Critical memory error...\n", OSCSEND);
			return 0;
		}
		osc_data = *buf;
		addr_ptr = osc_data;
	}
	
	// Copy OSC Address Pattern
	strcpy((char*)addr_ptr, osc_addr);
	
	type_ptr = addr_ptr + addr_size;
	mess_ptr = type_ptr + type_size;
	
	*(type_ptr++) = ',';
	
	// Iterate the arguments to fill the array
	for (i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "-i") == 0) {
			i++;
			*(type_ptr++) = 'i';
			if (sizeof(int) == 4) {
				bit32 = htonl(atoi(argv[i]));
			}
			else if (sizeof(long) == 4) {
				bit32 = htonl(atol(argv[i]));
			}
			memcpy(mess_ptr, (uint8_t*)&bit32, 4);
			mess_ptr += 4;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			i++;
			*(type_ptr++) = 'h';
			if (sizeof(long) == 8) {
				bit64 = htonll((int64_t)atol(argv[i]));
			}
			else if (sizeof(long long) == 8) {
				bit64 = htonll(atoll(argv[i]));
			}
			memcpy(mess_ptr, (uint8_t*)&bit64, 8);
			mess_ptr += 8;
		}
		else if (strcmp(argv[i], "-f") == 0) {
			i++;
			*(type_ptr++) = 'f';
			if (strcmp(argv[i], "-inf") == 0) {
				atom.f32 = log(0);
			}
			else if (strcmp(argv[i], "inf") == 0) {
				atom.f32 = 1./0.;
			}
			else {
				atom.f32 = (float)atof(argv[i]);
			}
			bit32 = htonf(atom.f32);
			memcpy(mess_ptr, (uint8_t*)&bit32, 4);
			mess_ptr += 4;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			i++;
			*(type_ptr++) = 'd';
			if (strcmp(argv[i], "-inf") == 0) {
				atom.f64 = log(0.);
			}
			else if (strcmp(argv[i], "inf") == 0) {
				atom.f64 = 1./0.;
			}
			else {
				atom.f64 = atof(argv[i]);
			}
			bit64 = htond(atom.f64);
			memcpy(mess_ptr, (uint8_t*)&bit64, 8);
			mess_ptr += 8;
		}
		else if (strcmp(argv[i], "-s") == 0) {
			i++;
			*(type_ptr++) = 's';
			strcpy((char*)mess_ptr, argv[i]);
			len = strlen(argv[i]);
			len += (4 - len % 4);
			mess_ptr += len;
		}
		else if (strcmp(argv[i], "-c") == 0) {
			i++;
			*(type_ptr++) = 'c';
			memcpy(mess_ptr, argv[i], 1);
			mess_ptr += 4;
		}
		else if (strcmp(argv[i], "-T") == 0) {
			i++;
			*(type_ptr++) = 'T';
		}
		else if (strcmp(argv[i], "-F") == 0) {
			i++;
			*(type_ptr++) = 'F';
		}
		else if (strcmp(argv[i], "-N") == 0) {
			i++;
			*(type_ptr++) = 'N';
		}
		else if (strcmp(argv[i], "-I") == 0) {
			i++;
			*(type_ptr++) = 'I';
		}
	}
	
	return osc_size;
}

int main (int argc, char* const argv[])
{
	char* ip;
	char* port;
	char* protocol;
	int sockfd, rv;
	struct addrinfo hints, *servinfo, *p;
	uint8_t* buf;
	int32_t size, bit32, sent;
	char isTCP = 0;
	
	if (argc < 5) {
		printf(usage);
		return 0;
	}
	
	ip = argv[1];
	port = argv[2];
	protocol = argv[3];
	
	// Check if protocol is tcp or udp
	if (strcmp(protocol, "tcp") != 0 && strcmp(protocol, "udp") != 0) {
		fprintf(stderr, "Specify protocol tcp or udp\n");
		printf(usage);
		return 1;
	}
	
	size = oscraw(&buf, argc-3, argv+3);
	if (size <= 0) {
		fprintf(stderr, "oscraw: error\n");
		return 1;
	}
	
	isTCP = strcmp(protocol, "tcp") == 0 ? 1 : 0;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = isTCP ? SOCK_STREAM : SOCK_DGRAM;
	
	if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1) {
			continue;
		}
		
		if (strcmp(protocol, "tcp") == 0) {
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				continue;
			}
		}
		
		break;
	}
	
	if (p == NULL) {
		if (isTCP) {
			fprintf(stderr, "socket: failed to connect\n");
		}
		else {
			fprintf(stderr, "socket: failed to bind socket\n");
		}
		
		freeaddrinfo(servinfo);
		return 1;
	}
	
	// Create a OSC packet and send
	if (isTCP) {
		// Add size of OSC message (32bit big-endian) at the beginning of buffer
		bit32 = htonl(size);
		memcpy(buf, &bit32, 4);
		
		if ((sent = send(sockfd, buf, size+4, 0)) == -1) {
			fprintf(stderr, "send: error\n");
		}
	}
	else {
		if ((sent = sendto(sockfd, buf, size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
			fprintf(stderr, "send: error\n");
		}
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);
	free(buf);
	return 0;
}

