

/**
*       Come here to shitbricks.
*       Work in progress
*       Currently is primary function for ExitProcess()
**/

#include <stdio.h>
#include <windows.h>

#include "eHandle.h"

/* NOTES

All functions used in the manager assume that the functions will be used safely.

No arg checking for maximum efficiency (and breakage)

///////////NOT THREAD SAFE////////////// :) (smiley face -FGM005)

*/

// Resource File-Globals

static void *ResourceListBase;

// Resource Functions

void RegListInit(void) //done, maybe
{
    printf(" Yuki.N:// Resource Manager Initializing!\n"); // DEBUG

    struct _ResourceManager *ResourceManager;

    ResourceManager = (struct _ResourceManager *)malloc(sizeof(struct _ResourceManager));

    if (ResourceManager == NULL)
    {
        printf(" Yuki.N:// Could not allocate resource list!\n");
    }

    ResourceListBase = (void *)ResourceManager;
    ResourceManager->ResourceCount = 0;

    // Fill primary list buffer as 'last' deallocation to take place  <-- This is the stupid

    return;
}


void RegPermResc(void *BufferLocation, uint_fast32_t Method)
{
    struct _ResourceManager *ResourceManager = (struct _ResourceManager *)ResourceListBase;

    printf(" Yuki.N:// Permanent Resource Registered.\n"); // DEBUG #1gig......loll                                             ies

    struct _ResourceNode *ResourceTemp = &(ResourceManager->ResourceNodes[ResourceManager->ResourceCount]);

    if (ResourceManager->ResourceCount < EH_MAXPERM)
    {
        ResourceTemp->Buffer = BufferLocation; // holy pointer
        ResourceTemp->DeallocType = Method;
        ResourceManager->ResourceCount++;
    }
    else
    {
        printf(" Yuki.N:// Too many permanent resources allocated!\n");
    }

    return;
}

void DestroyRescManager(void)
{
    struct _ResourceManager *ResourceManager = (struct _ResourceManager *)ResourceListBase;

    printf(" Yuki.N:// Destroying Resource Manager!\n");

    register struct _ResourceNode *ResourceTemp;

    while (ResourceManager->ResourceCount > 0)
    {
        ResourceTemp = &(ResourceManager->ResourceNodes[ResourceManager->ResourceCount]);

        switch (ResourceTemp->DeallocType)
        {
            case EH_TYPE_CSTD:
                free(ResourceTemp->Buffer);
                break;

            case EH_TYPE_WINV:
                VirtualFree(ResourceTemp->Buffer, 0, MEM_RELEASE);
                break;
        }

        ResourceManager->ResourceCount--;
    }

    free(ResourceListBase);

    return;
}


