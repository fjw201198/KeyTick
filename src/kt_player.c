#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "kt_player.h"

static const gchar *tock_file  = "tock.wav";
static const gchar *tick_file  = "tick.wav";
static const gchar *water_file = "water.wav";
static const gchar *config     = "keytick.conf";

enum
{
    SIGNAL_LOAD,
    SIGNAL_EXIT,
    N_SIGNALS
};

enum
{
    PROP_0,
    PROP_VOLUMN,
    PROP_SND_STYLE
};

static guint signals[N_SIGNALS];

struct _KtPlayer
{
    GObject      base_instance;
    ALuint      *cur_src;
    ALuint      *srcs;
    ALuint      *buffers;
    ALCdevice   *device;
    ALCcontext  *context;
    ALuint       vol;
    gboolean     chg_etc;
    GSettings   *settings;
};

struct _KtPlayerClass
{
    GObjectClass    parent_class;
    gpointer        (*kt_exit) (KtPlayer* player, gpointer data);
    gpointer        (*kt_load) (KtPlayer* player, gpointer data);
};

G_DEFINE_TYPE (KtPlayer, kt_player, G_TYPE_OBJECT)

static gpointer kt_player_load (KtPlayer *player, gpointer data);

static gchar *
kt_player_get_file_path (const gchar *fname)
{
    gchar *filepath = NULL;
    do
    {
        /* get current path */
        gchar  path[50];
        gchar *pos = NULL;
        size_t path_size;
        size_t fname_len = 0;
        int    flag = 1;
        bzero(path, 50);
        sprintf (path, "/proc/%d/exe", getpid());
        filepath = g_malloc(PATH_MAX * sizeof gchar);
        path_size = readlink(path, filepath, PATH_MAX);
        if (path_size < 0) 
        {
            g_free (filepath);
        }
        *(filepath+path_size) = '\0';
        pos = strrchr(filepath, '/');
        pos = pos + 1;
        fname_len = strlen(fname) + 1;
        /* find file in current directory */
        memmove (pos, fname, fname_len);
        if (access (filepath, F_OK) == 0) break;
        /* find file in ../sound, attention sizeof operator, it will bigger then strlen */
        memmove (pos, "../sound/", sizeof "../sound");
        pos = pos + sizeof "../sound/";
        memmove (pos, fname, fname_len);
        if (access (filepath, F_OK) == 0) break;
        /* find file in /usr/share/keytick/sound */
        memmove (filepath, "/usr/share/keytick/", sizeof "/usr/share/keytick");
        pos = filepath + sizeof "/usr/share/keytick";
        memmove (pos, fname, fname_len);
        if (access (filepath, F_OK) == 0) break;
        g_free (filepath);
        filepath = NULL;
    } while (0);
    return filepath;
}

static gpointer
kt_player_uninit(KtPlayer *player, gpointer data)
{
    /* release memory, and emite exit signal */
    alcMakeContextCurrent (NULL);
    alcDestroyContext (player->context);
    alcCloseDevice (player->device);
    alutExit ();
    g_free(player->srcs);
    g_free(player->buffers);
    player->cur_src = NULL;
    g_signal_emit (player, signals[SIGNAL_EXIT], 0, NULL);
    return player;
} 

static void
kt_player_init (KtPlayer *player)
{
    do
    {
        gchar  *filepath = NULL;
        ALuint *psrc     = NULL;
        player->chg_etc = FALSE;
        player->settings = g_settings_new_with_path ("player_conf", config);
        player->srcs    = g_malloc (KT_NSND * sizeof ALuint);
        player->buffers = g_malloc (KT_NSND * sizeof ALuint);
        player->cur_src = player->srcs;
        kt_player_load(player, NULL);
        player->device  = alcOpenDevice (NULL);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        player->context = alcCreateContext (player->device, NULL);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        alcMakeContextCurrent (player->context);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        filepath = kt_player_get_file_path (tock_file);
        if (! filepath) 
        {
            kt_player_uninit (player, NULL);
            break;
        }
        *player->buffers = alutCreateBufferFromFile (filepath);
        g_free(filepath);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        filepath = kt_player_get_file_path (tick_file);
        if (! filepath)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        *(player->buffers + 1) = alutCreateBufferFromFile (filepath);
        g_free (filepath);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        filepath = kt_player_get_file_path (water_file);
        if (! filepath)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        *(player->buffers + 2) = alutCreateBufferFromFile (filepath);
        g_free (filepath);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        alGenSources(KT_NSND, player->srcs);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        psrc = player->srcs;
        alSourcei (*psrc++, AL_BUFFER, *player->buffers);
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        alSourcei (*psrc++, AL_BUFFER, *(player->buffers + 1));
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        alSourcei (*psrc, AL_BUFFER, *(player->buffers + 2));
        if (alGetError() != AL_NO_ERROR)
        {
            kt_player_uninit (player, NULL);
            break;
        }

    } while (0);

}

void kt_player_alut_init (int *argc, char *argv[])
{
    alutInit (argc, argv);
}

gpointer
kt_player_load (KtPlayer *player, gpointer data)
{
    /* load configure file */
    do 
    {
        guint tmp_val;
        tmp_val = g_settings_get_uint (player->settings, "vol");
        if (tmp_val > 100) break;
        player->vol = tmp_val;
        tmp_val = g_settings_get_uint (player->settings, "style");
        if (tmp_val >= KT_NSND) break;
        player->cur_src = player->srcs + tmp_val;
    } while (0);
}

void
kt_player_save_config (KtPlayer *player)
{
    do 
    {
        guint tmp_val;
        if (!player || player->chg_etc != TRUE) break;
        g_settings_set_uint (player->settings, "vol", player->vol);
        tmp_val = player->cur_src - player->srcs;
        g_settings_set_uint (player->settings, "style", tmp_val);
    } while (0);
}

static void
kt_player_get_property (GObject      *object,
                       guint         prop_id,
                       GValue       *value,
                       GParamSpec   *pspec)
{
    KtPlayer *player = KT_PLAYER (object);
    switch (prop_id)
    {
    case PROP_VOLUMN:
        g_value_set_uint (value, player->vol);
        break;
    case PROP_SND_STYLE:
        g_value_set_uint (value, player->cur_src - player->srcs);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
kt_player_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    KtPlayer *player = KT_PLAYER (object);
    switch (prop_id)
    {
    case PROP_VOLUMN:
        if (player->vol != g_value_get_uint (value))
        {
            player->chg_etc = TRUE;
            player->vol = g_value_get_uint (value);
        }
        break;
    case PROP_SND_STYLE:
        if (g_value_get_uint (value) != player->cur_src - player->srcs)
        {
            player->chg_etc = TRUE;
            player->cur_src = player->srcs + g_value_get_uint (value);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
kt_player_class_init (KtPlayerClass *klass)
{
    klass->kt_exit = kt_player_uninit;
    klass->kt_load = kt_player_load;
    GObjectClass    *object_class = G_OBJECT_CLASS (klass);
    object_class->get_property = kt_player_get_property;
    object_class->set_property = kt_player_set_property;
    g_object_class_install_property (object_class,
                                     PROP_VOLUMN,
                                     g_param_spec_uint ("volumn",
                                                        "vol",
                                                        "the volumn of the sound",
                                                        0, 100, 50,
                                                        G_PARAM_READWRITE));
    g_object_class_install_property (object_class, 
                                     PROP_SND_STYLE,
                                     g_param_spec_uint ("sound-type",
                                                        "sound-style",
                                                        "the kind of the sound",
                                                        0, KT_NSND - 1, 1,
                                                        G_PARAM_READWRITE));

    signals[SIGNAL_EXIT] = g_signal_new ("kt-player-exit",
                                         G_TYPE_FROM_CLASS (object_class),
                                         G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                         G_STRUCT_OFFSET (KtPlayerClass, kt_exit),
                                         NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0,
                                         NULL);

    signals[SIGNAL_LOAD] = g_signal_new ("kt-player-load",
                                           G_TYPE_FROM_CLASS (object_class),
                                           G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                           G_STRUCT_OFFSET (KtPlayerClass, kt_load),
                                           NULL, NULL,
                                           g_cclosure_marshal_VOID__VOID,
                                           G_TYPE_NONE, 0,
                                           NULL);
}

void
kt_player_play (KtPlayer *player)
{
    alSourcePlay(*player->cur_src);
}
