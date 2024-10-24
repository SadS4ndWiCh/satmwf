#include <netinet/in.h>

#include "sock.h"
#include "protocol.h"

int main(void) {
    int server_fd = setup_sock(INADDR_LOOPBACK, 3000);

    struct CONMessage con = { "Caio" };
    struct Message msg = {
        .length  = sizeof(con),
        .type    = MCON,
        .payload = &con
    };

    u8 buffer[MESSAGE_HEADER_LENGTH + MESSAGE_PAYLOAD_MAX];
    Message_toBytes(&msg, buffer);

    if (send(server_fd, buffer, MESSAGE_HEADER_LENGTH + MESSAGE_PAYLOAD_MAX, 0) == -1) {
        return 1;
    }

    return 0;
}