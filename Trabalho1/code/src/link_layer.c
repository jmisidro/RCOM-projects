// Link layer protocol implementation

#include "link_layer.h"

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd, returnFd;

    printf("Opening connection...\n");
    // Open non canonical connection
    if ( (fd = openNonCanonical(VTIME_VALUE, VMIN_VALUE)) == -1)
        return -1;

    // installs alarm handler
    alarmHandlerInstaller();

    if (connectionParameters.role == LlTx) // Transmitter
    {
        returnFd = llOpenTransmitter(fd);
        if (returnFd < 0) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }
        else
            return returnFd;
    }
    else if (connectionParameters.role == LlRx) // Receiver
    {
        returnFd = llOpenReceiver(fd);
        if (returnFd < 0) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }
        else
            return returnFd;
    }

    perror("Invalid role");
    closeNonCanonical(oldtio, fd);
    return -1;
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
int llclose(int fd)
{
    if (ll.role == LlTx) {
        if (llCloseTransmitter(fd) < 0) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }
    }
    else if (ll.role == LlRx) {
        if (llCloseReceiver(fd) < 0) { 
            closeNonCanonical(oldtio, fd);
            return -1;
        }
    }
    else {
        perror("Invalid role");
        return -1;
    }

    // Open non canonical connection
    if (closeNonCanonical(oldtio, fd) == -1)
        return -1;

    return 1;
}
