#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>

#include "config.h"


// minimum number of buffers to request in VIDIOC_REQBUFS call
#define VIDIOC_REQBUFS_COUNT 2

typedef enum {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
} io_method;

struct buffer {
    void *                  start;
    size_t                  length;
};

struct camOpt {
    char *deviceName;
    char *jpegFilename;
    io_method iom;
    int jpegQuality;
    uint32_t width;
    uint32_t height;
};

typedef struct camOpt camOpt;

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_FPS 30
#define DEFAULT_JQAL 70


void captureInit(camOpt *coIn);
void capture(camOpt *coIn, uint8_t *rawImage);
void captureUninit();

#endif