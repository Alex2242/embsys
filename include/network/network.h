#ifndef NETWORK_H
#define NETWORK_H

#define PORT 21245
#define BUFF_SIZE 512

#include <stdint.h>


int createSocket();
int serverListen(int servSock, int port);
int clientConnect(int sock, char* servHost, int port);
int readData(int srcFd, void *data, uint32_t dataSize);
int sendData(int destFd, void *data, uint32_t dataSize);

#endif