#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>

#include "camera.h"
#include "image.h"
#include "network.h"
#include "api.h"



/**
	print usage information
*/
static void usage(FILE* fp, int argc, char** argv) {
	fprintf(fp,
		"Usage: %s [options]\n\n"
		"Options:\n"
		"-d | --device name   Video device name [/dev/video0]\n"
		"-a | --address		  The address of the server\n"
		"-p | --port		  The port of the server\n"
		"-h | --help          Print this message\n"
		"-o | --output        Set JPEG output filename\n"
		"-q | --quality       Set JPEG quality (0-100)\n"
		"-m | --mmap          Use memory mapped buffers\n"
		"-r | --read          Use read() calls\n"
		"-u | --userptr       Use application allocated buffers\n"
		"-W | --width         Set image width\n"
		"-H | --height        Set image height\n"
		"-c | --capture       Capture an image on the server\n"
		"-v | --version       Print version\n"
		"",
		argv[0]);
	}

static const char short_options [] = "cp:a:d:ho:q:mruW:H:v";

static const struct option
long_options [] = {
	{ "capture",	no_argument,		    NULL,		    'c' },
	{ "address",	required_argument,		NULL,		    'a' },
	{ "port",	    required_argument,		NULL,		    'p' },
	{ "device",     required_argument,      NULL,           'd' },
	{ "help",       no_argument,            NULL,           'h' },
	{ "output",     required_argument,      NULL,           'o' },
	{ "quality",    required_argument,      NULL,           'q' },
	{ "mmap",       no_argument,            NULL,           'm' },
	{ "read",       no_argument,            NULL,           'r' },
	{ "userptr",    no_argument,            NULL,           'u' },
	{ "width",      required_argument,      NULL,           'W' },
	{ "height",     required_argument,      NULL,           'H' },
	{ "version",	no_argument,		    NULL,		    'v' },
	{ 0, 0, 0, 0 }
};


int main(int argc, char **argv) {
    camOpt co = {
        .deviceName = "/dev/video0",
        .jpegFilename = "capture.jpg",
        .jpegFilenamePart = "capture-",
        .iom = IO_METHOD_MMAP,
        .jpegQuality = DEFAULT_JQAL,
        .width = DEFAULT_WIDTH,
        .height = DEFAULT_HEIGHT,
        .fps = DEFAULT_FPS,
        .continuous = false
    };

    // NOLINTNEXTLINE(readability-magic-numbers)
    char *serverAddr;
    bool requestImg = false;
    int port = PORT;

	for (;;) {
		int index;
		int c = 0;

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

			case 'p':
				port = atoi(optarg);
				break;

			case 'd':
				co.deviceName = optarg;
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

			case 'm':
				co.iom = IO_METHOD_MMAP;
				break;

			case 'r':
				co.iom = IO_METHOD_READ;
				break;

			case 'u':
				co.iom = IO_METHOD_USERPTR;
				break;

			case 'W':
				// set width
				co.width = atoi(optarg);
				break;

			case 'H':
				// set height
				co.height = atoi(optarg);
				break;
				
			case 'I':
				// set fps
				co.fps = atoi(optarg);
				break;
				
			case 'v':
				printf("Version: %s\n", VERSION);
				exit(EXIT_SUCCESS);
				break;

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

    if (serverAddr == NULL) {
        exit(EXIT_SUCCESS);
    }

	int sockFd = createSocket();
    printf("info: client connecting to %s on port %d\n", serverAddr, port);
    clientConnect(sockFd, serverAddr, port);

    printf("info: connected, sending syn/ack\n");
    Message msa = {syn, 0};
    sendData(sockFd, &msa, sizeof(Message));

    // ack
    Message msaRep;
    readData(sockFd, &msaRep, sizeof(Message));

    assert(msaRep.op == ack);
    printf("info: server responded to syn/ack\n");

    if (requestImg) {
        printf("info: requesting capture from server\n");
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

    printf("info: shuting down server\n");

    Message ms = {shutdownServ, 0};
    sendData(sockFd, &ms, sizeof(Message));

    close(sockFd);

	return 0;
}

 
