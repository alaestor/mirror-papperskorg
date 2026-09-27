#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include "yuki.h"
#include "yukistate.h"

uint32_t yuki_setaddressIPv4(struct yuki_basenode *Desc,
                             int8_t const *const Addr,
                             int8_t const *const Port)
{
    struct addrinfo HintInfo;
    struct addrinfo *AddrInfo;

    memset(&HintInfo, 0, sizeof(struct addrinfo));

    HintInfo.ai_family = AF_INET;

    if ((getaddrinfo((const char *)Addr, (const char *)Port, &HintInfo, &AddrInfo) == 0) && (AddrInfo->ai_family == AF_INET))
    {
        memcpy(&Desc->Addresses[0], AddrInfo->ai_addr, sizeof(struct sockaddr));
        return 1;
    }

    return 0;
}

void yuki_execstate(void)
{
    struct yuki_peerlist *restrict CurrentList = YukiEngine.CurrentList;
    struct yuki_basenode *restrict CurrentNode = CurrentList->BaseNodeArray;

    unsigned long int Option;
    yuki_endpoint_t EndPoint = yuki_getendpoint();
    int32_t FromLen = sizeof(struct sockaddr_in);
    uint32_t BytesRead = 0;

    // Check for incoming data

    ioctlsocket(EndPoint, FIONREAD, &Option);

    // If there is, get it and store in the list-wide buffer.

    if (Option > 0)
    {
        CurrentList->YSTTFlags |= YUKI_STATE_PACKET_INCOMING;

        BytesRead = recvfrom(EndPoint,
                             (char *)CurrentList->ListPacketBuffer,
                             YCA_MAXPACKBUFFSIZE,
                             0,
                             (struct sockaddr *)&CurrentList->ListAddressBuffer,
                             &FromLen);

    }

    // For each basenode, extract state and execute based on the offset from YSTTable.

    for (uint32_t Node = 0; Node < CurrentList->CurrentPeers; Node++)
    {
        yuki_state_t CurrentState = CurrentNode[Node].State;
        yuki_stfunc_t *CurrentFunction = CurrentList->YSTTable[CurrentState];


        // If the state var is within bounds, AND it has a valid function pointer, call it.
        // Else, state is a completion state that has no associated function; Skip it.

        if ((CurrentState < CurrentList->YSTTEntries) && (CurrentFunction != NULL))
        {
            // Does the node accept data?
            //fprintf(stderr, "Node within bounds\n");
            if (CurrentNode[Node].StateFlags & YUKI_NODE_ACCEPT_DATA)
            {
                // Do we NOT have data? Then GOTO the skip condition.
                // Call me dirty, but fuck the redundant else.
                //fprintf(stderr, "Node Accepts Data\n");
                if ((CurrentList->YSTTFlags & YUKI_STATE_PACKET_INCOMING) == 0)
                {
                    //fprintf(stderr, "Machine has no data\n");
                    goto YUKI_EXECSTATE_SKIPCONDITION; /****WARNING GOTO JUMP 1****/
                }

                //fprintf(stderr, "Machine has data\n");

                CurrentNode[Node].BufferUsed = BytesRead;
            }
            //else
                //fprintf(stderr, "Node does not accept data\n");

            //fprintf(stderr, "Executing state function for node %i, state %i\n", Node, CurrentState);
            CurrentState = CurrentFunction(&CurrentNode[Node]);
            //fprintf(stderr, "New state for node %i: %i\n", Node, CurrentState);

            CurrentNode[Node].State = CurrentState;
        }
        else if (CurrentFunction != NULL)
            fprintf(stderr, "Node %u has invalid state\n", Node);

        YUKI_EXECSTATE_SKIPCONDITION: /****WARNING GOTO TARGET 1****/
            ; // Null statement must exist after compound blocks for a label to be valid C
    }

    if ((CurrentList->YSTTFlags & YUKI_STATE_USED_BUFFER) == 0)
    {
        if (CurrentList->YSTTDefault != NULL)
            CurrentList->YSTTDefault();
    }

    // reset any global condition flags

    CurrentList->YSTTFlags = 0;
}









