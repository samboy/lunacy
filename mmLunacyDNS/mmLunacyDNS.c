/* Copyright (c) 2009-2020 Sam Trenholme
 *
 * TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This software is provided 'as is' with no guarantees of correctness or
 * fitness for purpose.
 *
 * This software links to Lunacy, a fork of Lua 5.1
 * Lua license is in the file COPYING
 */

/* mmLunacyDNS: A tiny DNS server which uses Lua for configuration and
 * for the main loop.  This is Lunacy, a fork of Lua 5.1, and it's 
 * embedded in the compiled binary
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* Luancy stuff */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* We use a special SOCKET type for easier Windows porting */
#define SOCKET int

/* Log a message */
void log_it(char *message) {
	if(message != NULL) {
		puts(message);
	}
}

/* This is the header placed before the 4-byte IP; we change the last four
 * bytes to set the IP we give out in replies */
char p[17] =
"\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x00\x00\x04\x7f\x7f\x7f\x7f";

/* Set the IP we send in response to DNS queries */
uint32_t set_return_ip(char *returnIp) {
        uint32_t ip;

	if(returnIp == NULL) {
		returnIp = "127.0.0.1";
	}
        /* Set the IP we give everyone */
        ip = inet_addr(returnIp);
        ip = ntohl(ip);
        p[12] = (ip & 0xff000000) >> 24;
        p[13] = (ip & 0x00ff0000) >> 16;
        p[14] = (ip & 0x0000ff00) >>  8;
        p[15] = (ip & 0x000000ff);
        return ip;
}

/* Convert a NULL-terminated string like "10.1.2.3" in to an IP */
uint32_t get_ip(char *stringIp) {
	uint32_t ip = 0;
        /* Set the IP we bind to (default is "0", which means "all IPs) */
        if(stringIp != NULL) {
        	ip = inet_addr(stringIp);
	}
        /* Return the IP we bind to */
        return ip;
}

/* Get port: Get a port locally and return the socket the port is on */
SOCKET get_port(uint32_t ip, struct sockaddr_in *dns_udp) {
        SOCKET sock;
        int len_inet;

        /* Bind to port 53 */
        sock = socket(AF_INET,SOCK_DGRAM,0);
        if(sock == -1) {
                perror("socket error");
                exit(0);
        }
        memset(dns_udp,0,sizeof(struct sockaddr_in));
        dns_udp->sin_family = AF_INET;
        dns_udp->sin_port = htons(53);
        dns_udp->sin_addr.s_addr = ip;
        if(dns_udp->sin_addr.s_addr == INADDR_NONE) {
                log_it("Problem with bind IP");
                exit(0);
        }
        len_inet = sizeof(struct sockaddr_in);
        if(bind(sock,(struct sockaddr *)dns_udp,len_inet) == -1) {
                perror("bind error");
                exit(0);
        }

        /* Linux kernel bug */
        /* fcntl(sock, F_SETFL, O_NONBLOCK); */

        return sock;
}

lua_State *init_lua(char *fileName) {
	char useFilename[512];
	lua_State *L = luaL_newstate(); // Initialize Lua
	/* The filename we use is {executable name}.lua.  
         * {executable name} is the name this is being called as,
         * usually mmLunacyDNS (or mmLunacyDNS.exe in Windows).
         * This way, if we want multiple Lua configuration files for
         * different use cases, we simple copy the binary (or link
         * to it) to make it use a different .lua configuration file.
         */
	if(fileName != NULL && *fileName != 0) {
		int a;
		int lastDot = 505;
		// Find the final '.' in the executable name
		for(a = 1; a < 500; a++) {
			if(fileName[a] == 0) {
				break;
			}
			if(fileName[a] == '.') {
				lastDot = a;
			}
		}
		for(a = 0; a < 500; a++) {
			useFilename[a] = *fileName;
			if(*fileName == 0 || a >= lastDot) {
				break;
			}
			fileName++;
		}
		useFilename[a] = '.'; a++;
		useFilename[a] = 'l'; a++;
		useFilename[a] = 'u'; a++;
		useFilename[a] = 'a'; a++;
		useFilename[a] = 0;
	} else {
		// Yes, it is possible to make argv[0] null
		strcpy(useFilename,"mmLunacyDNS.lua");
	}

	// Open and parse the .lua file
	if(luaL_loadfile(L, useFilename) == 0) {
		if(lua_pcall(L, 0, 0, 0) != 0) {
			log_it("Unable to parse lua file with name:");
			log_it(useFilename);
			log_it((char *)lua_tostring(L,-1));
			return NULL;
		}		
	} else {
		log_it("Unable to open lua file with name:");
		log_it(useFilename);
		log_it((char *)lua_tostring(L,-1));
		return NULL;
	}
	return L;
}

int main(int argc, char **argv) {
        int a, len_inet;
        SOCKET sock;
        char in[515];
        socklen_t foo = sizeof(in);
        struct sockaddr_in dns_udp;
        uint32_t ip = 0; /* 0.0.0.0; default bind IP */
        int leni = sizeof(struct sockaddr);
	lua_State *L;

	// Get bindIp and returnIp from Lua script
	if(argc == 1) {
		L = init_lua(argv[0]); // Initialize Lua
	} else if(argc == 3) {
		char *look = argv[1];
		if(look[0] == '-' && look[1] == 'f' && look[2] == 0) {
			L = init_lua(argv[2]); // Initialize Lua
		} else {
			log_it("Usage: mmLunacyDNS -f {config file}");
			return 1;
		}
	} else {
		log_it("Usage: mmLunacyDNS -f {config file}");
		return 1;
	}
	if(L == NULL) {
		log_it("Fatal error opening lua config file");
		return 1;
	}

	// Get bindIp from the Lua program 
        lua_getglobal(L,"bindIp"); // Push "bindIp" on to stack
        if(lua_type(L, -1) == LUA_TSTRING) {
		char *bindIp;
		bindIp = (char *)lua_tostring(L, -1); 
		ip = get_ip(bindIp);
	} else {
		log_it("Unable to get bindIp; using 0.0.0.0");
	}
	lua_pop(L, 1); // Remove result from stack, restoring the stack

	// Get returnIp from the Lua program	
        lua_getglobal(L,"returnIp"); // Push "bindIp" on to stack
        if(lua_type(L, -1) == LUA_TSTRING) {
		char *returnIp;
		returnIp = (char *)lua_tostring(L, -1); 
		set_return_ip(returnIp);
	} else {
		log_it("Unable to get returnIp; using 127.0.0.1");
		set_return_ip("127.0.0.1");
	}
	lua_pop(L, 1); // Remove result from stack, restoring the stack

        sock = get_port(ip,&dns_udp);

	log_it("Running mmLunacyDNS");

        /* Now that we know the IP and are on port 53, process incoming
         * DNS requests */
        for(;;) {
                /* Get data from UDP port 53 */
                len_inet = recvfrom(sock,in,255,0,(struct sockaddr *)&dns_udp,
                        &foo);
                /* Roy Arends check: We only answer questions */
                if(len_inet < 3 || (in[2] & 0x80) != 0x00) {
                        continue;
                }

                /* Prepare the reply */
                if(len_inet > 12) {
                        /* Make this an answer */
                        in[2] |= 0x80;
                        if(in[11] == 0) { /* EDNS not supported */
                                /* We add an additional answer */
                                in[7]++;
                        } else {
                                in[3] &= 0xf0; in[3] |= 4; /* NOTIMPL */
                        }
                }
                if(in[11] == 0) { /* Again, EDNS not supported */
                        for(a=0;a<16;a++) {
                                in[len_inet + a] = p[a];
                        }
                }

                /* Send the reply */
                sendto(sock,in,len_inet + 16,0, (struct sockaddr *)&dns_udp,
                        leni);
        }

}

