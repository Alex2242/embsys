#ifndef API_H
#define API_H

#include <stdint.h>

enum opcode {success, fail, readImg, sendImg, syn, ack, shutdownServ, disconnect};

typedef enum opcode opcode;

struct Message {
    opcode op;
    // size of the requested ressource in bytes
    // 0 means no ressource is to be expected
    uint64_t expectedSize;
};

typedef struct Message Message;

#endif
