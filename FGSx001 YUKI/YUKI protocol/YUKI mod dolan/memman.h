#ifndef MEMMAN_H_INCLUDED
#define MEMMAN_H_INCLUDED

#define NOMINMAX

#define WINVER 0x0601

#include <stdint.h>
#include <windows.h>
#include <stdio.h>

#include "yukitypes.h"

struct memman_pool;

struct memman_base
{
    struct memman_pool *ActivePool;

    uint32_t PoolCount;
    uint32_t PageSize;
    uint32_t LineSize;

    struct memman_pool
    {
        void *PoolData;
        void *CurrentAllocPos;
        uint32_t IsPoolInit;
        uint32_t Allocations;
        uint32_t ReserveRange;
        uint32_t AllocRange;
    } *Pools;
};

struct memman_allocation
{
    struct memman_pool *Pool;
    uint32_t Blocks; // Number of dirt (cacheline) blocks which we will use to build a house in minecraft

    uint8_t Padding[8];
}__attribute__((packed));

extern struct memman_base Base;

/// extern func prototypes
extern uint32_t memman_init(uint32_t PoolCountArg); // open the pool to the public
extern uint32_t memman_reservepool(uint32_t ReserveAmount); // grant VIP's special reservations to pools.
extern void *memman_allocpool(uint32_t AllocSize); // Swimmer sign-up sheet

/// inline func decl
static inline void memman_setactivepool(uint32_t Pool); // Limited life guards! Only use one pool at a time.
static inline uint32_t memman_getpool(void); // Fuck, I lost the pool. Where is it?!
static inline void memman_purgepool(void); // Someone pissed in the pool!
static inline void memman_purgeall(void); // Someone shat in the pool?!
static inline void memman_destroy(void); // Soviets invaded!! Don't let them get their hands on our secret pool technology! p.s. that escalated quickly

static inline uint32_t memman_parseslpi(SYSTEM_LOGICAL_PROCESSOR_INFORMATION const *restrict const SLPI,
                                        uint32_t BuffSize); // Blame windhoes

/// inline func defs
static inline void memman_setactivepool(uint32_t Pool)
{
    Base.ActivePool = &Base.Pools[Pool];
}

static inline uint32_t memman_getpool(void)
{
    fprintf(stderr, "Memman craps wants the pool\n");
    Base.Pools[Base.PoolCount].IsPoolInit = 1;
    return Base.PoolCount++;
}

static inline void memman_purgepool(void)
{
    struct memman_pool * restrict CurrentPool = Base.ActivePool;

    VirtualFree(CurrentPool->PoolData, 0, MEM_RELEASE);
}

static inline void memman_purgeall(void)
{
    uint32_t Counter = 0;

    struct memman_pool * restrict PoolArray = Base.ActivePool;

    while (PoolArray[Counter].IsPoolInit == 1)
    {
        VirtualFree(PoolArray[Counter++].PoolData, 0, MEM_RELEASE);
    }
}

static inline void memman_destroy(void)
{
    memman_purgeall();

    VirtualFree(Base.Pools, 0, MEM_RELEASE);
    //Pool's closed, bitchez!
}

static inline uint32_t memman_parseslpi(SYSTEM_LOGICAL_PROCESSOR_INFORMATION const *restrict const SLPI,
                                        uint32_t BuffSize)
{
    uint32_t NumOfProcs = BuffSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

    for (uint32_t Index = 0; Index < NumOfProcs; Index++)
    {
        printf("Line size of proc #[%i] == %i\n", Index, SLPI[Index].Data.Cache.LineSize); ///debug
    }

    return SLPI[0].Data.Cache.LineSize;
}

#endif
