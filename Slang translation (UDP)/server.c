// Experiment : Expand "new generation" English to formal English (UDP).
#include <stdio.h>
#include <string.h>
#include "nw.h"

#define RMAX 1000
#define WMAX 4000 // expanded text is longer than the input
#define CMAX 1    // the client of the current request

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

int isdelim(char c) { return (c == ' ' || c == ',' || c == '.' || c == '\n'); }

void getword(char input[], char word[]) {
    int i;
    for (i = 0; input[i] != '\0' && !isdelim(input[i]); i++) {
        word[i] = input[i];
    }
    word[i] = '\0';
}

void processmain(int cid) {
    char input[RMAX] = "";
    char output[WMAX] = "";

    while (1) {
        xgets(input, sizeof(input), cid);
        output[0] = '\0'; // start a fresh reply for every request

        for (int i = 0; input[i] != '\0'; i++) {
            if (isdelim(input[i])) {
                sprintf(output + strlen(output), "%c", input[i]);
            } else {
                char word[100] = "";
                getword(&input[i], word);
                i = i + strlen(word) - 1;

                if (strcmp(word, "idc") == 0) {
                    sprintf(output + strlen(output), "%s", "I don't care");
                } else if (strcmp(word, "tbh") == 0) {
                    sprintf(output + strlen(output), "%s", "to be honest");
                } else if (strcmp(word, "ig") == 0) {
                    sprintf(output + strlen(output), "%s", "I guess");
                } else if (strcmp(word, "tbf") == 0) {
                    sprintf(output + strlen(output), "%s", "to be fair");
                } else if (strcmp(word, "atm") == 0) {
                    sprintf(output + strlen(output), "%s", "at the moment");
                } else if (strcmp(word, "irl") == 0) {
                    sprintf(output + strlen(output), "%s", "in real life");
                } else if (strcmp(word, "lol") == 0) {
                    sprintf(output + strlen(output), "%s", "laughing out loud");
                } else if (strcmp(word, "asap") == 0) {
                    sprintf(output + strlen(output), "%s", "as soon as possible");
                } else if (strcmp(word, "omg") == 0) {
                    sprintf(output + strlen(output), "%s", "oh my god");
                } else if (strcmp(word, "ttyl") == 0) {
                    sprintf(output + strlen(output), "%s", "talk to you later");
                } else if (strcmp(word, "idk") == 0) {
                    sprintf(output + strlen(output), "%s", "I don't know");
                } else if (strcmp(word, "nvm") == 0) {
                    sprintf(output + strlen(output), "%s", "never mind");
                } else {
                    sprintf(output + strlen(output), "%s", word);
                }
            }
        }

        xputs(output, cid);
    }
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
