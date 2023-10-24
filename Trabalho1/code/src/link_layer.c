// Link layer protocol implementation

#include "link_layer.h"

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen()
{
    int fd, returnFd;

    printf("llopen: Opening connection...\n");
    // Open non canonical connection
    if ( (fd = openNonCanonical(VTIME_VALUE, VMIN_VALUE)) == -1)
        return -1;

    // installs alarm handler
    alarmHandlerInstaller();

    if (ll.role == LlTx) // Transmitter
    {
        returnFd = llOpenTransmitter(fd);
        if (returnFd == -1) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }
        else
            return returnFd;
    }
    else if (ll.role == LlRx) // Receiver
    {
        returnFd = llOpenReceiver(fd);
        if (returnFd == -1) {
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
// LLOPEN - Receiver
////////////////////////////////////////////////
int llOpenReceiver(int fd)
{
  unsigned char expectedByte[1];
  ll.frame_length = BUF_SIZE_SUP;
  expectedByte[0] = SET;

  if (readSupervisionFrame(ll.frame, fd, expectedByte, 1, ADD_SEND) == -1)
    return -1;

  printf("llopen: Received SET frame\n");

  if (createSupervisionFrame(ll.frame, UA, LlRx) != 0)
    return -1;


  // send SET frame to receiver
  if (sendFrame(ll.frame, fd, ll.frame_length) == -1)
    return -1;

  printf("llopen: Sent UA frame\n");

  return fd;
}

////////////////////////////////////////////////
// LLOPEN - Transmitter
////////////////////////////////////////////////
int llOpenTransmitter(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP];
  ll.frame_length = BUF_SIZE_SUP;

  // creates SET frame
  if (createSupervisionFrame(ll.frame, SET, LlTx) != 0)
    return -1;


  // send SET frame to receiver
  if (sendFrame(ll.frame, fd, ll.frame_length) == -1)
    return -1;

  printf("llopen: Sent SET frame\n");

  int read_value = -1;
  finish = FALSE;
  num_retr = 0;
  resendFrame = FALSE;

  // Sets alarm
  alarm(ll.timeout);

  unsigned char expectedByte[1];
  expectedByte[0] = UA;

  while (finish != TRUE) {
    // read_value contains the index the expectedByte found by the state machine if it succeeds, else -1
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, ADD_SEND);
    if (resendFrame) {
      sendFrame(ll.frame, fd, ll.frame_length);
      resendFrame = FALSE;
    }

    if (read_value >= 0) {
      // Cancels alarm
      alarm(0);
      finish = TRUE;
    }
  }

  if (read_value == -1) {
    printf("llopen ERROR: Closing file descriptor\n");
    return -1;
  }

  printf("llopen: Received UA frame\n");

  return fd;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(int fd, unsigned char *packet, int length)
{
    unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to receive the response
    unsigned char controlByte; // controlByte --> information frame number 

    if (ll.sequenceNumber == 0)
        controlByte = I_0;
    else
        controlByte = I_1;

    if (createInformationFrame(ll.frame, controlByte, packet, length) != 0) {
        closeNonCanonical(oldtio, fd);
        return -1;
    }

    int fullLength; // frame length after stuffing

    if ((fullLength = byteStuffing(ll.frame, length)) < 0) {
        closeNonCanonical(oldtio, fd);
        return -1;
    }

    ll.frame_length = fullLength;
    int numBytesWritten; // number of bytes written

    int dataSent = FALSE; // indicates whether the data has been sent

    while (!dataSent)
    {
        if ((numBytesWritten = sendFrame(ll.frame, fd, ll.frame_length)) == -1) {
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
            read_value = readSupervisionFrame(responseBuffer, fd, expectedBytes, 2, ADD_SEND);

            if (resendFrame) {
                sendFrame(ll.frame, fd, ll.frame_length);
                resendFrame = FALSE;
            }

            if (read_value >= 0) {
                // Cancels alarm
                alarm(0);
                finish = TRUE;
            }
        }

        if (read_value == -1) {
            printf("llwrite ERROR: Closing file descriptor\n");
            closeNonCanonical(oldtio, fd);
            return -1;
        }

        if (read_value == 0) // read a RR
            dataSent = TRUE;
        else // read a REJ
            dataSent = FALSE;

        printf("llwrite: Received response frame, C = %x\n", responseBuffer[2]);
    }

    if (ll.sequenceNumber == 0)
        ll.sequenceNumber = 1;
    else if (ll.sequenceNumber == 1)
        ll.sequenceNumber = 0;
    else
        return -1;


    return (numBytesWritten - 6); // length of the data packet sent to the receiver
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(int fd, unsigned char *packet)
{

  int numBytesRead; // number of bytes read
  unsigned char expectedBytes[2];
  expectedBytes[0] = I_0;
  expectedBytes[1] = I_1;

  int read_value;

  int isBufferFull = FALSE;

  while (!isBufferFull) {

    read_value = readInformationFrame(ll.frame, fd, expectedBytes, 2, ADD_SEND);

    printf("llread: Received Info frame\n");

    if ((numBytesRead = byteDestuffing(ll.frame, read_value)) < 0) {
      closeNonCanonical(oldtio, fd);
      return -1;
    }


    int controlByteRead;
    if (ll.frame[2] == I_0)
      controlByteRead = 0;
    else if (ll.frame[2] == I_1)
      controlByteRead = 1;

    unsigned char responseByte;

    if (ll.frame[numBytesRead - 2] == createBCC_2(&ll.frame[DATA_START], numBytesRead - 6)) { // checks if bcc2 is correct

        if (controlByteRead == ll.sequenceNumber) { // Expected frame (sequence number matches the number in control byte)  
            // transfers information to the packet
            for (int i = 0; i < numBytesRead - 6; i++)
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

    printf("llread: Sent response frame, C = %x\n", ll.frame[2]);

  }

  return (numBytesRead - 6); // number of bytes of the data packet read
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int fd)
{
    if (ll.role == LlTx) {
        if (llCloseTransmitter(fd) == -1) {
            closeNonCanonical(oldtio, fd);
            return -1;
        }
    }
    else if (ll.role == LlRx) {
        if (llCloseReceiver(fd) == -1) { 
            closeNonCanonical(oldtio, fd);
            return -1;
        }
    }
    else {
        perror("Invalid role");
        return -1;
    }

    // Close non canonical connection
    if (closeNonCanonical(oldtio, fd) == -1)
        return -1;

    return 1;
}

////////////////////////////////////////////////
// LLCLOSE - Receiver
////////////////////////////////////////////////
int llCloseReceiver(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP], expectedByte[1];
  ll.frame_length = BUF_SIZE_SUP;
  expectedByte[0] = DISC;

  if (readSupervisionFrame(ll.frame, fd, expectedByte, 1, ADD_SEND) == -1)
    return -1;

  printf("llclose: Received DISC frame\n");

  // creates DISC frame
  if (createSupervisionFrame(ll.frame, DISC, LlRx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frame_length) == -1)
    return -1;

  printf("llclose: Sent DISC frame\n");

  int read_value = -1;
  finish = FALSE;
  num_retr = 0;
  resendFrame = FALSE;

  alarm(ll.timeout);

  expectedByte[0] = UA;

  while (finish != TRUE) {

    // read_value contains the index the expectedByte found by the state machine if it succeeds, else -1
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, ADD_REC);

    if (resendFrame)
    {
      sendFrame(ll.frame, fd, ll.frame_length);
      resendFrame = FALSE;
    }

    if (read_value >= 0)
    {
      // Cancels alarm
      alarm(0);
      finish = TRUE;
    }
  }

  if (read_value == -1) {
    printf("llclose ERROR: Closing file descriptor\n");
    return -1;
  }

  printf("llclose: Received UA frame\n");

  return 0;
}

////////////////////////////////////////////////
// LLCLOSE - Transmitter
////////////////////////////////////////////////
int llCloseTransmitter(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP], expectedByte[1];
  ll.frame_length = BUF_SIZE_SUP;

  // creates DISC frame
  if (createSupervisionFrame(ll.frame, DISC, LlTx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frame_length) == -1)
    return -1;

  printf("llclose: Sent DISC frame\n");

  int read_value = -1;
  finish = FALSE;
  num_retr = 0;
  resendFrame = FALSE;

  // Sets alarm
  alarm(ll.timeout);

  expectedByte[0] = DISC;

  while (finish != TRUE) {
    // read_value contains the index the expectedByte found by the state machine if it succeeds, else -1
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, ADD_REC);

    if (resendFrame) {
      sendFrame(ll.frame, fd, ll.frame_length);
      resendFrame = FALSE;
    }

    if (read_value >= 0) {
      // Cancels alarm
      alarm(0);
      finish = TRUE;
    }
  }

  if (read_value == -1) {
    printf("llclose ERROR: Closing file descriptor\n");
    return -1;
  }

  printf("llclose: Received DISC frame\n");

  // creates UA frame
  if (createSupervisionFrame(ll.frame, UA, LlTx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frame_length) == -1)
    return -1;

  printf("llclose: Sent UA frame\n");

  return 0;
}
