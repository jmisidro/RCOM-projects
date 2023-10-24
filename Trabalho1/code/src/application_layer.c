// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{    
    // store linklayer info in ll struct to be used in the link layer protocol
    strcpy(ll.serialPort, serialPort);
    if (strcmp("tx", role) == 0)
        ll.role = LlTx;
    else if (strcmp("rx", role) == 0)
        ll.role = LlRx;
    else {
        perror("Invalid role");
        return;
    }
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;

    // File Transfer
    if (ll.role == LlTx) {
        if (sendFile(filename) < 0) {
            printf("\nxxxxxx sendFile failed xxxxxx\n\n");
            return;
        }
    }
    else {
        if (receiveFile(filename) < 0) {
            printf("\nxxxxxx receiveFile failed xxxxxx\n\n");
            return;
        }
    }
}


int receiveFile(const char *filename)
{
    struct timeval start, end;

    srand(time(0));

    // store file descriptor in ApplicationLayer struct
    al.fileDescriptor = llopen();
    if (al.fileDescriptor == -1) {
        printf("\nxxxxxx llopen failed xxxxxx\n\n");
        return -1;
    }
    else 
        printf("\n------ llopen complete ------\n\n");


    int packetSize;
    unsigned char packetBuffer[MAX_SIZE_PACK], data[MAX_SIZE_DATA];


    // register start time
    gettimeofday(&start , NULL);

    usleep(60000);
    packetSize = llread(al.fileDescriptor, packetBuffer);
    if (packetSize < 0)
        return -1;

    int numBitsReceived = 0;

    numBitsReceived += packetSize * 8;

    int packet_filesize_start, packet_filesize_end; // used to compare the filesize stored in the packet
    char packet_filename_start[255], packet_filename_end[255]; // used to compare the filename stored in the packet

    // received START control packet
    if (packetBuffer[0] == CTRL_START) {
        if (parseControlPacket(packetBuffer, &packet_filesize_start, packet_filename_start) < 0)
            return -1;
    }
    else
        return -1;


    // creates and opens file to write on
    FILE* fp;
    fp = fopen(filename, "w");
    
    if (fp == NULL) {
        perror(filename);
        return -1;
    }

    int sequenceNumber, expectedSequenceNumber = 0, dataLength;

    printf("\n------ Receiving file ... ------\n\n");

    // read throught received data packets (file data) until receiving the END packet
    while (TRUE)
    {
        usleep(60000);
        packetSize = llread(al.fileDescriptor, packetBuffer);
        if (packetSize < 0)
            return -1;

        numBitsReceived += packetSize * 8;


        if (packetBuffer[0] == CTRL_DATA) {  // received data packet

            if (parseDataPacket(packetBuffer, data, &sequenceNumber) < 0)
                return -1;

            if (expectedSequenceNumber != sequenceNumber) {
                printf("receiveFile ERROR: Sequence number does not match\n");
                return -1;
            }

            expectedSequenceNumber = (expectedSequenceNumber + 1) % 256;

            dataLength = packetSize - 4;

            // writes to the file the content read from the serial port
            if (fwrite(data, sizeof(unsigned char), dataLength, fp) != dataLength) {
                return -1;
            }
        }
        // received END packet, indicating the end of the file transfer
        else if (packetBuffer[0] == CTRL_END)
            break;
    }

    printf("\n------ Finished receiving file! ------\n\n");

    gettimeofday(&end, NULL);
    // get the difference in seconds, multiply by a million
    double transferTime = (end.tv_sec - start.tv_sec) * 1e6;
    // then add the difference in microseconds, finally divide by a million to convert result to seconds
    transferTime = (transferTime +(end.tv_usec - start.tv_usec)) * 1e-6;

    printf("\n------ Protocol Effiency ------\n\n");

    printf("Number of bits received =  %d\n", numBitsReceived);
    printf("File transfer time (seconds) =  %lf\n", transferTime);
    double R =  numBitsReceived/transferTime;
    double baudRate = 38400.0;
    double S = R / baudRate;

    printf("\n\nReceived bitrate R = %lf", R);
    printf("\n\nLink capacity (Baudrate) C = %lf", baudRate);
    printf("\n\nEffiency statistic S (S = R / C) = %lf\n\n", S);


    if (getFileSize(fp) != packet_filesize_start) {
        printf("receiveFile ERROR: file size specified in start packet does not match received file size\n");
        return -1;
    }

    if (parseControlPacket(packetBuffer, &packet_filesize_end, packet_filename_end) < 0)
        return -1;

    // checks if filesize and filename are the same in START and END packets
    if((packet_filesize_start != packet_filesize_end) || (strcmp(packet_filename_start, packet_filename_end) != 0)){
        printf("receiveFile ERROR: Specified info (filename and/or filesize) in START and END packets does not match");
        return -1;
    }

    if (llclose(al.fileDescriptor) == -1) {
        printf("\nxxxxxx llclose failed xxxxxx\n\n");
        return -1;
    }
    else
        printf("\n------ llclose complete ------\n\n");

    if (fclose(fp) != 0)
        return -1;

    return 0;
}

int sendFile(const char *filename) 
{
    // opens file to read from
    FILE* fp;
    fp = fopen(filename, "r");
    
    if (fp == NULL) {
        perror(filename);
        return -1;
    }

    // store file descriptor in ApplicationLayer struct
    al.fileDescriptor = llopen();
    if (al.fileDescriptor == -1) {
        printf("\nxxxxxx llopen failed xxxxxx\n\n");
        return -1;
    }
    else 
        printf("\n------ llopen complete ------\n\n");


    unsigned char packetBuffer[MAX_SIZE_PACK];
    int fileSize = getFileSize(fp);

    int packetSize = buildControlPacket(packetBuffer, CTRL_START, fileSize, filename);

    // sends control START packet, to indicate the start of the file transfer
    if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0) {
        fclose(fp);
        return -1;
    }

    unsigned char data[MAX_SIZE_DATA];
    int read_length;
    int sequenceNumber = 0;

    printf("\n------ Sending file ... ------\n\n");

    // read throught the file to be sent
    while (TRUE) {

        read_length = fread(data, sizeof(unsigned char), MAX_SIZE_DATA, fp);

        if (read_length != MAX_SIZE_DATA) {
            if (feof(fp)) { // reached end of file

                packetSize = buildDataPacket(packetBuffer, sequenceNumber, data, read_length);
                sequenceNumber = (sequenceNumber + 1) % 256;
                
                // sends the last data frame
                if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0) {
                    fclose(fp);
                    return -1;
                }

                break;
            }
            else {
                perror("ERROR while reading file data");
                return -1;
            }
        }

        packetSize = buildDataPacket(packetBuffer, sequenceNumber, data, read_length);
        sequenceNumber = (sequenceNumber + 1) % 256;
    
        // sends a data frame
        if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0) {
            fclose(fp);
            return -1;
        }
       
    }

    packetSize = buildControlPacket(packetBuffer, CTRL_END, fileSize, filename);

    // sends control END packet, indicating the end of the file transfer
    if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0) {
        fclose(fp);
        return -1;
    }

    printf("\n------ File has been sent! ------\n\n");

    if (llclose(al.fileDescriptor) == -1) {
        printf("\nxxxxxx llclose failed xxxxxx\n\n");
        return -1;
    }
    else
        printf("\n------ llclose complete ------\n\n");

    if (fclose(fp) != 0)
        return -1;

    return 0;
}