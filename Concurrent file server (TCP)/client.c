// TCP client: reads a line from the keyboard, sends it to the server,
// then reads one reply from the server and prints it.
#include <stdio.h>
#include <string.h>
#include "nw.h"
#define RMAX 4000
#define WMAX 1000
#define CMAX 1 // the client has one connection: the server

typedef struct Connection Connection;
struct Connection {
    int fd;
};
Connection conn[CMAX];

int xgets(char* data, int size, int cid) {
    int n = recv(conn[cid].fd, data, size - 1, 0);
    data[n > 0 ? n : 0] = '\0';
    return n;
}

int xputs(const char* data, int cid) {
    return send(conn[cid].fd, data, strlen(data), 0);
}

int processmain(int cid) {
    while (1) {
        char wdata[WMAX];
        char rline[RMAX];
        char* line;
        int status;

        line = fgets(wdata, sizeof(wdata), stdin);
        if (line == NULL) return -1; // Ctrl-D: end of keyboard input

        xputs(wdata, cid);

        status = xgets(rline, sizeof(rline), cid);
        if (status <= 0) return -1; // server closed the connection

        fputs(rline, stdout);
        fflush(stdout);
    }
    return 0;
}

int main() {
    int cid = 0;
    conn[cid].fd = xsocket(SOCK_STREAM);
    xconnect(conn[cid].fd, "127.0.0.1", 9999); // server's IP address and port
    printf("Connected\n");

    processmain(cid);

    close(conn[cid].fd);
    printf("Connection closed\n");
    return 0;
}
