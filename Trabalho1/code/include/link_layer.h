// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <termios.h>
#include "macros.h"
#include "ll_aux.h"

typedef enum
{
    LlTx, /* Transmitter */
    LlRx, /* Receiver */
} LinkLayerRole;

typedef struct
{
    char serialPort[50]; /* Dispositivo /dev/ttySx, x = 0, 1 */
    LinkLayerRole role; /*  Papel desempenhado na transmissão: Transmitter ou Receiver */
    int baudRate; /* Velocidade de transmissão */
    unsigned int nRetransmissions;/* Número de tentativas em caso de erro */
    unsigned int timeout; /* Valor do temporizador em segundos */
    unsigned char frame[MAX_SIZE_FRAME]; /* Trama */
    unsigned int frame_length; /* Tamanho atual da trama */
    unsigned int sequenceNumber; /* Número de sequência da trama: 0, 1 */

} LinkLayer;

// global variables
LinkLayer ll;
struct termios oldtio;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(int fd, const unsigned char *buf, int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(int fd, unsigned char *packet);

// Close previously opened connection.
// con includes fd and oldtio struct.
// fd is the File descriptor that was opened with the given port.
// Return "1" on success or "-1" on error.
int llclose(int fd);

#endif // _LINK_LAYER_H_
