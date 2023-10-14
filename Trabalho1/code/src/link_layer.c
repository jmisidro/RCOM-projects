// Link layer protocol implementation

#include "macros.h"
#include "ll_aux.h"
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
            llclose(fd);
            return -1;
        }
        else 
            return returnFd;
    }
    else if (connectionParameters.role == LlRx) // Receiver
    {
        returnFd = llOpenReceiver(fd);
        if (returnFd < 0) { 
            llclose(fd);
            return -1;
        }
        else 
            return returnFd;
    }

    perror("Invalid role");
    llclose(fd);
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
    printf("Closing connection...\n");
    
    if ( (closeNonCanonical(oldtio, fd)) == -1)
        return -1;

    return 1;
}
