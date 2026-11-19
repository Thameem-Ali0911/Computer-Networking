// UDP client: reads a line from the keyboard, sends it to the server
// as one datagram, then reads one reply datagram and prints it.
#include <stdio.h>
#include <string.h>
#include "nw.h"

#define RMAX 4000
#define WMAX 1000
#define CMAX 1 // the server we talk to

typedef struct Connection Connection;
struct Connection {
    int fd;
    sockaddr_in addr; // server's address, used on every send
};
Connection conn[CMAX];

int xgets(char* data, int size, int cid) {
    int n = recvfrom(conn[cid].fd, data, size - 1, 0, NULL, NULL);
    data[n > 0 ? n : 0] = '\0';
    return n;
}

int xputs(const char* data, int cid) {
    return sendto(conn[cid].fd, data, strlen(data), 0,
                  (sockaddr*)&conn[cid].addr, sizeof(sockaddr_in));
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
        if (status < 0) return -1; // 0 is an empty datagram, not an error

        fputs(rline, stdout);
        fflush(stdout);
    }
    return 0;
}

int main() {
    int cid = 0;
    conn[cid].fd = xsocket(SOCK_DGRAM);
    conn[cid].addr = (sockaddr_in){ // no connect: this address goes with each datagram
        .sin_family = AF_INET,
        .sin_port = htons(9999),
        .sin_addr.s_addr = inet_addr("127.0.0.1")}; // server's IP address

    processmain(cid);

    close(conn[cid].fd);
    return 0;
}
