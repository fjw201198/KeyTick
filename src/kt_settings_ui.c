#include "kt_settings_ui.h"

enum 
{
    SIGNAL_UPDATE,
    SIGNAL_EXIT,
    SIGNAL_SHOW,
    N_SIGNALS
};

enum
{
    INT_ITEM,
    STR_ITEM,
    N_ITEMS
};

static guint signals[N_SIGNALS] = {0};

struct _KtSettingsUi
{
    GtkWidget    base_instance;
    GtkWidget   *window;
    GtkWidget   *tips;
    GtkWidget   *label;
    GtkWidget   *box;
    GtkWidget   *hbox;
    GtkWidget   *combox;
    GtkCellRenderer *renderer;
    GtkListStore *list_store;
    GtkWidget   *close_btn;
    GtkWidget   *exit_btn;
};


struct _KtSettingsUiClass
{
    GtkWidgetClass   parent_class;
    void           (*show_ui) (KtSettingsUi* ksu, gpointer data);
};

G_DEFINE_TYPE(KtSettingsUi, kt_settings_ui, GTK_TYPE_WIDGET);

static void 
kt_settings_ui_finalize (GObject *ksu)
{
    /* KtSettingsUi *ui = KT_SETTINGS_UI (ksu); */
    /* gtk_widget_destroy (ui->window); */
    /* gtk_widget_destroy (ui->tips); */
    /* gtk_widget_destroy (ui->label); */
    /* gtk_widget_destroy (ui->combox); */
    /* gtk_list_store_clear (ui->list_store); */
    /* gtk_widget_destroy (ui->box); */
    /* gtk_widget_destroy (ui->hbox); */
    /* gtk_widget_destroy (ui->close_btn); */
    /* gtk_widget_destroy (ui->exit_btn); */
    G_OBJECT_CLASS (kt_settings_ui_parent_class)->finalize (ksu);
}

static gpointer
kt_settings_ui_close (GtkButton *btn, gpointer data)
{
    GtkWidget *window = GTK_WIDGET (data);
    gtk_widget_hide (window);
    return btn;
}

static gpointer
kt_settings_ui_exit (GtkWidget *exit_btn, gpointer data)
{
    KtSettingsUi *ksu = KT_SETTINGS_UI (data);
    g_signal_emit_by_name (ksu, "ui-exit", NULL);
    return ksu;
}

static void kt_settings_ui_show_cb (KtSettingsUi *ksu, gpointer data);

static gpointer
kt_settings_ui_change (GtkWidget *combox, gpointer data)
{
    KtSettingsUi *ksu = KT_SETTINGS_UI (data);
    g_signal_emit (ksu, signals[SIGNAL_UPDATE], 0, NULL);
    return combox;
}

static void
kt_settings_ui_init(KtSettingsUi *su)
{
    GtkWindow   *window;
    GtkTreeIter  iter;
    GtkTreeModel *model;
    GtkCellLayout *layout;
    GtkBox      *box;
    GtkBox      *hbox;

    su->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (su->window, 200, 200);
    window = GTK_WINDOW (su->window);
    gtk_window_set_resizable (window, FALSE);
    gtk_window_set_title (window, "KeyTick");
    su->tips = gtk_label_new ("窗口关闭后，按‘ASDF’可以显示设置窗口哦");
    su->label = gtk_label_new ("请选择声音效果");
    su->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 30);
    su->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 30);
    su->list_store = gtk_list_store_new (N_ITEMS, G_TYPE_INT, G_TYPE_STRING);
    gtk_list_store_append (su->list_store, &iter);
    gtk_list_store_set (su->list_store, &iter, 
                        INT_ITEM, 0,
                        STR_ITEM, "打字机效果",
                        -1);
    gtk_list_store_append (su->list_store, &iter);
    gtk_list_store_set (su->list_store, &iter, 
                        INT_ITEM, 1,
                        STR_ITEM, "滴答声效果",
                        -1);
    gtk_list_store_append (su->list_store, &iter);
    gtk_list_store_set (su->list_store, &iter, 
                        INT_ITEM, 2,
                        STR_ITEM, "水滴声效果",
                        -1);
    model = GTK_TREE_MODEL (su->list_store);
    su->combox = gtk_combo_box_new_with_model (model);
    su->renderer = gtk_cell_renderer_text_new ();
    layout = GTK_CELL_LAYOUT (su->combox);
    gtk_cell_layout_pack_start (layout, su->renderer, FALSE);
    gtk_cell_layout_set_attributes (layout, su->renderer, "text", STR_ITEM, NULL);
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (su->combox), &iter);

    su->close_btn = gtk_button_new_with_label ("关闭");
    su->exit_btn = gtk_button_new_with_label ("退出");
    
    box = GTK_BOX (su->box);
    gtk_box_pack_start(box, su->tips, TRUE, TRUE, 0);   
    gtk_box_pack_start(box, su->label, TRUE, TRUE, 0);   
    gtk_box_pack_start(box, su->combox, TRUE, TRUE, 0);

    hbox = GTK_BOX (su->hbox);
    gtk_box_pack_end(hbox, su->exit_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(hbox, su->close_btn, FALSE, FALSE, 0);
    gtk_box_pack_start (box, su->hbox, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (su->window), su->box);
    g_signal_connect (su->close_btn, "clicked", 
                      G_CALLBACK (kt_settings_ui_close), su->window);
    g_signal_connect (su->exit_btn, "clicked",
                      G_CALLBACK (kt_settings_ui_exit), su);
    g_signal_connect (su->window, "destroy", 
                      G_CALLBACK (kt_settings_ui_exit), su);
    g_signal_connect (su->combox, "changed",
                      G_CALLBACK (kt_settings_ui_change), su);
    gtk_widget_show_all (su->window);
}

GtkWidget *
kt_settings_ui_new ()
{
    GtkWidget *set_gui;
    set_gui = g_object_new (KT_TYPE_SETTINGS_UI, NULL);
    return set_gui;
}

void
kt_settings_ui_class_init (KtSettingsUiClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    /* GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass); */

    object_class->finalize = kt_settings_ui_finalize;
    klass->show_ui = kt_settings_ui_show_cb;
    
    signals[SIGNAL_UPDATE] = g_signal_new ("update-etc",
                                           G_TYPE_FROM_CLASS (object_class),
                                           G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                           0,  NULL, NULL,
                                           g_cclosure_marshal_VOID__VOID,
                                           G_TYPE_NONE,
                                           0, G_TYPE_NONE);
    signals[SIGNAL_EXIT] = g_signal_new ("ui-exit",
                                         G_TYPE_FROM_CLASS (object_class),
                                         G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                         0, NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE,
                                         0, NULL);
    signals[SIGNAL_SHOW] = g_signal_new ("ui-show",
                                         G_TYPE_FROM_CLASS (object_class),
                                         G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                         G_STRUCT_OFFSET (KtSettingsUiClass, show_ui),
                                         NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE,
                                         0, G_TYPE_NONE);
}

guint
kt_settings_ui_get_active (KtSettingsUi *ksu)
{
    guint iret = 0;
    do
    {
        GtkTreeIter iter;
        gboolean flag;
        flag = gtk_combo_box_get_active_iter (GTK_COMBO_BOX(ksu->combox), &iter);
        if (!flag ) break;
        gtk_tree_model_get (GTK_TREE_MODEL (ksu->list_store), &iter,
                            INT_ITEM, &iret, -1);
    } while (0);
    return iret;
}

void
kt_settings_ui_set_active (KtSettingsUi *ksu, guint index)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint ind;
    gboolean bret;
    model = GTK_TREE_MODEL (ksu->list_store);
    bret = gtk_tree_model_get_iter_first (model, &iter);   
    while (bret)
    {
        gtk_tree_model_get (model, &iter,
                            INT_ITEM, &ind, -1);

        if (ind == index) break;
        bret = gtk_tree_model_iter_next (model, &iter);
    }

    if (ind == index)
    {
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (ksu->combox), &iter);
    }
}

void 
kt_settings_ui_show (KtSettingsUi *ksu)
{
    gtk_widget_show_all (ksu->window);
}

void kt_settings_ui_show_cb (KtSettingsUi *ksu, gpointer data)
{
    kt_settings_ui_show (ksu);
}

void 
kt_settings_ui_hide (KtSettingsUi *ksu)
{
    gtk_widget_hide (ksu->window);
}
