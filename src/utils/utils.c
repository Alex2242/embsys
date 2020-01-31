#include <stdint.h>
#include <stdio.h>
#include <syslog.h>

#include "utils.h"


static uint8_t logOutput = STDOUT;

void setLogOutput(uint8_t lo) {
    logOutput = lo;
}

void logStdout(uint8_t logLevel, char* message) {
    switch (logLevel) {
        case LOG_INFO:
            printf("info: ");
            break;

        case LOG_WARNING:
            printf("warn: ");
            break;
        
        case LOG_ERR:
            printf("error: ");
            break;

        default:
            perror("error: Unhandled log level, message is: ");
            break;
    }

    printf("%s\n", message);
}

void logging(uint8_t logLevel, char* message) {
    switch (logOutput) {
        case SYSLOG:
            syslog(logLevel, message);
            break;

        case STDOUT:
            logStdout(logLevel, message);
            break;        


        default:
            perror("Unhandled log output");
            break;
    }
}