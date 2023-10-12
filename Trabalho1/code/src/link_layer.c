// Link layer protocol implementation

#include "macros.h"
#include "aux.h"
#include "link_layer.h"

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd;

    printf("Opening connection...\n");
    // Open non canonical connection
    if ( (fd = openNonCanonical(connectionParameters, VTIME_VALUE, VMIN_VALUE)) == -1)
        return -1;

    // Run state machine to ensure the establishment phase
    if(connectionParameters.role == LlTx)
        stateMachineTx(connectionParameters, fd);
    else if(connectionParameters.role == LlRx)
        stateMachineRx(connectionParameters, fd);

    return fd;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(LinkLayer connectionParameters, int fd)
{
    printf("Closing connection...\n");
    
    if ( (closeNonCanonical(connectionParameters.oldtio, fd)) == -1)
        return -1;

    return 1;
}
