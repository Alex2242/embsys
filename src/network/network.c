#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "network.h"

#define CHECK_ERROR(rc, s) if (rc != 0) {perror(s);}

int createSocket(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("Failed to create the network socket\n");
    }

    return sock;
}

int serverListen(int servSock, int port) {
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	int rc = bind(servSock, (struct sockaddr*)&servAddr , sizeof(servAddr));

    CHECK_ERROR(rc, "failed to bind to port")

	rc |= listen(servSock, 1);

    CHECK_ERROR(rc, "failed to listen on port")

    return rc;
}

int clientConnect(int sock, char* servHost, int port) {
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = inet_addr(servHost);

    int rc = connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));

    CHECK_ERROR(rc, "Failed to connect to client")

    return rc;
}

int sendData(int destFd, void *data, uint32_t dataSize) {
    uint8_t buffer[BUFF_SIZE];

    uint32_t nPackets = dataSize / BUFF_SIZE + (dataSize % BUFF_SIZE > 0);

    uint32_t trailingData = dataSize % BUFF_SIZE;

    for (uint32_t i = 0; i < nPackets; i++) {
        // deal with partial packet
        if (i == (nPackets - 1)) {
            memcpy(buffer, ((uint8_t*) data) + i * BUFF_SIZE, trailingData);
            int bWrite = write(destFd, buffer, BUFF_SIZE);
        } else {
            memcpy(buffer, ((uint8_t*) data) + i * BUFF_SIZE, BUFF_SIZE);
            write(destFd, buffer, BUFF_SIZE);
        }
    }

    return 0;
}

int readData(int srcFd, void *data, uint32_t dataSize) {
    uint8_t buffer[BUFF_SIZE];

    uint32_t bytesRead;

    uint32_t dataTransffered = 0;

    while (dataTransffered < dataSize) {
        bytesRead = read(srcFd, buffer, BUFF_SIZE);

        if ((dataTransffered + bytesRead) > dataSize) {
            int bytesLeft = dataSize - dataTransffered;
            memcpy(((char*) data) + dataTransffered , buffer, bytesLeft);
        }
        else {
            memcpy(((char*) data) + dataTransffered , buffer, bytesRead);
        }

        dataTransffered += bytesRead;
    } 

    return 0;
}