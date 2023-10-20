// Application layer protocol header.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include <string.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include "macros.h"
#include "ll_aux.h"
#include "link_layer.h"
#include "app_aux.h"

typedef struct {
    int fileDescriptor; /* File Descriptor correspondent to the serial port */
} ApplicationLayer;

// global variables
ApplicationLayer al;

/**
 * Application layer main function.
 * @param serialPort: Serial port name (e.g., /dev/ttyS0).
 * @param role: Application role {"tx", "rx"}.
 * @param baudrate: Baudrate of the serial port.
 * @param nTries: Application role {"tx", "rx"}.
 * @param timeout: Maximum number of frame retries.
 * @param filename: Name of the file to send / receive.
 */
void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);



/**
 * Function to receive a file, that was sent using the serial port
 * @param connectionParameters LinkLayer struct with information for the connection to be opened
 * @return 0 when sucess; negative value when error
 */
int receiveFile(const char *filename);

/**
 * Function to send a file, using the serial port
 * @param filename Name of the file to be sent through the serial port
 * @return 0 when sucess; negative value when error
 */
int sendFile(const char *filename);

#endif // _APPLICATION_LAYER_H_
