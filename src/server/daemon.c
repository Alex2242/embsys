#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>

#include "camera.h"
#include "image.h"
#include "network.h"
#include "api.h"


void daemon() {
	int socketServer = createSocket();
	int connFd;

	serverListen(socketServer);

	struct sockaddr cliAddr;
	uint32_t cliAddrLen = sizeof(cliAddr);

	Message msg;

	for (;;) {
		ACCEPT_CONN:
		connFd = accept(socketServer, &cliAddr, &cliAddrLen);

		for (;;) {
			readData(connFd, &msg, sizeof(Message));

			switch (msg.op) {
				case disconnect:
					connFd = 0;
					goto ACCEPT_CONN;
					break;

				case shutdownServ:
                    connFd = 0;
					break;
				
				case readImg: {
                    camOpt *co = malloc(sizeof(camOpt));

                    readData(connFd, co, sizeof(camOpt));

                    uint64_t imgSize = co->width * co->height * 3 * sizeof(uint8_t);
                    uint8_t *rawImage = malloc(imgSize);

                    capture(co, rawImage);
                    
                    Message msgRep = {sendImg, imgSize};

                    sendData(connFd, &msgRep, sizeof(Message));
                    sendData(connFd, rawImage, imgSize);

                    free(rawImage);

					break;
                }
				
				default:
					perror("unhandled operation code for server");
					break;
			}
		}
	}
}
