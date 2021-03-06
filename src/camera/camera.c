#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#include "camera.h"
#include "image.h"


#define CLEAR(x) memset (&(x), 0, sizeof(x))

// internal functions declaration
void errno_exit(const char* s);
int xioctl(int fd, int request, void* argp);

void captureStart();
void captureLoop(uint8_t *rawImage);
void captureStop();

int frameRead(uint8_t *rawImage);
void imageProcess(const void* p, uint8_t *rawImage);

void initIo(uint32_t buffer_size);
void readInit(uint32_t buffer_size);
void mmapInit();
void userptrInit(uint32_t buffer_size);

void deviceInit();
void deviceUninit();
void deviceClose();

// global variables
int fd = -1;
struct buffer *buffers = NULL;
uint32_t n_buffers = 0;

// global settings
camOpt *co = NULL;
bool initialized = false;


/**
	Print error message and terminate programm with EXIT_FAILURE return code.

	\param s error message to print
*/
void errno_exit(const char* s) {
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

/**
	Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

	\param fd file descriptor
	\param request request
	\param argp argument
	\returns result from ioctl
*/
int xioctl(int fd, int request, void* argp) {
	int r;

	do {
		r = v4l2_ioctl(fd, request, argp);
	}
	while (-1 == r && EINTR == errno);

	return r;
}

/**
	process image read
*/
inline void imageProcess(const void *p, uint8_t *rawImage) {
	// unsigned char* dst = malloc(co->width * co->height * 3 * sizeof(char));

	YUV420toYUV444(co->width, co->height, (unsigned char*) p, rawImage);
}

/**
	read single frame
*/
int frameRead(uint8_t *rawImage) {
	struct v4l2_buffer buf;

	switch (co->iom) {
		case IO_METHOD_READ:
			if (-1 == v4l2_read(fd, buffers[0].start, buffers[0].length)) {
				switch (errno) {
					case EAGAIN:
						return 0;

					case EIO:
						// Could ignore EIO, see spec.
						// fall through

					default:
						errno_exit("read");
				}
			}

			imageProcess(buffers[0].start, rawImage);
		break;

		case IO_METHOD_MMAP:
			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;

			if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
				switch (errno) {
					case EAGAIN:
						return 0;

					case EIO:
						// Could ignore EIO, see spec
						// fall through

					default:
						errno_exit("VIDIOC_DQBUF");
				}
			}

			assert(buf.index < n_buffers);

			imageProcess(buffers[buf.index].start, rawImage);

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
				errno_exit("VIDIOC_QBUF");
			}

			break;

		case IO_METHOD_USERPTR:
			CLEAR (buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;

			if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
				switch (errno) {
					case EAGAIN:
						return 0;

					case EIO:
						// Could ignore EIO, see spec.
						// fall through

					default:
						errno_exit("VIDIOC_DQBUF");
				}
			}

			uint32_t i;

			for (i = 0; i < n_buffers; ++i) {
				if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length == buffers[i].length) {
					break;
				}
			}

			assert (i < n_buffers);

			imageProcess((void *)buf.m.userptr, rawImage);

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
				errno_exit("VIDIOC_QBUF");
			}

			break;
	}

	return 1;
}

/**
	mainloop: read frames and process them
*/
void captureLoop(uint8_t *rawImage) {	
	int count = 3;
	uint8_t numberOfTimeouts = 0;

	struct timeval tv;
	int r;

	while (count-- > 0) {
		for (;;) {
			fd_set fds;

			// NOLINTNEXTLINE(readability-isolate-declaration)
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);

			if (r == -1) {
				if (EINTR == errno) {
					continue;
				}

				errno_exit("select");
			}

			if (r == 0) {
				if (numberOfTimeouts <= 0) {
					count++;
				} else {
					fprintf(stderr, "select timeout\n");
					exit(EXIT_FAILURE);
				}
			}

			if (frameRead(rawImage)) {
				break;
			}

			/* EAGAIN - continue select loop. */
		}
	}
}



/**
  start capturing
*/
void captureStart() {
	uint32_t i;
	enum v4l2_buf_type type;

	switch (co->iom) {
		case IO_METHOD_READ:
			/* Nothing to do. */
			break;

		case IO_METHOD_MMAP:
			for (i = 0; i < n_buffers; ++i) {
				struct v4l2_buffer buf;

				CLEAR(buf);

				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;

				if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
					errno_exit("VIDIOC_QBUF");
				}
			}

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
				errno_exit("VIDIOC_STREAMON");
			}

			break;

		case IO_METHOD_USERPTR:
			for (i = 0; i < n_buffers; ++i) {
				struct v4l2_buffer buf;

				CLEAR(buf);

				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_USERPTR;
				buf.index = i;
				buf.m.userptr = (unsigned long) buffers[i].start;
				buf.length = buffers[i].length;

				if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
					errno_exit("VIDIOC_QBUF");
				}
			}

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
				errno_exit("VIDIOC_STREAMON");
			}

			break;
	}
}

/**
	stop capturing
*/
void captureStop() {
	enum v4l2_buf_type type;

	switch (co->iom) {
		case IO_METHOD_READ:
			/* Nothing to do. */
			break;

		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
				errno_exit("VIDIOC_STREAMOFF");
			}

			break;
	}
}

/**
	initialize device
*/
void deviceInit() {
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_streamparm frameint;
	uint32_t min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",co->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",co->deviceName);
		exit(EXIT_FAILURE);
	}

	switch (co->iom) {
		case IO_METHOD_READ:
			if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
				fprintf(stderr, "%s does not support read i/o\n",co->deviceName);
				exit(EXIT_FAILURE);
			}
			break;

		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
				fprintf(stderr, "%s does not support streaming i/o\n",co->deviceName);
				exit(EXIT_FAILURE);
			}
		break;
	}

	/* Select video input, video standard and tune here. */
	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
				case EINVAL:
					/* Cropping not supported. */
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	} else {
		/* Errors ignored. */
	}

	CLEAR(fmt);

	// v4l2_format
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = co->width;
	fmt.fmt.pix.height = co->height;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
		errno_exit("VIDIOC_S_FMT");
	}

	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUV420) {
		fprintf(stderr,"Libv4l didn't accept YUV420 format. Can't proceed.\n");
		exit(EXIT_FAILURE);
	}

	/* Note VIDIOC_S_FMT may change width and height. */
	if (co->width != fmt.fmt.pix.width) {
		co->width = fmt.fmt.pix.width;
		fprintf(stderr,"Image width set to %i by device %s.\n", co->width, co->deviceName);
	}

	if (co->height != fmt.fmt.pix.height) {
		co->height = fmt.fmt.pix.height;
		fprintf(stderr,"Image height set to %i by device %s.\n", co->height, co->deviceName);
	}
	
	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	
	if (fmt.fmt.pix.bytesperline < min) {
		fmt.fmt.pix.bytesperline = min;
	}

	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	
	if (fmt.fmt.pix.sizeimage < min) {
		fmt.fmt.pix.sizeimage = min;
	}

	initIo(fmt.fmt.pix.sizeimage);
}

void deviceUninit() {
	uint32_t i;

	switch (co->iom) {
		case IO_METHOD_READ:
			free(buffers[0].start);
			break;

		case IO_METHOD_MMAP:
			for (i = 0; i < n_buffers; ++i) {
				if (-1 == v4l2_munmap(buffers[i].start, buffers[i].length)) {
					errno_exit("munmap");
				}
			}

			break;

		case IO_METHOD_USERPTR:
			for (i = 0; i < n_buffers; ++i) {
				free(buffers[i].start);
			}

			break;
	}

	free(buffers);
}

void initIo(uint32_t buffer_size) {
	switch (co->iom) {
		case IO_METHOD_READ:
			readInit(buffer_size);
			break;

		case IO_METHOD_MMAP:
			mmapInit();
			break;

		case IO_METHOD_USERPTR:
			userptrInit(buffer_size);
			break;
	}
}

void readInit(uint32_t buffer_size) {
	buffers = calloc(1, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		fprintf (stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

void mmapInit(void) {
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = VIDIOC_REQBUFS_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", co->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", co->deviceName);
		exit(EXIT_FAILURE);
	}

	buffers = calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			errno_exit("VIDIOC_QUERYBUF");
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = v4l2_mmap(NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) {
			errno_exit("mmap");
		}
	}
}

void userptrInit(uint32_t buffer_size) {
	struct v4l2_requestbuffers req;
	uint32_t page_size;

	page_size = getpagesize();
	buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

	CLEAR(req);

	req.count = VIDIOC_REQBUFS_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support user pointer i/o\n", co->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = calloc(4, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = memalign(/* boundary */ page_size, buffer_size);

		if (!buffers[n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}


/**
	open device
*/
void deviceOpen() {
	struct stat st;

	// stat file
	if (-1 == stat(co->deviceName, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", co->deviceName, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// check if its device
	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", co->deviceName);
		exit(EXIT_FAILURE);
	}

	// open device
	fd = v4l2_open(co->deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

	// check if opening was successfull
	if (fd == -1) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", co->deviceName, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/**
	close device
*/
void deviceClose() {
	if (-1 == v4l2_close(fd)) {
		errno_exit("close");
	}

	fd = -1;
}



void captureInit(camOpt *coIn) {
	co = coIn;

	if (initialized) {
		perror("warn: Capture is already init");
		return;
	}

	initialized = true;

	// open and initialize device
	deviceOpen();
	deviceInit();
}

void capture(camOpt *coIn, uint8_t *rawImage) {
	if (!initialized) {
		captureInit(coIn);
	}

	// start capturing
	captureStart();

	// process frames
	captureLoop(rawImage);

	// stop capturing
	captureStop();
}

void captureUninit() {
	if (!initialized) {
		perror("warn: Capture is already uninit");
		return;
	}

	initialized = false;

	// close device
	deviceUninit();
	deviceClose();
}