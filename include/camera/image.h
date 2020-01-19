#ifndef _YUV_H_
#define _YUV_H_

#include <stdint.h>

#include "camera.h"

void YUV420toYUV444(int width, int height, unsigned char* src, unsigned char* dst);
void jpegWrite(unsigned char* img, camOpt co);

#endif
