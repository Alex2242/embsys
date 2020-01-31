#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <utils.h>
#include <syslog.h>

#include "camera.h"
#include "image.h"
#include "network.h"
#include "api.h"


// declare prototype of daemon which has no header (internal)
void runDaemon(int port, camOpt co);


/**
	print usage information
*/
static void usage(FILE* fp, int argc, char** argv) {
	fprintf(fp,
		"Usage: server (-c | -s) [OPTION]...\n"
		"\tserver -s [OPTION]...\n"
		"\t\tstart the server and wait for connections from client\n"
		"\tserver -c [OPTION]...\n"
		"\t\tCapture an image on the server\n\n"
		"Options:\n"
		"\t-p | --port  PORT             Port for clients to connect [21245]\n"
		"\t-d | --device NAME            Video device name [/dev/video0]\n"
		"\t-m | --mmap                   Use memory mapped buffers [default]\n"
		"\t-r | --read                   Use read() calls\n"
		"\t-u | --userptr                Use application allocated buffers\n"
		"\t-p | --port PORT              The port of the server [21245]\n"
		"\t-o | --output FILENAME        Set JPEG output filename [capture.jpg]\n"
		"\t-q | --quality JPEG_QUALITY   Set JPEG quality (0-100) [70]\n"
		"\t-W | --width WIDTH            Set image width [640]\n"
		"\t-H | --height HEIGHT          Set image height [480]\n"
		"\t-S | --syslog                 Use system logger instead of stdout for logging\n"
		"\t-h | --help                   Print this message\n"
		"");
	}

static const char short_options [] = "Sscp:d:ho:q:mruW:H:";

static const struct option
long_options [] = {
	{ "device",     required_argument,      NULL,           'd' },
	{ "capture",	no_argument,		    NULL,		    'c' },
	{ "help",       no_argument,            NULL,           'h' },
	{ "server",     no_argument,            NULL,           's' },
	{ "port",     	required_argument,		NULL,           'p' },
	{ "output",     required_argument,      NULL,           'o' },
	{ "quality",    required_argument,      NULL,           'q' },
	{ "mmap",       no_argument,            NULL,           'm' },
	{ "read",       no_argument,            NULL,           'r' },
	{ "userptr",    no_argument,            NULL,           'u' },
	{ "width",      required_argument,      NULL,           'W' },
	{ "height",     required_argument,      NULL,           'H' },
	{ "syslog",    no_argument,            NULL,           'S' },
	{ 0, 0, 0, 0 }
};



int main(int argc, char **argv) {
	// default camera options
    camOpt co = {
        .deviceName = "/dev/video0",
        .jpegFilename = "capture.jpg",
        .iom = IO_METHOD_MMAP,
        .jpegQuality = DEFAULT_JQAL,
        .width = DEFAULT_WIDTH,
        .height = DEFAULT_HEIGHT,
    };

	bool runServer = false;
	bool captureLocal = false;
	int port = PORT;

	int c = 0;

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

			case 'd':
				co.deviceName = optarg;
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 's':
				runServer = true;
				break;

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			case 'c':
				captureLocal = true;
				break;

			case 'o':
				co.jpegFilename = optarg;
				break;

			case 'q':
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
				co.width = atoi(optarg);
				break;

			case 'H':
				co.height = atoi(optarg);
				break;

			case 'S':
				setLogOutput(SYSLOG);
				break;

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

	// incorrect usage
	if (runServer && captureLocal) {
		usage(stderr, argc, argv);
		exit(EXIT_FAILURE);
	}
	else if (runServer) {
		logging(LOG_INFO, "starting server");
		runDaemon(port, co);	
	}
	else if (captureLocal) {
		logging(LOG_INFO, "capturing image on the server");

		int imgSize = co.width * co.height * 3 * sizeof(char);

		char *rawImage = malloc(imgSize);

		capture(&co, rawImage);

        jpegWrite(rawImage, co);
	}

	return 0;
}

