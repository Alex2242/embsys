#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <syslog.h>

#define STDOUT 0
#define SYSLOG 1


void setLogOutput(uint8_t logOutput);
void logging(uint8_t logLevel, char* message);

#endif
