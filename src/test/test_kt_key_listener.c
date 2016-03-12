#include "../kt_key_listener.h"
#include "../kt_keyevt.h"

gpointer key_press_cb(KtKeyListener* lsn, guint key, gpointer data)
{
    printf ("key %d pressed\n", key);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    struct KtKeyList kl;
    kt_get_keyboards(&kl);
    /* struct KtKeyListNode *node; */
    /* for (node = kl.lh_first; node != NULL; node = node->entries.le_next) */
    /* { */
    /*     printf("%d\n", node->evtno); */
    /* } */
    if (kl.lh_first == NULL)
        return -1;
    KtKeyListener *listener = kt_key_listener_new_with_event_id (kl.lh_first->evtno);
    g_signal_connect(listener, "kt-keypress", 
            G_CALLBACK(key_press_cb), NULL);
    kt_key_listener_listen (listener);
    kt_key_listener_wait(listener);
    return 0;
}

