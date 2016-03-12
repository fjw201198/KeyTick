/* *
 * File: kt_keyevt.h
 * Desc: This file get the keyboard devices, and then listen process the keydown event.
 * */
#ifndef KEYTICK_SRC_KT_KEYEVT_H_
#define KEYTICK_SRC_KT_KEYEVT_H_
#include "sys/queue.h"

struct KtKeyListNode 
{
    unsigned int                evtno;
    LIST_ENTRY(KtKeyListNode)   entries;
};

LIST_HEAD(KtKeyList, KtKeyListNode);

int 
kt_get_keyboards(struct KtKeyList *keybrds);

//struct KtKeyList *

#endif
