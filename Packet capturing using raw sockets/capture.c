// Experiment : Packet capturing application using raw sockets.
// Frame layout (offsets in bytes):
//   0- 5 Ethernet destination MAC     12-13 EtherType (0x0800 = IPv4)
//   6-11 Ethernet source MAC
// IPv4 header starts at byte 14:
//  16-17 total length                 26-29 source IP address
//     23 protocol (1 ICMP, 6 TCP, 17 UDP)  30-33 destination IP address
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

unsigned char rdata[64 * 1024];

void print(const char* mesg, unsigned char* data, int begin, int end) {
    printf("%s: ", mesg);
    for (int i = begin; i <= end; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}
void process(unsigned char* data, int len) {
    printf("Len: %d\n", len);
    print("Eth to", data, 0, 5);
    print("Eth from", data, 6, 11);
    print("Eth Datatype", data, 12, 13);
    if (data[12] == 0x08 && data[13] == 0x00) { // IPv4
        print("IP from", data, 26, 29);
        print("IP to", data, 30, 33);
        print("IP Datatype", data, 23, 23);
        print("IP Total length", data, 16, 17);
    }
    printf("\n");
    fflush(stdout);
}

int main() {
    int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // see man packet
    if (fd < 0) { perror("Error"); exit(1); }

    while (1) {
        int len = recv(fd, rdata, sizeof(rdata), 0);
        if (len < 0) { perror("Error"); exit(1); }
        process(rdata, len);
    }

    close(fd);
    return 0;
}
