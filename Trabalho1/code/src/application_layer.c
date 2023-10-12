// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "macros.h"
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // llopen
    LinkLayer connection;
    strcpy(connection.serialPort, serialPort);
    if (strcmp('tx', role) == 0)
        connection.role = LlTx;
    else if (strcmp('rx', role) == 0)
        connection.role = LlRx;
    connection.baudRate = baudRate;
    connection.nRetransmissions = nTries;
    connection.timeout = timeout;
    llopen(connection);
}
