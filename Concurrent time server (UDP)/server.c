#include <stdio.h>
#include <string.h>
#include <time.h>
#include "nw.h"

#define RMAX 1000
#define WMAX 2000
#define CMAX 1 // the client of the current request

typedef struct Connection Connection;
struct Connection {
    int fd;           // the one server socket
    sockaddr_in addr; // who sent the current datagram
};
Connection conn[CMAX];

int xgets(char* data, int size, int cid) {
    socklen_t len = sizeof(sockaddr_in);
    int n = recvfrom(conn[cid].fd, data, size - 1, 0, (sockaddr*)&conn[cid].addr, &len);
    data[n > 0 ? n : 0] = '\0';
    return n;
}

int xputs(const char* data, int cid) {
    return sendto(conn[cid].fd, data, strlen(data), 0,
                  (sockaddr*)&conn[cid].addr, sizeof(sockaddr_in));
}

int processmain(int cid) {
    while (1) {
        char rline[RMAX];
        char wdata[WMAX];
        char command[100] = "";
        int status;

        status = xgets(rline, sizeof(rline), cid);
        if (status < 0) return -1; // 0 is an empty datagram, not an error

        printf("Request from %s:%d: %s", inet_ntoa(conn[cid].addr.sin_addr),
               ntohs(conn[cid].addr.sin_port), rline);

        sscanf(rline, "%99s", command);
        if (strcmp(command, "time") == 0)
            sprintf(wdata, "%ld\n", (long)time(0));
        else
            sprintf(wdata, "ERROR unknown command %s", rline);

        xputs(wdata, cid); // reply to the sender of this datagram
    }
    return 0;
}

int main() {
    int cid = 0;
    conn[cid].fd = xsocket(SOCK_DGRAM);
    xbind(conn[cid].fd, "0.0.0.0", 9999); // 0.0.0.0 = all local IP addresses
    // no listen, no accept: there are no connections in UDP

    processmain(cid);

    close(conn[cid].fd);
    return 0;
}
