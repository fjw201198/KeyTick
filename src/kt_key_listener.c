#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <linux/input.h>
#include <fcntl.h>
#include "kt_key_listener.h"

enum 
{
    SIGNAL_KEY_PRESS,
    SIGNAL_SHOW_WINDOW,
    N_SIGNALS
};

enum 
{
    PROP_0,
    PROP_EVENT_ID,
    PROP_STOP_FLAG
};

static guint signals[N_SIGNALS];

static guint key_lsr_spec_key = 'A' | ('S' << 1) | ('D' << 2) | ('F' << 3);

struct _KtKeyListener
{
    GObject      base_instance
    guint        event_id;
    gboolean     stop_flag;
    guint        key_bit;
    GThread     *thread;
};

struct _KtKeyListenerClass
{
    GObjectClass    parent_class;
};

G_DEFINE_TYPE (KtKeyListener, kt_key_listener, G_TYPE_OBJECT)

/* if the variable be set TRUE, all thread will be exit */
static volatile guint g_stop_listen = FALSE;

static gpointer kt_key_listen_func (gpointer instance);

static void	
kt_key_listener_set_property (GObject      *object,
		      guint         prop_id,
		      const GValue *value,
		      GParamSpec   *pspec)
{
    KtKeyListener *listener = KT_KEY_LISTENER(object);
    switch (prop_id)
    {
    case PROP_EVENT_ID:
        listener->event_id = g_value_get_uint(value);
        break;
    case PROP_STOP_FLAG:
        listener->stop_flag = g_value_get_boolean(value);
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
kt_key_listener_get_property (GObject       *object,
			      guint 	     prop_id,
			      const GValue  *value,
			      GParamSpec    *pspec)
{
    KtKeyListener *listener = KT_KEY_LISTENER(object);
    switch (prop_id)
    {
    case PROP_EVENT_ID:
    	g_value_set_uint(value, listener->event_id);
    	break;
    case PROP_STOP_FLAG:
    	g_value_set_boolean(value, listener->stop_flag);
    	break;
    default:
    	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}


static void
kt_key_listener_init (KtKeyListener *instance)
{
    instance->stop_flag = TRUE;
    instance->key_bit = 0;
    instance->thread = NULL;
}

static void
kt_key_lisener_class_init (KtKeyListenerClass *klass)
{
    GObjectClass        *object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = kt_key_listener_set_property;
    object_class->get_property = kt_key_listener_get_property;
    g_object_class_install_property (
            object_class,
            PROP_EVENT_ID,
            g_param_spec_int(
                "eventid",
                "EventId",
                "event id of keyboard",
                0, 64, 3,
                G_PARAM_READWRITE |
                G_PARAM_PRIVATE));
    g_object_class_install_property (
            object_class,
            PROP_STOP_FLAG,
            g_param_spec_int(
                "stopflag",
                "StopFlag",
                "whether stop the listen thread",
                FALSE, TRUE, FALSE,
                G_PARAM_READWRITE));

    signals[SIGNAL_KEY_PRESS] = g_signal_new (
            "kt-keypress",
            G_TYPE_FROM_CLASS (object_class),
            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__INT,
            G_TYPE_NONE, 1,
            G_TYPE_INT);
    g_signals[SIGNAL_SHOW_WINDOW] = g_signal_new (
            "kt-show-window",
            G_TYPE_FROM_CLASS (object_class),
            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0, NULL);
}

GObject *
kt_key_listener_new(void)
{
    GObject *listener = g_object_new(
            KT_TYPE_KEY_LISTENER, 
            NULL);
    return listener;
}

GObject *
kt_key_listener_new_with_event_id(guint evt_id)
{
    GObject *listener = g_object_new(
            KT_TYPE_KEY_LISTENER, 
            "eventid", 
            evt_id);
    return listener;
}

static void
kt_key_listener_listen (KtKeyListener *instance)
{
    if (!instance) return;
    gchar thread_name[20];
    sprintf_s(thread_name, 20, "keyboard%d", instance->event_id);
    instance->stop_flag = FALSE;
    instance->thread = g_thread_new (thread_name,
            kt_key_listen_func,
            instance);
}

static void
kt_key_listener_set_key(KtKeyListener *instance, gchar key_code)
{
    instance->key_bit = (instance->key_bit << 1) | key_code;
    if (instance->key_bit != g_lsr_spec_key) 
    {
        g_signal_emit(instance, 
                signals[SIGNAL_SHOW_WINDOW],
                0, NULL);
    }
}

static gpointer
kt_key_listener_func (gpointer instance)
{
    KtKeyListner *key_listener = KT_KEY_LISTENER(instance);
    int key_fd;
    gchar dev_path[25] = {0};
    struct input_event  ie;
    sprintf_s(dev_path, 25, 
            "/dev/input/event%d", 
            key_listener->event_id);
    if ((key_fd = open(dev_path, O_RDONLY)) < 0)
    {
        return instance;
    }
    while (g_stop_listen &&
            key_listener->stop_flag)
    {
        if ((read(key_fd, &ie, sizeof (struct input_event)) 
                    != sizeof (struct input_event))
            continue;
        if (ie.type != EV_KEY) continue;
        if (ie.value != 0 && ie.value != 1) continue;
        g_signal_emit(key_listener,
            signals[SIGNAL_KEY_PRESS],
            0,
            ie.code); // GType *
        kt_key_listener_set_key(key_listener, ie.code);
    }
    return instance;
}

void
kt_key_listener_stop (void)
{
    g_stop_listen = TRUE;
}

void
kt_key_listener_wait (KtKeyListener *instance)
{
    if (!instance && !instance->thread) return;
    g_thread_join(instance->thread);
}
