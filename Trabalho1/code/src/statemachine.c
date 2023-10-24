#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "ll_aux.h"
#include "statemachine.h"
#include "macros.h"


int is_expected(unsigned char byte, state_machine* sm) {
  for (int i = 0; i < sm->expectedBytesLength; i++) {
    if (sm->expectedBytes[i] == byte)
      return i;
  }

  return -1;
}


state_machine* create_state_machine(unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte) {
    state_machine* sm = malloc(sizeof(state_machine));
    sm->state = START;
    sm->expectedBytes = expectedBytes;
    sm->expectedBytesLength = expectedBytesLength;
    sm->addressByte = addressByte;
    sm->dataLength = 0;
    return sm;
}


void event_handler(state_machine* sm, unsigned char byte, unsigned char* frame, int mode) {

    static int i = 0;

    if(mode ==  SUPERVISION){
      switch(sm->state) {

      case START:
          if (byte == FLAG) {
              sm->state = FLAG_RCV;
              frame[0] = byte;
          }
          break;

      case FLAG_RCV:
          if (byte == FLAG)
              break;
          else if (byte == sm->addressByte) {
              sm->state = A_RCV;
              frame[1] = byte;
          }
          else
              sm->state = START;
          break;

      case A_RCV:
          if (byte == FLAG)
              sm->state = FLAG_RCV;
          else {
              int n;
              if ((n = is_expected(byte, sm))>=0){
                sm->state = C_RCV;
                sm->foundIndex = n;
                frame[2] = byte;
              }
              else
                sm->state = START;
          }
          break;

      case C_RCV:
          if (byte == createBCC(frame[1], frame[2])){
              sm->state = BCC_OK;
              frame[3] = byte;
          }

          else if (byte == FLAG)
              sm->state = FLAG_RCV;
          else
              sm->state = START;
          break;

      case BCC_OK:
          if (byte == FLAG){
              sm->state = STOP;
              frame[4] = byte;
          }
          else
              sm->state = START;
          break;

      default:
          break;

    }
  }

    else if(mode == INFORMATION){

    switch(sm->state) {

        case START:
            i = 0;
            if (byte == FLAG) {
                sm->state = FLAG_RCV;
                frame[i++] = byte;
            }
            break;

        case FLAG_RCV:
            if (byte == FLAG)
                break;
            else if (byte == sm->addressByte) {
                sm->state = A_RCV;
                frame[i++] = byte;
            }
            else {
                sm->state = START;
                i = (int) sm->state;
            }
            break;

        case A_RCV:
            if (byte == FLAG) {
                sm->state = FLAG_RCV;
                i = (int) sm->state;
            }
            else {
              if (is_expected(byte, sm) >= 0){
                sm->state = C_RCV;
                frame[i++] = byte;
              }
              else {
                sm->state = START;
                i = (int) sm->state;
              }
            }
            break;

        case C_RCV:
            if (byte == createBCC(frame[1], frame[2])){
                sm->state = BCC_OK;
                frame[i++] = byte;
            }
            else {
              if (byte == FLAG)
                sm->state = FLAG_RCV;
              else
                sm->state = START;

              i = (int) sm->state;
            }
            break;

        case BCC_OK:
            if(byte ==  FLAG){
              frame[i] = byte;
              sm->state = STOP;
              sm->dataLength = i-4;
            }
            else{
              frame[i++] = byte;
            }
            break;


        default:
            break;

    }
  }


}

void destroy_state_machine(state_machine* sm) {
  free(sm);
}