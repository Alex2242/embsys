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


// declare prototype of daemon which has no header (internal)
void daemon();

/**
SIGINT interput handler
*/
void StopContCapture(int sig_id) {
	printf("stoping continuous capture\n");
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

/**
	print usage information
*/
static void usage(FILE* fp, int argc, char** argv) {
	fprintf(fp,
		"Usage: %s [options]\n\n"
		"Options:\n"
		"-d | --device name   Video device name [/dev/video0]\n"
		"-h | --help          Print this message\n"
		"-o | --output        Set JPEG output filename\n"
		"-q | --quality       Set JPEG quality (0-100)\n"
		"-m | --mmap          Use memory mapped buffers\n"
		"-r | --read          Use read() calls\n"
		"-u | --userptr       Use application allocated buffers\n"
		"-W | --width         Set image width\n"
		"-H | --height        Set image height\n"
		"-I | --interval      Set frame interval (fps) (-1 to skip)\n"
		"-c | --continuous    Do continous capture, stop with SIGINT.\n"
		"-v | --version       Print version\n"
		"",
		argv[0]);
	}

static const char short_options [] = "d:ho:q:mruW:H:I:vc";

static const struct option
long_options [] = {
	{ "device",     required_argument,      NULL,           'd' },
	{ "help",       no_argument,            NULL,           'h' },
	{ "output",     required_argument,      NULL,           'o' },
	{ "quality",    required_argument,      NULL,           'q' },
	{ "mmap",       no_argument,            NULL,           'm' },
	{ "read",       no_argument,            NULL,           'r' },
	{ "userptr",    no_argument,            NULL,           'u' },
	{ "width",      required_argument,      NULL,           'W' },
	{ "height",     required_argument,      NULL,           'H' },
	{ "interval",   required_argument,      NULL,           'I' },
	{ "version",	no_argument,		NULL,		'v' },
	{ "continuous",	no_argument,		NULL,		'c' },
	{ 0, 0, 0, 0 }
};



int main(int argc, char **argv) {
    // camOpt co;
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

	for (;;) {
		int index;
		int c = 0;

		c = getopt_long(argc, argv, short_options, long_options, &index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 0: /* getopt_long() flag */
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
				// fprintf(stderr, "You didn't compile for mmap support.\n");
				// exit(EXIT_FAILURE);
				break;

			case 'r':
				co.iom = IO_METHOD_READ;
				// fprintf(stderr, "You didn't compile for read support.\n");
				// exit(EXIT_FAILURE);
				break;

			case 'u':
				co.iom = IO_METHOD_USERPTR;
				// fprintf(stderr, "You didn't compile for userptr support.\n");
				// exit(EXIT_FAILURE);
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

			case 'c':
				// set flag for continuous capture, interuptible by sigint
				co.continuous = true;
				// InstallSIGINTHandler();
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

	daemon();	

	return 0;
}

