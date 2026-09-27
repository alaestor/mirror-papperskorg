#ifndef E_HANDLE_H_INCLUDED
#define E_HANDLE_H_INCLUDED

// Defines for resource registration

#define EH_MAXPERM 32

#define EH_BASE_ALLOC 0

#define EH_TYPE_CSTD 0
#define EH_TYPE_WINV 1
#define EH_TYPE_WINH 2
#define EH_TYPE_CUSTOM 3 //Reserved for dynamic function call handling e.g. real exceptions

#include <windows.h>
#include <stdint.h>

void RegPermResc(void *BufferLocation, uint_fast32_t Method);
void RegListInit(void);
void DestroyRescManager(void);

// STRUCTURES FOR RPR

struct _ResourceNode
{
    void *Buffer;
    uint_fast32_t DeallocType;
    //struct _ResourceNode *NextChain;  Only needed if you change to linked list
};

struct _ResourceManager
{
    uint_fast32_t ResourceCount;
    struct _ResourceNode ResourceNodes[EH_MAXPERM];
};

#endif
