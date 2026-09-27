
#ifndef SOCKHANDLER_H_INCLUDED
#define SOCKHANDLER_H_INCLUDED

#include "eHandle.h"

int LSockInit(void);
void SendData(void *Data, int_fast32_t Size, struct sockaddr_in *SendNode);
char *ReceiveData(struct sockaddr_in *SendNode, uint_fast32_t *ReturnSize, void *Buffer);

__attribute__((noreturn)) void ShitBricks(void); // Eternal assumed

#endif
