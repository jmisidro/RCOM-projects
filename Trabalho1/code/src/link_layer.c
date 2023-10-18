// Link layer protocol implementation

#include "link_layer.h"

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd, returnFd;

    printf("llopen: Opening connection...\n");
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
int llwrite(int fd, const unsigned char *buf, int bufSize)
{
    unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to receive the response
    unsigned char controlByte; // controlByte --> information frame number 

    if (ll.sequenceNumber == 0)
        controlByte = I_0;
    else
        controlByte = I_1;

    if (createInformationFrame(ll.frame, controlByte, buffer, bufSize) != 0) {
        closeNonCanonical(oldtio, fd);
        return -1;
    }

    int fullLength; // frame length after stuffing

    if ((fullLength = byteStuffing(ll.frame, bufSize)) < 0) {
        closeNonCanonical(oldtio, fd);
        return -1;
    }

    ll.frame_length = fullLength;
    int numWritten; // number of writen characters

    int dataSent = FALSE; // indicates whether the data has been sent

    while (!dataSent)
    {
        if ((numWritten = sendFrame(ll.frame, fd, ll.frame_length)) == -1) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }

        printf("llwrite: Sent I frame\n");

        int read_value = -1;
        finish = FALSE;
        num_retr = 0;
        resendFrame = FALSE;

        alarm(ll.timeout);

        unsigned char expectedBytes[2];

        if (controlByte == I_0) {
            expectedBytes[0] = RR_1;
            expectedBytes[1] = REJ_0;
        }
        else if (controlByte == I_1) {
            expectedBytes[0] = RR_0;
            expectedBytes[1] = REJ_1;
        }

        while (finish != TRUE) {
            // read_value contains the index the expectedByte found by the state machine if it succeeds, else -1
            read_value = readSupervisionFrame(responseBuffer, fd, expectedBytes, 2, END_SEND);

            if (resendFrame) {
                sendFrame(ll.frame, fd, ll.frame_length);
                resendFrame = false;
            }

            if (read_value >= 0) {
                // Cancels alarm
                alarm(0);
                finish = TRUE;
            }
        }

        if (read_value == -1) {
            printf("llwrite: Closing file descriptor\n");
            closeNonCanonical(oldtio, fd);
            return -1;
        }

        if (read_value == 0) // read a RR
            dataSent = TRUE;
        else // read a REJ
            dataSent = FALSE;

        printf("llwrite: Received response frame, N = %d\n", responseBuffer[2]);
    }

    if (ll.sequenceNumber == 0)
        ll.sequenceNumber = 1;
    else if (ll.sequenceNumber == 1)
        ll.sequenceNumber = 0;
    else
        return -1;


    return (numWritten - 6); // length of the data packet sent to the receiver
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(int fd, unsigned char *packet)
{

  int numBytes;
  unsigned char expectedBytes[2];
  expectedBytes[0] = I_0;
  expectedBytes[1] = I_1;

  int read_value;

  int isBufferFull = FALSE;

  while (!isBufferFull) {

    read_value = readInformationFrame(ll.frame, fd, expectedBytes, 2, END_SEND);

    printf("llread: Received I frame\n");

    if ((numBytes = byteDestuffing(ll.frame, read_value)) < 0) {
      closeNonCanonical(oldtio, fd);
      return -1;
    }

    int controlByteRead;
    if (ll.frame[2] == I_0)
      controlByteRead = 0;
    else if (ll.frame[2] == I_1)
      controlByteRead = 1;

    unsigned char responseByte;

    if (ll.frame[numBytes - 2] == createBCC_2(&ll.frame[DATA_START], numBytes - 6)) { // checks if bcc2 is correct

        if (controlByteRead == ll.sequenceNumber) { // Expected frame (sequence number matches the number in control byte)  
            // transfers information to the buffer
            for (int i = 0; i < numBytes - 6; i++)
                packet[i] = ll.frame[DATA_START + i];

            isBufferFull = TRUE;
        }
        // updates the response and sequence number whether it was the Expected frame or a Duplicated frame
        //                                      (in which case, we discard the rest of the information)
        // Expected frame: store the information and request the next frame
        // Duplicated frame: request the next frame
        if (controlByteRead == 0) {
            responseByte = RR_1;
            ll.sequenceNumber = 1;
        }
        else {
            responseByte = RR_0;
            ll.sequenceNumber = 0;
        }
    }
    else { // if bcc2 is not correct

      if (controlByteRead != ll.sequenceNumber) { // duplicated frame; discards information and requests the next frame

        if (controlByteRead == 0) {
          responseByte = RR_1;
          ll.sequenceNumber = 1;
        }
        else {
          responseByte = RR_0;
          ll.sequenceNumber = 0;
        }
      }
      else { // ignores frame data (because of error) and rejects this frame

        if (controlByteRead == 0) {
          responseByte = REJ_0;
          ll.sequenceNumber = 0;
        }
        else {
          responseByte = REJ_1;
          ll.sequenceNumber = 1;
        }
      }
    }


    if (createSupervisionFrame(ll.frame, responseByte, LlRx) != 0) {
      closeNonCanonical(oldtio, fd);
      return -1;
    }

    ll.frame_length = BUF_SIZE_SUP;

    // send RR/REJ frame to receiver
    if (sendFrame(ll.frame, fd, ll.frame_length) == -1) {
      closeNonCanonical(oldtio, fd);
      return -1;
    }

    printf("llread: Sent response frame, N = %d\n", controlByteRead);

  }

  return (numBytes - 6); // number of bytes of the data packet read
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
