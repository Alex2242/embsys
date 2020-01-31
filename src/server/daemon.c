#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <unistd.h>
#include <syslog.h>
#include <utils.h>

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
	exit(EXIT_SUCCESS);
}

void InstallSIGINTHandler() {
	struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
	
	sa.sa_handler = StopContCapture;

	if (sigaction(SIGINT, &sa, 0) != 0) {
		fprintf(stderr, "could not install SIGINT handler, continuous capture disabled");
	}
}


void runDaemon(int port, camOpt co) {
	InstallSIGINTHandler();

	socketServer = createSocket();

	serverListen(socketServer, port);

	struct sockaddr cliAddr;
	uint32_t cliAddrLen = sizeof(cliAddr);

	Message msg;

	//NOLINTNEXTLINE(readability-magic-numbers)
	char logMessage[128];
	snprintf(logMessage, sizeof(logMessage),"server listening on port %d\n", port);
	logging(LOG_INFO, logMessage);

	for (;;) {
		ACCEPT_CONN:
		connFd = accept(socketServer, &cliAddr, &cliAddrLen);
		logging(LOG_INFO, "new client connected\n");

		for (;;) {
			readData(connFd, &msg, sizeof(Message));

			switch (msg.op) {
				case disconnect:
					logging(LOG_INFO, "client disconnecting\n");
					close(connFd);
					goto ACCEPT_CONN;
					break;

				case shutdownServ:
					logging(LOG_INFO, "server shutdown\n");
                    close(connFd);
					// make sure to close the video device
					captureUninit();
					exit(EXIT_SUCCESS);
					break;
				
				case readImg: {
					logging(LOG_INFO, "image requested by client\n");

                    camOpt coIn;

					// read client's camera options
                    readData(connFd, &coIn, sizeof(camOpt));

					// copy the relevant camera options for capture
					co.width = coIn.width;
					co.height = coIn.height;

                    int imgSize = co.width * co.height * 3 * sizeof(uint8_t);

                    char *rawImage = malloc(imgSize);

                    capture(&co, rawImage);
                    
					// notify client of incoming image size
                    Message msgRep = {sendImg, imgSize};

                    sendData(connFd, &msgRep, sizeof(Message));
					// send raw image
                    sendData(connFd, rawImage, imgSize);

                    free(rawImage);

					break;
                }

				case syn: {
					logging(LOG_INFO, "client syn\n");
					Message msgRep = {ack, 0};
					sendData(connFd, &msgRep, sizeof(Message));
					break;
				}

				default:
					snprintf(logMessage, sizeof(logMessage),  "unhandled operation %d code for server\n", msg.op);
					logging(LOG_ERR, logMessage);
					break;
			}
		}
	}
}
