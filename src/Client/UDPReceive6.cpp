/*
 *  UDPReceive6.cpp
 *
 *  Created by Helmut Hlavacs (2022).
 *  Modified to linux version by Petri Kiviniemi
 *
 */


#include "UDPReceive6.h"

extern "C" {
#include <stdio.h>
#include <time.h>
}


typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;


UDPReceive6::UDPReceive6() {
	recbuffer = new char[65000];
}


void UDPReceive6::init(int port) {
    // Create an IPv6 UDP socket
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    // Bind the socket to the specified port and any IPv6 address
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;

    int ret = bind(sock, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    if (ret == -1) {
        std::cerr << "Error binding socket\n";
        close(sock);
        return;
    }
}


int UDPReceive6::receive(char* buffer, int len, double* ptime) {
	struct sockaddr_in6 si_other;
	socklen_t slen = sizeof(si_other);

	RTHeader_t* pheader = (RTHeader_t*)recbuffer;

	while (true) {
		auto ret = recvfrom(sock, recbuffer, 65000, 0, (sockaddr*)&si_other, &slen);

		if (ret > sizeof(RTHeader_t)) {
			memcpy(buffer, recbuffer + sizeof(RTHeader_t), ret - sizeof(RTHeader_t));
			return ret - sizeof(RTHeader_t);
		}
	}
	return 0;
}


void UDPReceive6::closeSock() {
	close(sock);
	sock = 0;
}


/*
int main() {

	startWinsock();

	UDPReceive6 receiver;

	receiver.init(50000);

	while (true) {
		char buf[65000];
		double ptime;
		auto ret = receiver.receive(buf, sizeof buf, &ptime);
		buf[ret] = '\0';
		printf("Message: % s\n", buf);
	}

	WSACleanup();
}
*/



