#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

#include "sock.h"
#include "protocol.h"

int main(void) {
    int server_fd = setup_sock(INADDR_LOOPBACK, 3000);

    struct CONMessage con = { "Caio" };
    struct Message msg = {
        .length  = sizeof(con),
        .type    = MCON,
        .payload = (u8 *) &con
    };

    if (Message_send(server_fd, &msg)) {
        printf("%s:%d ERROR: failed to send request connection message to server.\n", __FILE__, __LINE__);
        return 1;
    }

    printf("%s:%d INFO: request to connect the server: CON\n", __FILE__, __LINE__);

    struct Message con_result;
    if (Message_recv(server_fd, &con_result) == -1) {
        printf("%s:%d ERROR: failed to receive request connection reply.\n", __FILE__, __LINE__);
        return 1;
    }

    if (con_result.type == MFCN) {
        printf("%s:%d ERROR: failed to connect to chat room.\n", __FILE__, __LINE__);
        return 1;
    }

    if (con_result.type != MSCN) {
        printf("%s:%d ERROR: receive an unexpected message: %#02x\n", __FILE__, __LINE__, con_result.type);
        return 1;
    }

    u8 userid = ((struct SCNMessage *) con_result.payload)->id;
    printf("%s:%d INFO: successfuly joined the receives and receives the id: %d\n", __FILE__, __LINE__, userid);

    struct MSGMessage chat_message = {
        .author_id = userid,
        .message   = "Hello World!",
    };

    msg.length  = sizeof(chat_message);
    msg.type    = MMSG;
    msg.payload = (u8 *) &chat_message;

    if (Message_send(server_fd, &msg) == -1) {
        printf("ERROR: fail to send chat message.\n");
        return 1;
    }

    printf("%s:%d INFO: send chat message to server: '%s'\n", __FILE__, __LINE__, chat_message.message);

    msg.length  = 0;
    msg.type    = MDIS;
    msg.payload = NULL;

    if (Message_send(server_fd, &msg) == -1) {
        printf("ERROR: fail to request to disconnect. closing anyway...\n");
        close(server_fd);
    }

    printf("%s:%d INFO: request to disconnect the server: DIS\n", __FILE__, __LINE__);

    printf("INFO: disconnected from server.\n");

    return 0;
}