#ifndef KEYTICK_SRC_KT_SETTINGS_UI_H
#define KEYTICK_SRC_KT_SETTINGS_UI_H
#include <gtk/gtk.h>

#define KT_TYPE_SETTINGS_UI             (kt_settings_ui_get_type ())
#define KT_SETTINGS_UI(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), KT_TYPE_SETTINGS_UI, KtSettingsUi))
#define KT_IS_SETTINGS_UI(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), KT_TYPE_SETTINGS_UI))
#define KT_SETTINGS_UI_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), KT_TYPE_SETTINGS_UI, KtSettingsUiClass))
#define KT_IS_SETTINGS_UI_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), KT_TYPE_SETTINGS_UI))
#define KT_SETTINGS_UI_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), KT_TYPE_SETTINGS_UI, KtSettingsUiClass))

typedef struct _KtSettingsUi        KtSettingsUi;
typedef struct _KtSettingsUiClass   KtSettingsUiClass;

GType           kt_settings_ui_get_type         (void)  G_GNUC_CONST;

GtkWidget*      kt_settings_ui_new              (void);

void            kt_settings_ui_set_active       (KtSettingsUi *su, guint index);
guint           kt_settings_ui_get_active       (KtSettingsUi *su);

void            kt_settings_ui_show             (KtSettingsUi *su);
void            kt_settings_ui_hide             (KtSettingsUi *su);

#endif
