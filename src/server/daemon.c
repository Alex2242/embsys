#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <unistd.h>

#include "camera.h"
#include "image.h"
#include "network.h"
#include "api.h"

static int socketServer;
static int connFd;

/**
SIGINT interput handler
*/
void StopContCapture(int sig_id) {
	printf("stoping continuous capture\n");
	close(connFd);
	close(socketServer);
	captureUninit();
}

void InstallSIGINTHandler() {
	struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
	
	sa.sa_handler = StopContCapture;

	if (sigaction(SIGINT, &sa, 0) != 0) {
		fprintf(stderr, "could not install SIGINT handler, continuous capture disabled");
	}
}


void runDaemon(int port) {
	InstallSIGINTHandler();

	socketServer = createSocket();

	serverListen(socketServer, port);

	struct sockaddr cliAddr;
	uint32_t cliAddrLen = sizeof(cliAddr);

	Message msg;

	printf("info: server listening on port %d\n", port);

	for (;;) {
		ACCEPT_CONN:
		connFd = accept(socketServer, &cliAddr, &cliAddrLen);
		printf("info: new client connected\n");

		for (;;) {
			readData(connFd, &msg, sizeof(Message));

			switch (msg.op) {
				case disconnect:
					printf("info: client disconnecting\n");
					close(connFd);
					goto ACCEPT_CONN;
					break;

				case shutdownServ:
					printf("info: server shutdown\n");
                    close(connFd);
					// make sure to close the video device
					captureUninit();
					exit(EXIT_SUCCESS);
					break;
				
				case readImg: {
					printf("info: image requested by client\n");
                    camOpt co;

                    readData(connFd, &co, sizeof(camOpt));
					co.deviceName = "/dev/video0";

                    int imgSize = co.width * co.height * 3 * sizeof(char);

                    char *rawImage = malloc(imgSize);

                    capture(&co, rawImage);
                    
                    Message msgRep = {sendImg, imgSize};

                    sendData(connFd, &msgRep, sizeof(Message));
                    sendData(connFd, rawImage, imgSize);

                    free(rawImage);

					break;
                }

				case syn: {
					printf("info: client syn\n");
					Message msgRep = {ack, 0};
					sendData(connFd, &msgRep, sizeof(Message));
					break;
				}

				default:
					printf("error: unhandled operation %d code for server\n", msg.op);
					break;
			}
		}
	}
}
