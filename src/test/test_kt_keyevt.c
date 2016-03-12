#include <stdio.h>
#include "../kt_keyevt.h"

int main() {
    struct KtKeyList kl;
    kt_get_keyboards(&kl);
    struct KtKeyListNode *node;
    for (node = kl.lh_first; node != NULL; node = node->entries.le_next)
    {
        printf("%d\n", node->evtno);
    }
    return 0;
}
