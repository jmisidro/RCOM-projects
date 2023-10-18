// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "macros.h"
#include "ll_aux.h"
#include <string.h>
#include <stdio.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // llopen
    int fd;

    // store linklayer info in ll struct
    strcpy(ll.serialPort, serialPort);
    if (strcmp("tx", role) == 0)
        ll.role = LlTx;
    else if (strcmp("rx", role) == 0)
        ll.role = LlRx;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;
    fd = llopen(ll);
    if (fd == -1) {
        printf("llopen failed\n");
        return;
    }
    else 
        printf("------ llopen complete ------\n");

    // llclose
    if (llclose(fd) == -1) {
        printf("llclose failed\n");
        return;
    }
    else
        printf("------ llclose complete ------\n");
}
