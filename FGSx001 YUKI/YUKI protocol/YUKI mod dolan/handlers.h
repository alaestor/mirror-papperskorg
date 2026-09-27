#ifndef HANDLERS_H_INCLUDED
#define HANDLERS_H_INCLUDED

#include "yuki.h"

__attribute__((fastcall)) void yuki_cmd_hello(struct yuki_peerlist *PeerList);
__attribute__((fastcall)) void yuki_cmd_helloack(struct yuki_peerlist *PeerList);

#endif // HANDLERS_H_INCLUDED
