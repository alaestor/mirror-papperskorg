#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "..\..\YUKI mod dolan\yuki.h"
#include "..\..\YUKI mod dolan\memman.h"
#include "..\..\YUKI mod dolan\yukistate.h"

#define YP_MAGIC 14

// What are we gonna test on the bed?     =3 pomf

yuki_stfunc_t Connect0;
yuki_stfunc_t Connect1;
yuki_stfunc_t Connect2;
yuki_dhfunc_t Default;


int_least32_t main(int_least32_t argc, int_least8_t **argv)
{
    printf(" Pomf:// Hello world!\n");

    ///memman debug
    memman_init(1); // args are fine, sets state (pushes state)
    fprintf(stderr, "Made it past memman\n");
    uint32_t PoolIndex = memman_getpool(); //gets state
    fprintf(stderr, "Made it past memman\n");
    memman_setactivepool(PoolIndex);
    fprintf(stderr, "Made it past memman\n"); //sets state
    memman_reservepool(10000); // anti-pattern, pushes state
    fprintf(stderr, "Made it past memman\n");
//
//    int8_t *Data = memman_allocpool(5);
//    strncpy((char *)Data, "Hi\n", 4);
//
//    printf((char *)Data);
//
//    memman_destroy();
//
//    return 0;

    int_least32_t TempMaxPeer = 1;

    // Stackblock0 Start
    {
        struct WSAData wDat/*ass*/;
        WSAStartup(MAKEWORD(2,1), &wDat);

        struct yuki_addrcouple AddrCouple;
        AddrCouple.AddrVersion.IPv4.sin_family = AF_INET;
        AddrCouple.AddrVersion.IPv4.sin_port = htons(1337);
        AddrCouple.AddrVersion.IPv4.sin_addr.s_addr = INADDR_ANY;
        AddrCouple.Mode = AF_INET;

        struct yuki_peerlist *restrict List = yuki_createlist(&TempMaxPeer, &AddrCouple, 0, PoolIndex);
        yuki_setactivelist(List);
    }

    fprintf(stderr, "A case of the derps\n");

    yuki_setmagic(0x1337);

    // Initiate and use Windows Events (for wait-on-read)

    WSAEVENT Event = WSACreateEvent();
    WSAEventSelect(yuki_getendpoint(), Event, FD_READ);
    // WaitForSingleObject(Event, INFINITE); proto-call

    fprintf(stderr, "A case of the events\n");

    struct yuki_basenode *restrict NewDesc = yuki_getdescriptor();

    fprintf(stderr, "Descriptors, pls.\n");

    // initial state for connection...see ".h" for substate chain (new form of documentation)

    yuki_setstate(NewDesc, YUKI_CONNECT_INIT);

    yuki_setaddressmode(NewDesc, YUKI_IPV4);

    // register handshake behavior
    yuki_registerstatehandler(Connect0, 0); // syn
    yuki_registerstatehandler(Connect1, 1); // ack
    yuki_registerstatehandler(NULL, 2); // finish
    yuki_registerdefaulthandler(Default);

    fprintf(stderr, "state my sets\n");

    if (yuki_setaddressIPv4(NewDesc,
                            (int8_t const *const)"74.79.19.46",
                            (int8_t const *const)"1337") == 1)
    {
        const struct sockaddr_in *Temp = (struct sockaddr_in *)(NewDesc->Addresses);

        printf("heyyoooo success!\n Address: %s\n Port: %i\n",
               inet_ntoa(Temp->sin_addr),
               ntohs(Temp->sin_port));
    }
    else
        printf("heyyoooo suckedsess.... =( \n ");

    do
    {
        yuki_execstate(); // press the go button, yuki is allowed to do send behavior.  State is modified to YUKI_CONNECT_WAITACK
        Sleep(10);
    } while (1);



    // yuki is asynchronous, thus it returns immediately after the send
    // wait for incoming HELLO_ACK by checking the state of the socketw

    return 0;
}

__attribute__((fastcall)) yuki_stfuncret_t Connect0(yuki_stfuncarg0_t Arg)
{
    struct yuki_staticpacket *restrict Packet = (struct yuki_staticpacket *restrict)Arg->NodePacketBuffer;

    fprintf(stderr, "Handshaking Peer\n");

    Packet->Magic = yuki_getmagic();
    Packet->Command = YPC_HELLO;
    Arg->BufferUsed = sizeof(*Packet);

    sendto(yuki_getendpoint(), (const char *)Packet, sizeof(*Packet), 0, &Arg->Addresses[0], sizeof(*Arg->Addresses));

    yuki_setstateflags(Arg, YUKI_NODE_ACCEPT_DATA);

    return YUKI_CONNECT_WAIT;
}

__attribute__((fastcall)) yuki_stfuncret_t Connect1(yuki_stfuncarg0_t Arg)
{
    struct yuki_peerlist *CurrentList = yuki_getactivelist();
    yuki_endpoint_t EndPoint = yuki_getendpoint();

    struct sockaddr_in *Addresses = (struct sockaddr_in *)Arg->Addresses;

    int_least32_t Command;

    if (Addresses[0].sin_addr.S_un.S_addr == CurrentList->ListAddressBuffer.sin_addr.S_un.S_addr)
    {
        Command = yuki_validate();

        struct yuki_staticpacket *restrict const Packet =
            (struct yuki_staticpacket *restrict const)Arg->NodePacketBuffer;

        uint32_t const PacketBytes = Arg->BufferUsed;

        if (Command == YPC_HELLOACK)
        {
            memcpy(Packet, CurrentList->ListPacketBuffer, PacketBytes);

            fprintf(stderr, "Got a response from peer, finishing.\n");
            Packet->Command = YPC_HELLOFIN;

            sendto(EndPoint,
                   (const char *)Packet,
                   PacketBytes,
                   0,
                   (const struct sockaddr *)&Addresses[0],
                   sizeof(*Addresses));

            yuki_setglobalstate(YUKI_STATE_USED_BUFFER);

            return YUKI_CONNECT_SUCCESS;
        }
    }

    return YUKI_CONNECT_WAIT;
}

__attribute__((fastcall)) void Default(void)
{
    struct yuki_peerlist const *restrict const CurrentList = yuki_getactivelist();

    struct yuki_staticpacket const *restrict const Packet =
        (struct yuki_staticpacket const *restrict const)CurrentList->ListPacketBuffer;

    uint16_t Command = yuki_validate();

    /*
     documentation: "we say ...uhm"

    This function will assume and handle a new connection/node/peer/dude/member_of_BOGSAT
    by checking existing basenodes first and then adding new basenodes as needed.

    Upon creation of a basenode, the state of the base node needs to be set such that
    the next execution of yuki_execstate begins the process of filling in the basenodes properties.

    As we have not yet implemented a hashbrowns, i mean map,
    lookups will become increasingly inefficient as more peers are added.
        ^This will be changed (for the better (hopefully))
    */
}






