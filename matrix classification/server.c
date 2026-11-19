#include <stdio.h>
#include <string.h>
#include "nw.h"

#define RMAX 1000
#define WMAX 2000
#define CMAX 100

typedef struct Connection Connection;
struct Connection {
    int fd;
    sockaddr_in addr;
};
Connection conn[CMAX];
int nconn = 0;

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
        char rline[RMAX];
        char wdata[WMAX];
        int n = 0; // stays 0 if the line is not a number
        int status;

        // read N
        status = xgets(rline, sizeof(rline), cid);
        if (status <= 0) return -1;

        sscanf(rline, "%d", &n);

        // read the elements one by one and check them
        int upper = 1, lower = 1;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                sprintf(wdata, "Enter a[%d][%d]\n", i, j);
                xputs(wdata, cid);

                status = xgets(rline, sizeof(rline), cid);
                if (status <= 0) return -1;

                int value = 0; // stays 0 if the line is not a number
                sscanf(rline, "%d", &value);

                if (i > j && value != 0) upper = 0; // non-zero below the diagonal
                if (i < j && value != 0) lower = 0; // non-zero above the diagonal
            }
        }

        const char* type;
        if (upper && lower)
            type = "diagonal";
        else if (upper)
            type = "upper triangular";
        else if (lower)
            type = "lower triangular";
        else
            type = "not triangular";

        sprintf(wdata, "Matrix type: %s\n", type);
        xputs(wdata, cid);
    }
    return 0;
}

int main() {
    int fd = xsocket(SOCK_STREAM);
    xbind(fd, "0.0.0.0", 9999); // 0.0.0.0 = all local IP addresses
    listen(fd, 10);

    for (;;) {
        if (nconn == CMAX) return -1;
        int cid = nconn++;
        conn[cid].fd = xaccept(fd, &conn[cid].addr);
        printf("Client %d connected\n", cid);

        processmain(cid); // serve this client until it leaves

        close(conn[cid].fd);
        conn[cid].fd = -1;
        printf("Client %d disconnected\n", cid);
    }
    return 0;
}
