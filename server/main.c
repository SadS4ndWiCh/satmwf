#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#include "sock.h"
#include "protocol.h"

int main(void) {
    int server_fd = setup_sock(INADDR_LOOPBACK, 3000);

    int client_fd = accept(server_fd, NULL, NULL);

    u8 buffer[MESSAGE_HEADER_LENGTH + MESSAGE_PAYLOAD_MAX];
    if (recv(client_fd, buffer, sizeof(u16), 0) == -1) {
        printf("ERROR: failed to receive length.\n");
        return 1;
    }

    if (recv(client_fd, &buffer[sizeof(u16)], sizeof(u8), 0) == -1) {
        printf("ERROR: failed to receive type.\n");
        return 1;
    }

    u16 length = 0;
    memcpy(&length, buffer, sizeof(u16));

    if (recv(client_fd, &buffer[sizeof(u16) + sizeof(u8)], length, 0) == -1) {
        printf("ERROR: failed to receive payload.\n");
        return 1;
    }

    struct Message msg;
    Message_fromBytes(&msg, buffer);

    printf("Length: %d | Type: %#02x\n", msg.length, msg.type);

    if (msg.type == MCON) {
        struct CONMessage *con = (struct CONMessage *) msg.payload;
        printf("Nick: %s\n", con->nick);
    }

    close(server_fd);
    return 0;
}