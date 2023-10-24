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
 * @param filename Name of the file to be saved, received through the serial port
 * @return 0 if it was sucessful; negative value otherwise
 */
int receiveFile(const char *filename);

/**
 * Function to send a file,using the serial port
 * @param filename Name of the file to be sent through the serial port
 * @return 0 if it was sucessful; negative value otherwise
 */
int sendFile(const char *filename);

#endif // _APPLICATION_LAYER_H_
