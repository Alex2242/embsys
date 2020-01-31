#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "utils.h"

static const char *exportPath = "/sys/class/gpio/export";
static const char *unexportPath = "/sys/class/gpio/unexport";

static bool exported = false;
static bool ledActivated = false;
static uint8_t pinNumber = 0;
//NOLINTNEXTLINE(readability-magic-numbers)
static char buff[128];

void exportGpio() {
    if (pinNumber) {
        snprintf(buff, sizeof(buff), "%d", pinNumber);
        int i = strlen(buff);
        int fExport = open(exportPath, O_WRONLY);
        write(fExport, buff, sizeof(buff));
        close(fExport);
        exported = true;
    }
    else {
        snprintf(buff, sizeof(buff), "Led pin number %d is invalid", pinNumber);
        logging(LOG_ERR, buff);
    }
}

void unexport() {
    snprintf(buff, sizeof(buff), "%d", pinNumber);
    int fExport = open(unexportPath, O_WRONLY);
    write(fExport, buff, sizeof(buff));
    close(fExport);
    exported = false;
}

void flipLed() {
    snprintf(buff, sizeof(buff), "/sys/class/gpio/gpio%d/value", pinNumber);
    int fExport = open(buff, O_WRONLY);
    ledActivated ? write(fExport, "0", sizeof(buff)) : write(fExport, "1", sizeof(buff));
    ledActivated = !ledActivated;
    close(fExport);
}

void useGpio(uint8_t pn) {
    pinNumber = pn;
    exportGpio();
}

void disableGpio() {
    if (exported) {
        unexport();
    }
}