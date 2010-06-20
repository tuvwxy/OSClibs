/*
 *  oscpacktest.c
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

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "oscpack.h"

const char msg[] = "Open Sound Control (OSC) is an open, \
transport-independent, message-based protocol developed for communication \
among computers, sound synthesizers, and other multimedia devices.";

const char usage[] = "usage: oscpacktest ip port tcp|udp\n";

// Define the OSC message here. (use this macro so you don't have to copy at
// different parts of the code)
//
// First string is the OSC address pattern. (must start with '/')
// Next specify types. Look at oscpack.h for available types.
// Followed by the types, add the values separating with commas
#define OSC_MSG "/oscpacktest", "ifs", 123456, 0.123456, msg

int main (int argc, char* const argv[])
{
	char* ip;
	char* port;
	char* protocol;
	int sockfd, rv;
	struct addrinfo hints, *servinfo, *p;
	uint8_t* buf;
//	uint8_t buf[1024];
	int32_t size, bit32, sent;
	char isTCP = 0;
	
	if (argc != 4) {
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
	
	
	// Find out OSC packet size 
	size = oscsize(OSC_MSG);
	if (size < 0) {
		fprintf(stderr, "oscsize: error\n");
		freeaddrinfo(servinfo);
		return 1;
	}
	
	// Allocate buffer to the right size (note that you need 4 more bytes when
	// sending OSC messages over TCP)
	if (isTCP) {
		buf = (uint8_t*) malloc(sizeof(uint8_t) * (size + 4));
	}
	else {
		buf = (uint8_t*) malloc(sizeof(uint8_t) * size);
	}
	
	if (!buf) {
		fprintf(stderr, "malloc: fail\n");
		freeaddrinfo(servinfo);
		return 1;
	}
	
	// Create a OSC packet and send
	if (isTCP) {
		// Use oscpack to packet a OSC message.
		// For TCP, offset buffer by 4 bytes so we can add the packet size at 
		// the beginning of the buffer.
		size = oscpack(buf+4, OSC_MSG);
		if (size <= 0) {
			fprintf(stderr, "oscpack: error\n");
			freeaddrinfo(servinfo);
			free(buf);
			return 1;
		}
		
		// Add size of OSC message (32bit big-endian) at the beginning of buffer
		bit32 = htonl(size);
		memcpy(buf, &bit32, 4);
		
		if ((sent = send(sockfd, buf, size+4, 0)) == -1) {
			fprintf(stderr, "send: error\n");
		}
	}
	else {
		// Use oscpack to packet a OSC message.
		// No need to offset the buffer for UDP.
		size = oscpack(buf, OSC_MSG);
		if (size <= 0) {
			fprintf(stderr, "oscpack: error\n");
			freeaddrinfo(servinfo);
			free(buf);
			return 1;
		}
		
		if ((sent = sendto(sockfd, buf, size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
			fprintf(stderr, "send: error\n");
		}
	}
	
	if (sent > 0) {
		printf("oscpacktest: %d bytes sent over %s\n", sent, protocol);
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);
	free(buf);
	return 0;
}
