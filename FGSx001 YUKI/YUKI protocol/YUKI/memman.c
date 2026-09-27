#include <stdint.h>
#include <windows.h>
#include <stdio.h>
#include "memman.h"
#include "fglmaths.h"


/// For indepth explaination of the MemMan source; http://dft.ba/-memman-source

struct memman_base Base;

uint32_t memman_init(uint32_t PoolCountArg)
{
    fprintf(stderr, "Ace Dolan Craps Wants the Size:");
    Base.PoolCount = PoolCountArg;

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

    uint32_t BitIndex;
    uint32_t BuffSize = 0;

    // GetTheLogicBehindWhyTheHellMicrosoftLovesLong(Function_Names);
    //if (GetLogicalProcessorInformation(NULL, (DWORD *)&BuffSize) != ERROR_INSUFFICIENT_BUFFER)
        //return 0;

    //uint32_t const SLPIBufferSize = fmaths_roundup_power2_next(BuffSize);
    //uint8_t SLPIBuffer[SLPIBufferSize];

    // GetTheLogicBehindWhyTheHellMicrosoftLovesLong(Structure_Names);
    //SYSTEM_LOGICAL_PROCESSOR_INFORMATION const *restrict const SLPI = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION const *restrict const)SLPIBuffer; // oh god windhoes! WHY?!

    uint32_t PageSize = SysInfo.dwPageSize;
    uint32_t MemSize = sizeof(*Base.Pools) * PoolCountArg;
    uint32_t AllocSize = PageSize + PageSize * (MemSize / PageSize);

    fprintf(stderr, "Ace Dolan Craps Wants the Size: %u\n", AllocSize);

    Base.PageSize = PageSize;

    Base.Pools = VirtualAlloc(NULL, AllocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    Base.LineSize = 32;

    if (Base.Pools == NULL)
    {
        for (uint32_t PoolIndex = 0; PoolIndex < PoolCountArg; PoolIndex++)
            Base.Pools[PoolIndex].Allocations = 0;

        return 1;
    }

    return 0;
}


uint32_t memman_reservepool(uint32_t ReserveAmount)
{
    struct memman_pool *CurrentPool = Base.ActivePool;

    CurrentPool->PoolData = VirtualAlloc(NULL, ReserveAmount, MEM_RESERVE, PAGE_READWRITE);

    if (CurrentPool->PoolData == NULL)
        return 0;

    CurrentPool->ReserveRange = ReserveAmount;
    CurrentPool->IsPoolInit = 1;
    CurrentPool->AllocRange = 0;
    CurrentPool->CurrentAllocPos = CurrentPool->PoolData;

    return 1;
}


void *memman_allocpool(uint32_t AllocSize)
{
    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE -2\n");

    struct memman_pool *restrict CurrentPool = Base.ActivePool;
    struct memman_allocation *restrict const CurrentAlloc = CurrentPool->CurrentAllocPos;

    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE -1\n");

    uint32_t const RoundedSize = fmaths_roundup_power2_ceiling(sizeof(struct memman_allocation) + AllocSize, Base.LineSize);

    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE -1.1\n");

    uint32_t const TotalBlocks = RoundedSize / Base.LineSize;
    uint8_t *EndingAddress = ((uint8_t *)CurrentAlloc) + RoundedSize;

    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE 0\n");

    if (EndingAddress > ((uint8_t *)CurrentAlloc + CurrentPool->AllocRange)) // is gooooood
    {
        if (VirtualAlloc(CurrentAlloc, RoundedSize, MEM_COMMIT, PAGE_READWRITE) != NULL)
        {
            CurrentPool->AllocRange += EndingAddress - ((uint8_t *)CurrentAlloc);

            fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE 1\n");

            if (CurrentPool->AllocRange &= (Base.PageSize - 1))
                CurrentPool->AllocRange += Base.PageSize;

            fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE 2\n");
        }
        else
        {
            return NULL;
        }
    }

    void *ReturnAddr = ((uint8_t *)CurrentAlloc) + sizeof(struct memman_allocation);

    CurrentPool->CurrentAllocPos = EndingAddress;

    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE 3\n");

    CurrentAlloc->Pool = CurrentPool;
    CurrentAlloc->Blocks = TotalBlocks;

    fprintf(stderr, "MEMMAN_ALLOCPOOL STAGE 4\n");

    return ReturnAddr;
}





/*
memman_compresspool()
{
}

memman_rectifypool()
{
}

*/
