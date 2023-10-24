// Application layer auxiliary file header.

#ifndef _APP_AUX_H_
#define _APP_AUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "macros.h"
#include "ll_aux.h"
#include "link_layer.h"


/**
 * Function that builds an application data packet
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @param sequenceNumber Sequence number of the packet
 * @param dataBuffer Buffer with the data to fill the packet
 * @param dataLength Length of the data in the buffer
 * @return Length of the packet buffer
 */
int buildDataPacket(unsigned char *packetBuffer, int sequenceNumber, unsigned char *dataBuffer, int dataLength);

/**
 * Function that parses the data packets
 * @param packetBuffer Buffer with the data packet
 * @param data Pointer to the file data packet extracted, to be returned by the function
 * @param sequenceNumber Pointer to the sequence number of the packet, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseDataPacket(unsigned char *packetBuffer, unsigned char *data, int *sequenceNumber);


/**
 * Function that builds a control packet
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @param controlByte Can be CTRL_START or CTRL_END, to show if the control packet indicates the beginning or end of the file
 * @param fileSize Size of the file, in bytes
 * @param fileName Name of the file
 * @return Length of the packet buffer
 */
int buildControlPacket(unsigned char *packetBuffer, unsigned char controlByte, int fileSize, const char *fileName);

/**
 * Function that parses the control packets
 * @param packetBuffer Buffer with the control packet
 * @param fileSize Pointer to the size of the file, to be returned by the function
 * @param fileName Pointer to the name of the file, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseControlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName);

/**
 * Auxiliary function to obtain the size of a file, from its file pointer
 * @param fp File pointer to the file
 * @return Size of the file in question
 */
int getFileSize(FILE *fp);


#endif // _LL_AUX_H_