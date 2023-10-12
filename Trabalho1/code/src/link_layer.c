// Link layer protocol implementation

#include <termios.h>
#include "macros.h"
#include "aux.h"
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd;
    struct termios oldtio;
    if ( (fd = openNonCanonical(connectionParameters, oldtio, VTIME_VALUE, VMIN_VALUE)) == -1)
        return -1;

    if(connectionParameters.role == LlTx)
        stateMachineTx(fd);
    else if(connectionParameters.role == LlRx)
        stateMachineRx(fd);

    if ( (closeNonCanonical(fd, oldtio)) == -1)
        return -1;

    return 1;
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
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
