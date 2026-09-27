/*****************************************************************************
 *                                                                           *
 *   Copywrong (C) 1337-9001 Hooin Kyoma <Kyoma@FutureGadgetLab.fake>        *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 9 of the License, or       *
 *   (at your option) any licence you want. I dun give fucks.                *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more lulcats.                            *
 *                                                                           *
 *   You shouldn't have received a copy of the GNU General Public License    *
 *   along with this program; if not, dont write to the                      *
 *   Free Software Foundation Inc. because you may be retarded.              *
 *                                                                           *
 *   Divergence 1.048596, Future Gadget Lab, Chiyoda-ku KuramaeBashi Douri   *
 *****************************************************************************/

/*     //                                                                                 /\/\
//     // let the opening comments: BEGIN!!! --plz read all, or scriptkitty will be sad  (,p_p) "meowster;
*/     //                                                                                 /__|`         plz wuv me..."


/*
//           _______
//　　 　 　  　　/ヾ::::::ヽ､
//　　 　  　 　く__＞:::::ヽ＞
//　　  　　　　  /　 ●　　　● |
//  　　　　　　 (    ( _●_) ミ 　　　CUM-ON NSA; LETS DO DIS!
//　　　　　　  彡､　,､､|∪|,/
//　　　　  　　/　　ヽ   ヽノ　ヽ
//　　  　　　 |_   'ﾟl===}]_\_＿＿______/|        ,         ,`     ,      .          \\\\
//  fﾆﾆﾆﾆllﾆ| 　 ＼[ l===ﾆﾆl]}||||||]l|====fﾆｺ ------+|]:{O----<C-----<Q----<0----~@||>
//  \l_,,=-'''\　  ＼..''ニ「_l⌒l｡__｡_]三i三三iF `         `'     `.      '         ////
//  　　　　　　〈,,/ヽ__＿）||[   `ー'
//
//                                               -=ShotsFired=-
//
*/

/*\
------------------------OVERVIEW

=The YUKI network protocol;

FG-01 Edition:IV v2.67 patch:20586 build:3;
Developed by the FUTURE GADGET LAB;
GNU public lisence, open source.




=Objective:

The FGL is developing the "YUKI network protocol"
to disrupt the evil agenda of SERN and The Organization,
ensuring peace and freedom throughout this worldline,
until the FGL is eventually the supreme ruler it.
--We will dominate the world before you can.




=Developers and Maintainers:

-Houoin Kyoma
-Hack(er)
-Kyoma's hostage (scriptkitty)
-stdio.h

------------------------END
\*/


/*//WE HERE AT THE FGL LIKE COMMENTS AND CAPSLOCK*/
//yo dawg i heard you like /*comments*/ so we //commented your /*comment*/ so you can // while you /**/


/// Notes concerning this source file:
//   Currently, these functions assume perfectly formatted data.
//   They DO NOT validate internal data. Use them carefully.
//   Several things are GCC specific, but we have supplied c99 compliant alternitives.

/// TODO:
// Yuki peerlist cleaning
// Yuki mem allocation & cleaning
// Backcode for windows 64bit and Linux 32+64bit compatibility
// Code the inner working of yuki

/*\ TODO Extended
YUKI Protocol Major Todo list v1.2 mod 8

Current summary:

    As it stands currently, YUKI is in a state where we have a working dispatch system
(this is the callback mechanism we made for both data and header command arguments).
I suspect that at some point, we'll have to change that around slightly to make more sense,
such as figuring out if we even need an actual callback mechanism for, say, purely data.

    The one thing I personally want to make sure of is that we wrangle YUKI's abilities.
What we don't want is YUKI being a catch-all framework for everything we do.  Like you said,
inter-operability and compartmentalization is key here.

    Going forward, things we'll have to really think about are the elements of YUKI that
will act "statefully", i. e. in such a way that it remembers things passed to it.  One of
the domains where this will be common is the routing engine that we plan to have in it.



[ ] - Memory Manager (memman_*)

    [X] - Initialization of memman's resources.  This includes pool structures and
          any counters we need to use.

    [X] - Pools need to act as reservers, meaning that the memory they allocate is treated
          as a granted resource that is _immutable_.  This allows for severe optimization,
          and the pros outweigh the cons (slightly less efficient with space).

    [X] - The actual allocatable structures need to contain enough state to describe their
          size, and which pool they belong to.  This makes getting and referencing memory
          extremely fast (in fact as fast possible), but makes freeing and resizing a pain
          in the ass.  That's acceptable, as good engine design appreciates better run-time
          versus indexing setup time.

    [ ] - Rectification and compression.


[ ] - YUKI General

    Notes: We've already completed the serial optimzations (and formulated a pattern to
           use them) for inlining certain "behavior functions", like the get/set types
           we'll make in the future.  Same thing with making YUKI have a base static
           member.  This replicates C++'s "class" behavior, which is optimal here.

    [ ] - Local peer grouping.  Essentially, this is making a simple peer-mesh, identical
          to the way GunZ does connections:  Everyone is connected to Everyone in that
          mesh.  The challenge, of course, is on the addressing side of things.  Since
          YUKI is supposed to be a layer over IP to establish an application-layer route
          table, that inherently suffers if there is no "tier 1 IP router" equivalent
          that can dole out routes.  I have a creative way to solve this problem, see {1}.

          Further, completing this requires refinement of the YUKI-state manager, contained
          inside all the yuki_basenodes.

        [ ] - Base Node State Engine.  This is the user-define state machine.  Since
                this interacts directly with YUKI state (and is the main overhead cost),
                the runtable part of the BNSE will be done in assembly language.  Yes,
                be very afraid.

        [ ] - UDP Punchthrough System.  We went over this before, but this is a pretty
                nifty feature.  Leaving it out would be detrimental to the goal.  All
                methods will be covered; direct guess peering (the non-aggressive router
                pattern), overhead guess (one router does port bashing), and tunnelling
                (when up against two aggressive port bashers).  Fully symmetric NATs
                cannot be punched, and so for that we'll need:

            [ ] - Inter-peer group connection bridges.

    [ ] - Local peer group bridges.  This is the natural extension of the above; allowing
          peered groups to connect to other peered groups.  Completing this means refining
          route behavior.

          [ ] - YUKI Subnet Route Engine.  This is essentially an exact mirror of using
                OSPF (Open Shortest Path First) to establish BGP style routes.  The
                main difference being that the "weight" used to determine "shortest" is
                left of to the user of YUKI.  We HAVE to get this right in order for
                YUKI to work like we want it to.  The YSRE (as it will henceforth be
                called) IS optional, in the beginning.  For full on dream-game realization,
                it's required.


 {1} : agreeeing upon a network number is as simple as starting at the lowest possible.
 For the peer group bridges, conflicting subnets can be resolved by a game of chance.
 Have the connector (the network trying to bridge) do a rand() to see who's network number stands up.


 ALSO

 // Guidelines for use of the forbidden "goto"
 //
 // (1) never go against the flow of logic/code
 // (2) never make a loop
 //
 // if any rules are broken, the guilty must commit seppuku.

 \*/

#define _WIN32_WINNT 0x0601

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
//#include "SockHandler.h"
//#include "eHandle.c"
#include "yuki.h"
#include "yukistate.h"
#include "memman.h"

#define SWAG gay
#define YUKI_TEST_MAGIC 1337


// GLOBAL //
struct yuki_engine YukiEngine;
//---------//


/// MaxPeer is neg sign if error <- outdated, UPDATE
struct yuki_peerlist *yuki_createlist(int_fast32_t *MaxPeer,
                             struct yuki_addrcouple *AddrCouple,
                             yuki_endpoint_t TempSock,
                             uint32_t MemoryPool)
{
    uint32_t BaseNodeArraySize =  *MaxPeer * sizeof(struct yuki_basenode);

    uint32_t ListSize = sizeof(struct yuki_peerlist)
                        + BaseNodeArraySize
                        + YCA_MAXPACKBUFFSIZE
                        + (sizeof(yuki_stfunc_t *) * YUKI_MAXSTATES);

    fprintf(stderr, "Hasty Allocation Station Previous\n");

    struct yuki_peerlist *PeerList = memman_allocpool(ListSize);

    fprintf(stderr, "Hasty Allocation Station\n");

    if (PeerList == NULL)
    {
        *MaxPeer = ER_YCL_NOMEM;

        return NULL;
    }

    PeerList->BaseNodeArray = (struct yuki_basenode *)(PeerList + 1);
    PeerList->MaxPeers = *MaxPeer;
    PeerList->CurrentPeers = 0;
    PeerList->ListSize = ListSize;
    PeerList->Subnet = 0;

    PeerList->ListPacketBuffer = ((uint8_t *)PeerList->BaseNodeArray) + BaseNodeArraySize;

    PeerList->YSTTable = (yuki_stfunc_t **)(PeerList->ListPacketBuffer + YCA_MAXPACKBUFFSIZE);
    PeerList->YSTTEntries = 0;

    // Stack block
    {
        struct yuki_basenode *BaseNodes = PeerList->BaseNodeArray;
        struct yuki_basenode *CurrentNode;

        for (int32_t Counter = 0; Counter < *MaxPeer; Counter++)
        {
            CurrentNode = &BaseNodes[Counter];
            CurrentNode->NodePacketBuffer = memman_allocpool(YUKI_STATE_BUFF_SIZE);
            CurrentNode->BufferSize = YUKI_STATE_BUFF_SIZE;
            CurrentNode->BufferUsed = 0;
        }
    }

    // Here; I store and organize your socks.

    /*
            /\ /\   _
           (=^_^=) / \    ScriptKitty:
            /:::\  ||`'         "Mew-socks!"
            |:::|__//
           *\_-_/__/
    */

    if (AddrCouple->Mode != YCL_ALLREADY)
    {
        TempSock = socket(AddrCouple->Mode, SOCK_DGRAM, IPPROTO_UDP);

        if (TempSock == (yuki_endpoint_t)INVALID_SOCKET)
        {
            *MaxPeer = ER_YCL_INVALSOCK;
            return NULL;
        }

        uint_least32_t OptVal = 1;

        if (setsockopt(TempSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&OptVal, sizeof(OptVal)) != 0)
        {
            *MaxPeer = ER_YCL_REUSEFAIL;
            return NULL;
        }

        if (ioctlsocket(TempSock, FIONBIO, (unsigned long int *)&OptVal) != 0)
        {
            *MaxPeer = ER_YCL_NOBLOCKFAIL;
            return NULL;
        }

        // binding a socket -- BDSM?
        int_least32_t CheckBind = bind(TempSock, (struct sockaddr const *)&AddrCouple->AddrVersion,
                                       (AddrCouple->Mode == AF_INET) ?
                                       sizeof(AddrCouple->AddrVersion.IPv4) : sizeof(AddrCouple->AddrVersion.IPv6));

        if (CheckBind != 0)
        {
            *MaxPeer = ER_YCL_BINDFAIL;
            return NULL;
        }
    }

    memcpy(&PeerList->AddrCouple, AddrCouple, sizeof(*AddrCouple));
    PeerList->EndPoint = TempSock;
    PeerList->YukiPool = MemoryPool;

    return PeerList;
}

int_least32_t yuki_validate(void)
{
    struct yuki_peerlist const *restrict const CurrentList = yuki_getactivelist();
    struct yuki_staticpacket const *restrict const CurrentPacket = (struct yuki_staticpacket const *restrict const)CurrentList->ListPacketBuffer;

    fprintf(stderr, "%u magical %u\n", CurrentPacket->Magic, CurrentList->Options.Magic);

    if (CurrentPacket->Magic != CurrentList->Options.Magic)
        return ER_YP_NOMAGIC;

    /*PRINTF'S ARE FOR DEBUG*/
    printf(" YUKI:// Packet MAGIC validated!\n");

    return CurrentPacket->Command;

    return ER_YP_NOCMD;
}

/// Debug
void send_the_D(struct sockaddr_in *addr)
{
    int8_t Data[16];
    struct yuki_staticpacket *HeaderData = (struct yuki_staticpacket *)Data;

    HeaderData->Magic = 14; // yay magic numbars
    HeaderData->Command = YPC_HELLO;

    strcpy((char *)(Data + sizeof(*HeaderData)), "Coffee?");

    sendto(YukiEngine.PeerLists->EndPoint, (char *)&Data, sizeof(Data), 0, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
}

void dat_magic(uint16_t Magic)
{
    YukiEngine.PeerLists->Options.Magic = Magic;
}

//void yuki_ezmodeinit(void)

///dem snippets
/*

//                       /// all uint8_t bitfields are (GCC)
//    uint8_t is_Free:1; // is entry mark for deletion?
//    uint8_t has_Route:1;
//    uint8_t has_Source:1;
//    uint8_t has_BaseMsg:1; // is Yuki ctl msg?
//    uint8_t has_NextRoute:1; // is manually routed?
//
//    // Yuki is on her period, we need pads.
//    uint8_t : 3; // anonymous bit field padding (GCC)


//static inline void yuki_addpeer(struct yuki_basenode *Node); // A transfer student has joined Yuki's class
//static inline void yuki_addpeer(struct yuki_basenode *Node)
//{
//    struct yuki_basenode *BaseNodeArray = YukiEngine.PeerLists->BaseNodeArray;
//    struct yuki_basenode *CurrentNode = &BaseNodeArray[Node->Node_ID];
//
//    memcpy(CurrentNode, Node, sizeof(struct yuki_basenode));
//}
    acertainmagical[];
*/
