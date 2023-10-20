// Application layer protocol header.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_


typedef struct {
    int fileDescriptor; /* File Descriptor correspondent to the serial port */
    int status; /* Transmitter | Receiver */
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

#endif // _APPLICATION_LAYER_H_
