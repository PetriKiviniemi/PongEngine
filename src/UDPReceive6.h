/*
 *  UDPReceive6.h
 *
 *  Created by Helmut Hlavacs (2022).
 *  Modified to Linux version by Petri Kiviniemi
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



class UDPReceive6 {

public:
	int sock;
	struct sockaddr_in6 addr;
	char* recbuffer;

	UDPReceive6();
	~UDPReceive6() { delete recbuffer; };
	void init(int port);
	int receive(char* buffer, int len, double* ptime);
	void closeSock();
};
