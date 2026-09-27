#ifndef YUKITYPES_H_INCLUDED
#define YUKITYPES_H_INCLUDED

/// Type defs

// State Machine -- "the stack of crazy"
typedef uint32_t yuki_state_t;
typedef yuki_state_t yuki_stfuncret_t;
typedef struct yuki_basenode *yuki_stfuncarg0_t;
typedef __attribute__((fastcall)) yuki_stfuncret_t yuki_stfunc_t(yuki_stfuncarg0_t CurrentNode);

typedef uint32_t yuki_endpoint_t;
typedef uint16_t yuki_magic_t;

// WINDHOES CRANK INCLUDES

typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP
{
    RelationProcessorCore,
    RelationNumaNode,
    RelationCache,
    RelationProcessorPackage,
    RelationGroup,
    RelationAll = 0xffff
} LOGICAL_PROCESSOR_RELATIONSHIP;

#define LTP_PC_SMT 0x1

typedef enum _PROCESSOR_CACHE_TYPE
{
    CacheUnified,
    CacheInstruction,
    CacheData,
    CacheTrace
} PROCESSOR_CACHE_TYPE;

#define CACHE_FULLY_ASSOCIATIVE 0xFF

typedef struct _CACHE_DESCRIPTOR
{
    BYTE Level;
    BYTE Associativity;
    WORD LineSize;
    DWORD Size;
    PROCESSOR_CACHE_TYPE Type;
} CACHE_DESCRIPTOR, *PCACHE_DESCRIPTOR;

typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION
{
    ULONG_PTR ProcessorMask;
    LOGICAL_PROCESSOR_RELATIONSHIP Relationship;

    union _Data
    {
        struct
        {
            BYTE Flags;
        } ProcessorCore;

        struct
        {
            DWORD NodeNumber;
        } NumaNode;

        CACHE_DESCRIPTOR Cache;
        ULONGLONG Reserved[2];

    } Data;

} SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

WINBASEAPI BOOL WINAPI GetLogicalProcessorInformation(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
                                                      PDWORD ReturnedLength);

#endif // YUKITYPES_H_INCLUDED
