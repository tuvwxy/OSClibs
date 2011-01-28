OSCLibs README
==============

This is a collection of [OpenSoundControl](http://opensoundcontrol.org/) (OSC) protocol command-line tools and libraries. 

Tools & Libraries
-----------------

### oscpack

`oscpack` is a printf-like library to encode OSC packets.

For example to create an OSC packet and send it via UDP:

    // UDP example
    uint8_t packet[256];
    int32_t size = oscpack(packet, "/my/address", "ifs", 123, 1.23, "msg"); 
    send(socket, packet, size, 0);

To send it via TCP, you need to encode the size of the OSC message at the
beginning of the packet.

    // TCP example
    uint8_t packet[256];
    int32_t size = oscpack(packet+4, "/my/address", "ifs", 123, 1.23, "msg");
    int32_t bit32 = htonl(size);
    memcpy(packet, &bit32, 4);
    send(socket, packet, size+4, 0);

"/my/address" is the OSC address.
"ifs" is the types of the arguments; in this case, 32-bit integer, 32-bit
floating point, and string.

Supported types are:

*   i: 32-bit integer
*   h: 64-bit integer
*   f: 32-bit floating point
*   d: 64-bit double floating point
*   s: string (array of char)	
*   c: ASCII character
*   T: True  (no argument needed)
*   F: False (no argument needed)
*   N: Nil (no argument needed)
*   I: Infinitum (no argument needed)

Unsupported types are (TODO...):

*   b: blob
*   t: timetag
*   r: 32 bit RGBA color (array of 4 32-bit integer?)
*   m: 4 byte MIDI message. Bytes from MSB to LSB are: 
    port id, status byte, data1, data2

Additionaly, `oscsize()` can be used to find out the size of the OSC packet.

### oscraw

`oscraw` is a commad-line tool to print hexidecimal values of the OSC packet.

    $ ./oscraw udp /my/address -i 123 -f 1.23 -s "this is a string"
    
    8-bit hex data is:
    2f 6d 79 2f 61 64 64 72 65 73 73 00 2c 69 66 73 
    00 00 00 00 00 00 00 7b 3f 9d 70 a4 74 68 69 73 
    20 69 73 20 61 20 73 74 72 69 6e 67 00 00 00 00 
    
### oscsend

`oscsend` is a command-line tool to send OSC message via TCP or UDP.

    $ ./oscsend 127.0.0.1 7374 udp /my/address -i 123 -f 1.23 -s "this is a string"

Compilation
-----------

### Using GCC

All tools and libraries use POSIX library and should compile without any
problem on *nix systems. oscpack has been ported to Windows, but others require
tweaking to get it working on Windows.

oscraw:
    cd oscraw/
    gcc -o oscraw oscraw.c
    
oscsend:
    cd oscsend/
    gcc -lm -o oscsend oscsend.c

`-lm` is need to incude the math library.

### Making a universal binary on OS X

You can pass `-arch` to gcc to specify the target architecture. On Snow Leopard,
supported targets are i386 (Intel/32bits), x86_64 (Intel/64bits), or ppc 
(PowerPC/32bits).

    gcc -lm -arch i386 -arch x86_64 -arch ppc -o oscsend oscsend.c


Copyright
---------

This software is Copyright © 2010-2011 The Regents of the University of California. All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its documentation for educational, research and non-profit purposes, without fee, and without a written agreement is hereby granted, provided that the above copyright notice, this paragraph and the following three paragraphs appear in all copies.

Permission to make commercial use of this software may be obtained by contacting:
Technology Transfer Office
9500 Gilman Drive, Mail Code 0910
University of California
La Jolla, CA 92093-0910
(858) 534-5815
invent@ucsd.edu

This software program and documentation are copyrighted by The Regents of the University of California. The software program and documentation are supplied "as is", without any accompanying services from The Regents. The Regents does not warrant that the operation of the program will be uninterrupted or error-free. The end-user understands that the program was developed for research purposes and is advised not to rely exclusively on the program for any reason.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

