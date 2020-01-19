#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>

#include "api.h"
#include "network.h"

Message* request(Message req, int fd) {
    sendData(fd, &req, sizeof(Message));
    struct Message *rep = readData(fd, sizeof(Message));

    return rep;
}


Message* receiveMsg(int fd) {
    struct Message *msg = readData(fd, sizeof(Message));

    return msg;
}