#ifndef YUKI_H_INCLUDED
#define YUKI_H_INCLUDED

#define YCL_ALLREADY 1337


/// ERR CODES ///
#define ER_YCL_NOMEM (-1) // Program has alzheimers
#define ER_YCL_INVALSOCK (-2) // This is not your sock. Maybe the man next to you put it in your washingmachine.
#define ER_YCL_REUSEFAIL (-3) // Cannot reuse sock; has hole in it.
#define ER_YCL_NOBLOCKFAIL (-4) // Double negative? or error...
#define ER_YCL_BINDFAIL (-5) // No BDSM =(

#define ER_YCA_BADSIZE (-6) // Size matters to Yuki.
#define ER_YCA_RECVFAIL (-7) // Bitch got rejected.
#define ER_YCA_PACFAIL (-8) // I died a lot on PacMan.

#define ER_YP_NOMAGIC (-9) // Packet is a muggle.
#define ER_YP_NOCMD (-10) // ScriptKitty, the BDSM slave, is lost without commands from his meowster.
///----------///

/// PAC CMDS ///
#define YP_MAXCMDS 128 // The total number of possible commands in a yuki_cmdhandler (peerlist).
#define YP_DEFAULTCMDMAX 2 // Total number of default YUKI commands
//------------//
#define YPC_HELLO 0
#define YPC_HELLOACK 1
#define YPC_HELLOFIN 2
///----------///

#define YCA_MAXPACKBUFFSIZE 2048

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <stdio.h>
#include "yukitypes.h"

///----------/// Typedefs - Refer to "YUKI protocol 6 compilers ultimate error"
struct yuki_peerlist; // Forward declaration (structure "prototype") - 'cuz dependancy hell catch22 typedefs
///bomb this
typedef void (__attribute__((fastcall)) *yuki_cmdhandler_funcptr)(struct yuki_peerlist *);
typedef void (__attribute__((fastcall)) *yuki_datacallback_funcptr)(struct yuki_peerlist *);
///----------///


/// Forward Decl
struct yuki_addrcouple;
struct yuki_callbackarg;
struct yuki_options;

union yuki_addrs;

struct yuki_engine
{
    struct yuki_peerlist *PeerLists;
    struct yuki_peerlist *CurrentList;
};

///---------------
///---------------
///-----NOTE------
///---------------
///---------------
///
/// 1) fuk callbackdata <<< FOR SERIALS NEXT TIME
/// 2) put stuffs w/ magic in peerlist <<< FOR DOUBLE SERIALS NEXT TIME

struct yuki_peerlist
{
    uint32_t MaxPeers;
    uint32_t CurrentPeers;
    uint32_t ListSize;
    uint32_t Subnet;
    uint32_t YSTTEntries;
    yuki_stfunc_t **YSTTable;
    yuki_dhfunc_t *YSTTDefault;

    uint32_t YSTTFlags;

    yuki_endpoint_t EndPoint;
    uint8_t *ListPacketBuffer;
    struct sockaddr_in ListAddressBuffer;

    struct yuki_basenode *BaseNodeArray;

    uint32_t YukiPool;

    struct yuki_addrcouple
    {
        union yuki_addrs
        {
            struct sockaddr_in IPv4;
            struct sockaddr_in6 IPv6;
        } AddrVersion;

        uint32_t Mode;
    } AddrCouple;

    struct yuki_options
    {
        yuki_magic_t Magic;
    } Options;
};

struct yuki_basenode
{
    struct sockaddr Addresses[2];
    uint32_t Node_ID;
    yuki_state_t State;
    uint8_t *NodePacketBuffer;
    uint32_t BufferSize;
    uint32_t BufferUsed;
    uint16_t Mode;
    uint16_t StateFlags;
};

struct yuki_staticpacket /// Refer to "YUKI protocol 5 dolans ultimate diagram"
{
    uint16_t Magic;
    uint8_t Command;
}__attribute__((packed));

// we opted for GCC because #FuckTheSystem #FuckThePolice #BreakingBad #YOLO #Swag
/*
struct yuki_basenode // 16 bytes, but Yuki is still insecure. She wants to lose more bytes but doesnt know how.
{
    uint8_t IP_Address[8]; // IPv4|6
    uint32_t Node_ID;

    _Bool is_Free:1; // is entry mark for deletion
    _Bool is_IPv4:1; // IPv ? 4 : 6
    _Bool has_Route:1;
    _Bool has_Source:1;
    _Bool has_BaseMsg:1;
    _Bool has_NextRoute:1;

    // Yuki is on her period, we need pads.
    _Bool bit_padding_Because:1;
    _Bool bit_padding_Bool:1;
    uint8_t byte_padding[3];
}__attribute__((packed));
*/

extern struct yuki_engine YukiEngine;

/// Extern func prototypes
extern struct yuki_peerlist *yuki_createlist(int_fast32_t *MaxPeer,
                                             struct yuki_addrcouple *AddrCouple,
                                             yuki_endpoint_t TempSock,
                                             uint32_t MemoryPool);

extern int_least32_t yuki_validate(void);

/*DEBUG*/
extern void send_the_D(struct sockaddr_in *addr);
extern void dat_magic(uint16_t Magic);
//void WeArentQuiteSureWhatThisFuncDoesPlsTellUsIfYouKnow(void)

/// inline func decl
static inline void yuki_setactivelist(struct yuki_peerlist *restrict List);
static inline struct yuki_peerlist *yuki_getactivelist(void);
static inline struct yuki_basenode *yuki_getdescriptor(void);
static inline void yuki_setmagic(yuki_magic_t Magic);
static inline yuki_magic_t yuki_getmagic(void);
static inline void yuki_cleanup(void); // Yuki has friends coming over, and her place is a mess!!!


/// inline func defs
static inline void yuki_setactivelist(struct yuki_peerlist *restrict List)
{
    YukiEngine.CurrentList = List;
}

static inline struct yuki_peerlist *yuki_getactivelist(void)
{
    return YukiEngine.CurrentList;
}

static inline struct yuki_basenode *yuki_getdescriptor(void)
{
    struct yuki_peerlist *CurrentList = yuki_getactivelist();

    fprintf(stderr, "A case of the lels\n");

    if (CurrentList->CurrentPeers < CurrentList->MaxPeers)
    {
        return &CurrentList->BaseNodeArray[CurrentList->CurrentPeers++];
    }

    return NULL;
}

static inline void yuki_setmagic(yuki_magic_t Magic)
{
    struct yuki_peerlist *restrict const CurrentList = yuki_getactivelist();

    CurrentList->Options.Magic = Magic;
}

static inline yuki_magic_t yuki_getmagic(void)
{
    struct yuki_peerlist const *restrict const CurrentList = yuki_getactivelist();

    return CurrentList->Options.Magic;
}

static inline void yuki_cleanup(void)
{
    closesocket(YukiEngine.PeerLists->EndPoint);
    free(YukiEngine.PeerLists); /// UPDATE >> nope.memman
}

#endif // YUKI_H_INCLUDED
