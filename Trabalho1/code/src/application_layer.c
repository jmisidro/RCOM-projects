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
    /* llopen */
    // store linklayer info in ll struct
    strcpy(ll.serialPort, serialPort);
    if (strcmp("tx", role) == 0)
        ll.role = LlTx;
    else if (strcmp("rx", role) == 0)
        ll.role = LlRx;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;
    al.fileDescriptor = llopen(ll);
    if (al.fileDescriptor == -1) {
        printf("\nxxxxxx llopen failed xxxxxx\n\n");
        return;
    }
    else 
        printf("\n------ llopen complete ------\n\n");

    /* llclose */ 
    if (llclose(al.fileDescriptor) == -1) {
        printf("\nxxxxxx llclose failed xxxxxx\n\n");
        return;
    }
    else
        printf("\n------ llclose complete ------\n\n");
}
