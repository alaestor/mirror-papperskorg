#ifndef YUKISTATE_H_INCLUDED
#define YUKISTATE_H_INCLUDED

#include <stdint.h>
#include "yukitypes.h"
#include "yuki.h"

/// Defines
// Buffer Define
#define YUKI_STATE_TABLE_MAX 2048
#define YUKI_STATE_BUFF_SIZE 4096

// Return values
#define YUKI_STATE_FAIL 0
#define YUKI_STATE_OK 1
#define YUKI_STATE_CONTINUE 2
#define YUKI_STATE_ERR 0xFFFFFFFF


// Basic state defines
#define YUKI_MAXSTATES 2048
#define YUKI_CONNECT_INIT 0
#define YUKI_CONNECT_WAIT 1
#define YUKI_CONNECT_SUCCESS 2

// Global list state defines
#define YUKI_STATE_PACKET_INCOMING 0x00000001ul

// Basenode local state defines
#define YUKI_NODE_ACCEPT_DATA 0x00000001ul

// Net-type defines
#define YUKI_IPV4 4
#define YUKI_IPV6 6

/// Extern func prototypes
extern uint32_t yuki_setaddressIPv4(struct yuki_basenode *Desc,
                                    int8_t const *const Addr,
                                    int8_t const *const Port); // remmeber to stay const-istant from now on
extern void yuki_execstate(void);

/// inline func decl
static inline void yuki_setstate(struct yuki_basenode *Desc, uint32_t State);

// NOTES:  State Flags are OR'd witht he current values.
static inline void yuki_setstateflags(struct yuki_basenode *Desc, uint16_t StateFlags);
static inline yuki_state_t yuki_getstate(struct yuki_basenode *Desc);
static inline void yuki_setaddressmode(struct yuki_basenode *Desc, uint32_t Mode);
static inline yuki_endpoint_t yuki_getendpoint(void);

// NOTES:  State Handler handles two cases.  When Func is a valid function pointer,
// the index state is an "action" state.  If it's NULL, the index state is a
// completion state, with no associated function. (e.g. the end of a handshake).
static inline void yuki_registerstatehandler(yuki_stfunc_t *Func, uint32_t Index);

/// inline func defs
static inline void yuki_setstate(struct yuki_basenode *Desc, uint32_t State)
{
    Desc->State = State;
}

static inline void yuki_setstateflags(struct yuki_basenode *Desc, uint16_t StateFlags)
{
    Desc->StateFlags |= StateFlags;
}

static inline yuki_state_t yuki_getstate(struct yuki_basenode *Desc)
{
    return Desc->State;
}

static inline void yuki_setaddressmode(struct yuki_basenode *Desc, uint32_t Mode)
{
    Desc->Mode = Mode;
}

static inline yuki_endpoint_t yuki_getendpoint(void)
{
    return YukiEngine.CurrentList->EndPoint;
}

static inline void yuki_registerstatehandler(yuki_stfunc_t *Func, uint32_t Index)
{
    struct yuki_peerlist *restrict const CurrentList = yuki_getactivelist();
    CurrentList->YSTTable[Index] = (yuki_stfunc_t *)Func;
    CurrentList->YSTTEntries++;
}


#endif // YUKISTATE_H_INCLUDED
