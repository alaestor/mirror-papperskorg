/*****************************************************************************
 *                                                                           *
 *   Copywrong (C) 1337-9001 Hooin Kyoma <Kyoma@FutureGadgetLab.net>         *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 9 of the License, or       *
 *   (at your option) any lisence you want. I dun give fucks.                *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more lulcats.                            *
 *                                                                           *
 *   You shouldn't have received a copy of the GNU General Public License    *
 *   along with this program; if not, dont write to the                      *
 *   Free Software Foundation Inc. because you may be retarded               *
 *                                                                           *
 *   Divergence 1.048596, Future Gadget Lab, Chiyoda-ku KuramaeBashi Douri   *
 *****************************************************************************/

// Here; I store and organize your socks.

/*
        /\ /\   _
       (=^_^=) / \    ScriptKitty:
        /:::\  ||`'         "Mew-socks!"
        |:::|__//
       *\_-_/__/
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <winsock2.h>









//
//
//
/// Ancient code, passed down from the Elders. Be warned; Here lay dragons and the crushed hopes of men.
//
//
//

/*

int LSockInit(void)
{
    printf(" Yuki.N:// LSockInit has began. Attempting to allocate socket.\n");

    int eCheck; // for error-return checking
    uint_fast32_t Lsock;
    WSADATA wDat; // wDatAss

    if(WSAStartup(MAKEWORD(2,2),&wDat) != 0) // check for failz
    {
        printf(" Yuki.N:// LSockInit ERROR!\n");
    }

    Lsock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); // Yo windows; you has my socks?

    if(Lsock == INVALID_SOCKET) // check for failz
    {
        printf(" Yuki.N:// LSockInit ERROR!\n");
    }

    int const sOption = 1;
    eCheck = setsockopt(Lsock, SOL_SOCKET, SO_REUSEADDR, (const void *)&sOption, sizeof(sOption));

    if(eCheck != 0) // check for failz
    {
        printf(" Yuki.N:// LSockInit ERROR!\n");
    }

    struct sockaddr_in LsockIN;

    LsockIN.sin_family = AF_INET;
    LsockIN.sin_port = htons(conf_LPort);
    LsockIN.sin_addr.S_un.S_addr = (conf_LIPAUTO) ? INADDR_ANY : inet_addr(conf_LIP);

    if(LsockIN.sin_addr.S_un.S_addr == INADDR_NONE) // check for failz
    {
        printf(" Yuki.N:// LSockInit ERROR!\n");;
    }


    eCheck = bind(Lsock,(struct sockaddr const *)&LsockIN,sizeof(LsockIN)); // binding a socket -- BDSM?

    if(eCheck != 0) // check for failz
    {
        printf(" Yuki.N:// LSockInit ERROR!\n");
    }

    printf(" Yuki.N:// LSockInit has completed. Lsock returned.\n");

    return Lsock;
}

void SendData(void *Data, int_fast32_t Size, struct sockaddr_in *SendNode)
{
    if(! InputLoop.NetState.WriteState)
    {
        return;
    }

    int Result = 0;

    do
    {
        Result = sendto(InputLoop.DriverData.Lsock, ((char *)Data)+Result, Size-Result, 0, (struct sockaddr *)SendNode, sizeof(struct sockaddr));

        if(Result == SOCKET_ERROR)
        {
            InputLoop.NetState.WriteState = 0;
            printf(" Yuki.N:// ERRORCODE FROM SENDTO: %i\n",WSAGetLastError());

            break;
        }
    }
    while(Size-Result > 0);

    return;
}

char *ReceiveData(struct sockaddr_in *SendNode, uint_fast32_t *ReturnSize, void *Buffer)
{
    int TempSize = sizeof(struct sockaddr);
    int RecvReturn;


    RecvReturn = recvfrom(InputLoop.DriverData.Lsock, (char *)Buffer, *ReturnSize, 0, (struct sockaddr *)SendNode, &TempSize);

    if(RecvReturn == SOCKET_ERROR)
    {
        //eLogger("ReceiveData","RecvFrom returned SOCKET_ERROR!"); //put this somewhere!
        printf(" Yuki.N:// RecvFrom returned a socket error!\n");

        RecvReturn = WSAGetLastError();

        switch(RecvReturn)
        {
            case WSAEWOULDBLOCK:
                break;

            case WSAEMSGSIZE:
                break;

            default:
                printf(" Yuki.N:// ERROR! RecvReturn: %i\n",RecvReturn);
        }

        return NULL;
    }

    *ReturnSize = RecvReturn;

    return (char *)Buffer;
}

*/









