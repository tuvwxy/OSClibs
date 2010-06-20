/*
 *  main.c
 *
 *	This is a command line tool to get hex code values of a OSC message. Types
 *	and values can be specified. Depengin on TCP or UDP implementation, size and
 *	first 4 bytes of the format will be different. TCP has 4 extra bytes at the
 *	beginning of the message to specify the size of the OSC message. UDP doesn't
 *	require this becauseo of the way underlying protocol works.
 *
 *  Created by Toshiro Yamada.
 *  Copyright 2010 Calit2, UCSD. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "byteorder.h"

#define HELP "\n" \
"usage: OSCRaw tcp|udp /osc/address -type messages ...\n" \
"\n" \
"The first argument should be \"tcp\" or \"udp\". If set to \"tcp\", the\n" \
"first bytes specify the size of the OSC message.\n" \
"\n" \
"The second argument is the OSC Address Pattern. OSC Address Pattern starts\n" \
"with '/' (forward slash) and matches the URL syntax.\n" \
"\n" \
"After the OSC Address, OSC messages are specified by type and value (if \n" \
"applicable). The following is the list of types that can be use:\n" \
"\n" \
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
"\n"

int main (int argc, const char * argv[]) {
	
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
	int i;
	int32_t len;
	char bad_mess = 0;
	
	if (argc < 3) {
		goto help;
	}
	
	// Check first argument
	net = (char*)argv[1];
	if (strcmp(net, "tcp") != 0 && strcmp(net, "udp") != 0) {
		printf("Error: First argument must be \"tcp\" or \"udp\"" \
			   "(without quotes)\n");
		goto help;
	}
	
	// Check OSC Address Pattern
	osc_addr = (char*)argv[2];
	if (osc_addr[0] != '/') {
		printf("Error: OSC Address Pattern must start with '/' " \
			   "(forward slash)\n");
		goto help;
	}
	addr_size = strlen(osc_addr);
	addr_size += (4 - addr_size % 4);
	
	// +1 for ',' of type specifier
	type_size = 1;
	
	// Iterate once to check OSC type and messages, and calculate size
	for (i = 3; i < argc; ++i) {
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
		printf("Error: Bad OSC Type or Message\n");
		goto help;
	}
	
	type_size += (4 - type_size % 4);
	
	osc_size = addr_size + type_size + mess_size;
	
	// Create an array
	if (strcmp(net, "tcp") == 0) {
		osc_data = (uint8_t*)calloc(1, osc_size+4);
		if (osc_data == NULL) {
			fprintf(stderr, "Critical memory error...\n");
			exit(1);
		}
		bit32 = htonl(osc_size);
		memcpy(osc_data, (uint8_t*)&bit32, 4);
		osc_size += 4;
		addr_ptr = osc_data+4;
	}
	else {
		osc_data = (uint8_t*)calloc(1, osc_size);
		if (osc_data == NULL) {
			fprintf(stderr, "Critical memory error...\n");
			exit(1);
		}
		addr_ptr = osc_data;
	}
	
	// Copy OSC Address Pattern
	strcpy((char*)addr_ptr, osc_addr);
	
	type_ptr = addr_ptr + addr_size;
	mess_ptr = type_ptr + type_size;
	
	*(type_ptr++) = ',';
	
	// Iterate the arguments to fill the array
	for (i = 3; i < argc; ++i) {
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
			bit32 = htonf((float)atof(argv[i]));
			memcpy(mess_ptr, (uint8_t*)&bit32, 4);
			mess_ptr += 4;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			i++;
			*(type_ptr++) = 'd';
			bit64 = htond(atof(argv[i]));
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
	
	printf("\n8-bit hex data is:\n");
	for (i = 0; i < osc_size; ++i) {
		printf("%02x ", osc_data[i]);
		if (i % 16 == 15) {
			printf("\n");
		}
//		else if (i % 8 == 7 || i % 16 == 15) {
//			printf("   ");
//		}
	}
	printf("\n\n");
	
	free(osc_data);
	return 0;
	
help:
	printf(HELP);
	
    return 0;
}
