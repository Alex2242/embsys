#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <syslog.h>

#include "camera.h"
#include "image.h"
#include "network.h"
#include "api.h"



/**
	print usage information
*/
static void usage(FILE* fp, int argc, char** argv) {
	fprintf(fp,
		"Usage: \n"
		"\tclient -a SERVER_ADDR [OPTION]...\n"
		"\t\tconnect the server, handshake and disconnect\n"
		"Options:\n"
		"\t-p | --port PORT              The port of the server [21245]\n"
		"\t-o | --output FILENAME        Set JPEG output filename [capture.jpg]\n"
		"\t-q | --quality JPEG_QUALITY   Set JPEG quality (0-100) [70]\n"
		"\t-W | --width WIDTH            Set image width [640]\n"
		"\t-H | --height HEIGHT          Set image height [480]\n"
		"\t-c | --capture                Capture an image on the server\n"
		"\t-s | --shutdown               Shutdown server after transaction\n"
		"\t-h | --help                   Print this message\n"
		"");
	}

static const char short_options [] = "cp:a:ho:q:W:H:s";

static const struct option
long_options [] = {
	{ "capture",	no_argument,		    NULL,		    'c' },
	{ "address",	required_argument,		NULL,		    'a' },
	{ "port",	    required_argument,		NULL,		    'p' },
	{ "help",       no_argument,            NULL,           'h' },
	{ "output",     required_argument,      NULL,           'o' },
	{ "quality",    required_argument,      NULL,           'q' },
	{ "width",      required_argument,      NULL,           'W' },
	{ "height",     required_argument,      NULL,           'H' },
	{ "shutdown",	no_argument,		    NULL,		    's' },
	{ 0, 0, 0, 0 }
};


int main(int argc, char **argv) {
    camOpt co = {
        .deviceName = "/dev/video0",
        .jpegFilename = "capture.jpg",
        .iom = IO_METHOD_MMAP,
        .jpegQuality = DEFAULT_JQAL,
        .width = DEFAULT_WIDTH,
        .height = DEFAULT_HEIGHT,
    };

    char *serverAddr = NULL;
    bool requestImg = false;
    int port = PORT;

	bool shutdownServer = false;

	int c;

	for (;;) {
		int index;

		c = getopt_long(argc, argv, short_options, long_options, &index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 0: /* getopt_long() flag */
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);
				break;

            case 'a':
                serverAddr = optarg;
                break;

            case 'c':
                requestImg = true;
                break;

            case 's':
                shutdownServer = true;
                break;

			case 'p':
				port = atoi(optarg);
				break;

			case 'h':
				// print help
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			case 'o':
				// set jpeg filename
				co.jpegFilename = optarg;
				break;

			case 'q':
				// set jpeg quality
				co.jpegQuality = atoi(optarg);
				break;

			case 'W':
				// set width
				co.width = atoi(optarg);
				break;

			case 'H':
				// set height
				co.height = atoi(optarg);
				break;
				
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

    if (serverAddr == NULL) {
		usage(stderr, argc, argv);
        exit(EXIT_SUCCESS);
    }

    syslog(LOG_INFO, "client connecting to %s on port %d\n", serverAddr, port);

	int sockFd = createSocket();
    clientConnect(sockFd, serverAddr, port);

    syslog(LOG_INFO, "client connected to %s on port %d\n", serverAddr, port);
    syslog(LOG_INFO, "sending syn/ack\n");
	// syn
    Message msa = {syn, 0};
    sendData(sockFd, &msa, sizeof(Message));

    // ack
    Message msaRep;
    readData(sockFd, &msaRep, sizeof(Message));

	// confirm that the received message is ack
    assert(msaRep.op == ack);
    syslog(LOG_INFO, "server responded to syn/ack\n");

    if (requestImg) {
        syslog(LOG_INFO, "requesting capture from server\n");
        Message mi = {readImg, sizeof(camOpt)};
        Message mir;

        sendData(sockFd, &mi, sizeof(Message));
        sendData(sockFd, &co, sizeof(camOpt));

        readData(sockFd, &mir, sizeof(Message));

        assert(mir.op == sendImg);

        uint64_t imgSize = co.width * co.height * 3 * sizeof(uint8_t);
        uint8_t *rawImage = malloc(imgSize);

        readData(sockFd, rawImage, imgSize);

        jpegWrite(rawImage, co);

        free(rawImage);
    }
	
	if (shutdownServer) {
		syslog(LOG_INFO, "shuting down server\n");
		Message ms = {shutdownServ, 0};
		sendData(sockFd, &ms, sizeof(Message));
	}
	else {
		syslog(LOG_INFO, "disconnecting from server\n");
		Message ms = {disconnect, 0};
		sendData(sockFd, &ms, sizeof(Message));
	}

    close(sockFd);

	return 0;
}

 
