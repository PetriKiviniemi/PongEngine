/*
 *  UDPSend6.cpp
 *
 *  Created by Helmut Hlavacs (2022).
 *  Modified to linux version by Petri Kiviniemi
 *
 */


#include "UDPSend6.h"

extern "C" {
#include <stdio.h>
#include <time.h>
}


typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;

void UDPSend6::init(char *address, int addr_len,  int port ) {	
	sock = socket( AF_INET6, SOCK_DGRAM, 0 );

    struct addrinfo hints;

    memset(&addr, 0, sizeof(addr));
    memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;

    struct addrinfo *result = NULL;
    auto dwRetval = getaddrinfo(address, nullptr, &hints, &result);
    if ( dwRetval != 0 ) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        return;
    }
	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		if (ptr->ai_family == AF_INET6) {
			memcpy(&addr, ptr->ai_addr, ptr->ai_addrlen);
			addr.sin6_port = htons(port);
			addr.sin6_family = AF_INET6;
		}
	}
	freeaddrinfo(result);
}

int UDPSend6::send( char *buffer, int len ) {
	char sendbuffer[65000];
	
	packetnum++;
	
	if( len>65000 ) {
		return 0;
	}
		
	RTHeader_t header;
	header.time = clock() / (double)CLOCKS_PER_SEC;
	header.packetnum = packetnum;
		
	int ret;
	memcpy( sendbuffer, &header, sizeof( header ) );
    memcpy( sendbuffer + sizeof( header), buffer, len );
		
	ret = sendto( sock, sendbuffer, sizeof( header ) + len, 0, (const struct sockaddr *) &addr, sizeof(addr) );
    return ret;
}


void UDPSend6::closeSock() {
	close(sock);
	sock=0;
}



/*
int main() {

	startWinsock();

	UDPSend6 sender;

	sender.init("::1", 50000);


	char buf[100] = "1234567890abcdefghijklm\n";

	sender.send(buf, strlen(buf));

	WSACleanup();
}
*/





