/*
 *  UDPSend6.h
 *
 *  Created by Helmut Hlavacs (2022).
 *
 */

#include <iostream>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
}

class UDPSend6 {

public:
	int sock=0;
	struct sockaddr_in6 addr;
	unsigned int packetnum=0;

	UDPSend6(){};
	~UDPSend6(){};
	void init( char *address, int addr_len, int port );
	int send( char *buffer, int len  );
	void closeSock();
};



