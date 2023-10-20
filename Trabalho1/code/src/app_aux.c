// Application layer protocol auxiliary funtions

#include "app_aux.h"

int buildDataPacket(unsigned char *packetBuffer, int sequenceNumber, unsigned char *dataBuffer, int dataLength) {

    packetBuffer[0] = CTRL_DATA;

    packetBuffer[1] = (unsigned char)sequenceNumber;

    int l1, l2;
    // number of octets (K = 256 * L2 + L1) in the data field
    l1 = dataLength % 256;
    l2 = dataLength / 256;

    packetBuffer[2] = (unsigned char)l2;

    packetBuffer[3] = (unsigned char)l1;

    for (int i = 0; i < dataLength; i++)
        packetBuffer[i + 4] = dataBuffer[i];

    return dataLength + 4;
}

int parseDataPacket(unsigned char *packetBuffer, unsigned char *data, int *sequenceNumber) {
    
    // checks if the control field is correct 
    if (packetBuffer[0] != CTRL_DATA)
        return -1;

    *sequenceNumber = (int)packetBuffer[1];

    // number of octets (K = 256 * L2 + L1) in the data field
    int dataLength = 256 * (int)packetBuffer[3] + (int)packetBuffer[2];

    for (int i = 0; i < dataLength; i++)
    {
        data[i] = packetBuffer[i + 4];
    }

    return 0;
}

int buildControlPacket(unsigned char *packetBuffer, unsigned char controlByte, int fileSize, char *fileName) {

    packetBuffer[0] = controlByte;

    packetBuffer[1] = TYPE_FILESIZE; // T1

    int length = 0, currentFileSize = fileSize;

    // cicle to separate file size in bytes (V1)
    while (currentFileSize > 0){
        int rest = currentFileSize % 256;
        int div = currentFileSize / 256;
        length++;

        // shifts all bytes to the right, to make space for the new byte
        for (unsigned int i = 2 + length; i > 3; i--)
            packetBuffer[i] = packetBuffer[i - 1];

        packetBuffer[3] = (unsigned char)rest;

        currentFileSize = div;
    }

    packetBuffer[2] = (unsigned char)length;  // L1

    packetBuffer[3 + length] = TYPE_FILENAME; // T2

    packetBuffer[4 + length] = (unsigned char)(strlen(fileName) + 1); // file name length (including '\0) (L2)
    
    int fileNameStart = 5 + length; // beginning of V2

    for (unsigned int j = 0; j < (strlen(fileName) + 1); j++)  // strlen(fileName) + 1 in order to add the '\0' char
        packetBuffer[fileNameStart + j] = fileName[j];
    

    return 3 + length + 2 + strlen(fileName) + 1; // total length of the packet
}

int parseControlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName) {

    // checks if the control field is correct 
    if (packetBuffer[0] != CTRL_START && packetBuffer[0] != CTRL_END)
        return -1;

    int length1, length2;

    if (packetBuffer[1] == TYPE_FILESIZE) { // T1

        *fileSize = 0;
        length1 = (int)packetBuffer[2]; // L1

        for (int i = 0; i < length1; i++) // V1
            *fileSize = *fileSize * 256 + (int)packetBuffer[3 + i];
    }
    else
        return -1;

    int fileNameStart = 5 + length1;

    if (packetBuffer[fileNameStart - 2] == TYPE_FILENAME) { // T2

        length2 = (int)packetBuffer[fileNameStart - 1]; // L2

        for (int i = 0; i < length2; i++) // V2
            fileName[i] = packetBuffer[fileNameStart + i];
    }
    else
        return -1;

    return 0;
}
