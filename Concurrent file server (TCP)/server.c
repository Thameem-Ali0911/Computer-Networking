// Experiment : Concurrent file server (TCP, one thread per client).
#include <pthread.h>
#include <stdint.h>
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
    pthread_t thread_id;
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

// Input:  Filename
// Output: PID SUCCESS/ERROR Filename, then the contents line by line
int processmain(int cid) {
    char rline[RMAX];
    char wdata[WMAX];
    char filename[100] = "";
    int pid = getpid();
    int status;

    // read filename
    status = xgets(rline, sizeof(rline), cid);
    if (status <= 0) return -1; // client left

    sscanf(rline, "%99s", filename);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        sprintf(wdata, "%d ERROR %s\n", pid, filename);
        xputs(wdata, cid);
        return -1;
    }

    sprintf(wdata, "%d SUCCESS %s\n", pid, filename);
    xputs(wdata, cid);

    // send the file line by line
    while (1) {
        char* line = fgets(rline, sizeof(rline), file);
        if (line == NULL) break; // end of file

        sleep(1); // slows output so that concurrency is visible
        xputs(rline, cid);
    }

    fclose(file);
    return 0; // the caller closes the connection: that marks the end
}

void* mythread(void* arg) {
    int cid = (int)(intptr_t)arg;

    processmain(cid); // one file request per connection

    close(conn[cid].fd);
    conn[cid].fd = -1;
    printf("Client %d disconnected\n", cid);
    return NULL;
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

        pthread_create(&conn[cid].thread_id, NULL, mythread, (void*)(intptr_t)cid);
        pthread_detach(conn[cid].thread_id);
    }
    return 0;
}
