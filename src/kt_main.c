//#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include "kt_keyevt.h"
#include "kt_key_listener.h"
#include "kt_player.h"
#include "kt_settings_ui.h"

typedef struct _KtKeyBrds
{
    struct KtKeyList *kl;
    KtKeyListener   **klsn;
    guint             key_num;
} KtKeyBrds;

typedef struct _KtMain
{
    KtKeyBrds    *kbds;
    KtPlayer     *player;
    GtkWidget    *ui;
} KtMain;

KtMain *main_data;

static gpointer
kt_key_press_cb (KtKeyListener* lsn, guint key, gpointer data)
{
    /* kt_player_play (main_data->player); */
    g_signal_emit_by_name (main_data->player, "kt-player-play", NULL);
    return lsn;
}

static gpointer
kt_show_settings_cb (KtKeyListener* lsn, gpointer data)
{
    KtMain *ktmain = (KtMain *)data;
    guint index = kt_player_get_sound_type (ktmain->player);
    KtSettingsUi *ui = KT_SETTINGS_UI (ktmain->ui);
    kt_settings_ui_set_active (ui, index);
    g_signal_emit_by_name (ui, "ui-show", NULL);
    return lsn;
}

static KtKeyBrds *
kt_key_brds_new (void)
{
    guint index = 0;
    KtKeyListener **pNode;
    KtKeyBrds  *kbds = g_malloc (sizeof(KtKeyBrds));
    kbds->kl = g_malloc (sizeof (struct KtKeyList));
    kt_get_keyboards (kbds->kl);
    kbds->key_num = 0;
    struct KtKeyListNode *node;
    for (node = kbds->kl->lh_first; node != NULL; node = node->entries.le_next)
    {
        ++ kbds->key_num;
    }
    kbds->klsn = g_malloc (kbds->key_num * sizeof (KtKeyListener*));
    pNode = kbds->klsn;
    for (index = 0, node = kbds->kl->lh_first; 
         index != kbds->key_num; 
         ++ index, ++ pNode, node = node->entries.le_next)
    {
           *pNode = kt_key_listener_new_with_event_id(node->evtno);
    }
    return kbds;
}

static void 
kt_key_brds_listen (KtMain* ktmain)
{
    guint index;
    for (index = 0; index != ktmain->kbds->key_num; ++ index)
    {
       KtKeyListener *listener = *(ktmain->kbds->klsn + index);
       g_signal_connect (listener, "kt-keypress",
                         G_CALLBACK (kt_key_press_cb), NULL);
       g_signal_connect (listener, "kt-show-window",
                         G_CALLBACK (kt_show_settings_cb), ktmain); 
        kt_key_listener_listen (listener);
    }
}

static gpointer 
kt_update_etc_cb (KtSettingsUi *ui, gpointer data)
{
    guint sound_id;
    sound_id = kt_settings_ui_get_active (ui);
    kt_player_set_sound_type (main_data->player, sound_id);
}

static gpointer
kt_ui_exit_cb (KtSettingsUi *ui, gpointer data)
{
    kt_player_save_config (main_data->player);
    gtk_main_quit();
}

static KtMain*
kt_main_new (void)
{
    KtMain *km;
    KtSettingsUi *gui;
    guint snd_type = 0;
    km = g_malloc (sizeof (KtMain));
    km->kbds = kt_key_brds_new ();
    km->player = kt_player_new ();
    g_signal_connect (km->player, "kt-player-exit", 
                      G_CALLBACK (kt_player_uninit), NULL);
    km->ui = kt_settings_ui_new ();
    snd_type = kt_player_get_sound_type (km->player);
    gui = KT_SETTINGS_UI (km->ui);
    kt_settings_ui_set_active (gui, snd_type);
    g_signal_connect (gui, "update-etc", 
                      G_CALLBACK (kt_update_etc_cb), NULL);
    g_signal_connect (gui, "ui-exit",
                      G_CALLBACK (kt_ui_exit_cb), NULL);
    return km;
}

int 
main(int argc, char *argv[])
{
    XInitThreads();
    gtk_init (&argc, &argv);
    kt_player_alut_init (&argc, argv);
    main_data = kt_main_new (); 
    kt_key_brds_listen(main_data);
    /* kt_settings_ui_show (KT_SETTINGS_UI (main_data->ui));   */
    gtk_main();
}
