#ifndef KEYTICK_SRC_KT_KEY_LISTNER_H_
#define KEYTICK_SRC_KT_KEY_LISTNER_H_

#include <gtk/gtk.h>
#define KT_TYPE_KEY_LISTENER               (kt_key_listener_get_type ())
#define KT_KEY_LISTENER(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_TYPE_KEY_LISTENER, KtKeyListener))
#define KT_IS_KEY_LISTENER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_TYPE_KEY_LISTENER))
#define KT_KEY_LISTENER_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),  KT_TYPE_KEY_LISTENER, KtKeyListenerClass))
#define KT_IS_KEY_LISTENER_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass), KT_TYPE_KEY_LISTENER))
#define KT_KEY_LISTENER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), KT_TYPE_KEY_LISTENER, KtKeyListenerClass))

typedef struct _KtKeyListener       KtKeyListener;
typedef struct _KtKeyListenerClass  KtKeyListenerClass;

GType           kt_key_listener_get_type            (void) G_GNUC_CONST;

KtKeyListener*  kt_key_listener_new                 (void);
KtKeyListener*  kt_key_listener_new_with_event_id   (guint evt_id);
void            kt_key_listener_listen              (KtKeyListener *instance);
void            kt_key_listener_stop                (void);
void            kt_key_listener_wait                (KtKeyListener *instance);

#endif
