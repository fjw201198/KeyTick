/* *
 * File: kt_keyevt.c
 * */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "kt_keyevt.h"

#define KT_MAX_BUF_SZ 40960

enum {
    KT_SUCCESS          =  0,
    KT_ERR_NO_KEYBRD    =  1,
    KT_ERR_OPEN_DEV     =  2,
    KT_ERR_THREAD       =  3,
    KT_ERR_SOUND        =  4,
    KT_ERR_MEM          =  5,
    KT_ERR_UNKNOWN      = -1,
};

int
kt_get_keyboards(struct KtKeyList *keybrds)
{
    /* init list */
    if (keybrds == 0) 
    {
        keybrds = malloc(sizeof(struct KtKeyList));
    }
    LIST_INIT(keybrds);
    /* read the device lists */
    char *proc_dev = "/proc/bus/input/devices";
    int fd = open(proc_dev, O_RDONLY);
    if (fd < 0) {
        char *err_msg = "[FAIL] open /proc/bus/input/devices failed\n";
        write(2, err_msg, strlen(err_msg));
        return KT_ERR_OPEN_DEV;
    }
    char dev_buf[KT_MAX_BUF_SZ];
    if (read(fd, dev_buf, KT_MAX_BUF_SZ) < 0) 
    {
        char *err_msg = "[FAIL] read devices info failed\n";
        write(2, err_msg, strlen(err_msg));
        return KT_ERR_OPEN_DEV;
    }
    char *lc = dev_buf;
    while (lc != 0)
    {
        lc = strstr(lc, "keyboard");
        if (lc == 0) 
        {
            break;
        }
        lc = strstr(lc + 1, "event");
        if (lc == 0) 
        {
            char *err_msg = "[FAIL] no keyboard device found\n";
            write(2, err_msg, strlen(err_msg));
            return KT_ERR_NO_KEYBRD;
        }
        struct KtKeyListNode *tmpNode = malloc(sizeof(struct KtKeyListNode));
        if (tmpNode == 0) {
            char *err_msg = "[FAIL] alloc memory failed\n";
            write(2, err_msg, strlen(err_msg));
            return KT_ERR_MEM;
        }
        if (sscanf(lc, "event%d", &tmpNode->evtno) < 0)
        {
            char *err_msg = "[FAIL] read evtno failed\n";
            write(2, err_msg, strlen(err_msg));
            return KT_ERR_NO_KEYBRD;
        }
        LIST_INSERT_HEAD(keybrds, tmpNode, entries);
    }
    return KT_SUCCESS;         
}
