// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "macros.h"
#include "aux.h"
#include <string.h>
#include <stdio.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // llopen
    int fd;

    strcpy(ll.serialPort, serialPort);
    if (strcmp("tx", role) == 0)
        ll.role = LlTx;
    else if (strcmp("rx", role) == 0)
        ll.role = LlRx;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;
    fd = llopen(ll);

    // llclose
    llclose(fd);
}
