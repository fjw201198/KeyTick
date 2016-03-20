#include <stdio.h>
#include <stdlib.h>
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

static const gchar *config     = "keytick.ini";

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
    GKeyFile    *settings;
    gchar       *confpath;
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
kt_player_get_cur_path (gchar **curpath, size_t* psize)
{
    /* get current path */
    gchar  path[50];
    gchar *pos = NULL;
    size_t fname_len = 0;
    int    flag = 1;
    bzero(path, 50);
    sprintf (path, "/proc/%d/exe", getpid());
    *curpath = g_malloc(PATH_MAX);
    *psize = readlink(path, *curpath, PATH_MAX);
    if (*psize < 0) 
    {
        g_free (*curpath);
    }
    pos = strrchr(*curpath, '/');
    *(pos + 1) = '\0';
    *psize = (pos - *curpath) + 1;
    return *curpath;
}

static gchar *
kt_player_get_file_path (const gchar *fname)
{
    gchar *filepath = NULL;
    do
    {
        gchar *pos = NULL;
        size_t path_size;
        size_t fname_len;
        filepath = g_malloc(PATH_MAX);
        kt_player_get_cur_path(&filepath, &path_size);
        fname_len = strlen(fname) + 1;
        /* find file in current directory */
        strcat(filepath, fname);
        if (access (filepath, F_OK) == 0) break;
        /* find file in ../sound, attention sizeof operator, it will bigger then strlen */
        pos = filepath + path_size;
        memmove (pos, "../sound/", sizeof("../sound"));
        pos = pos + sizeof("../sound");
        memmove (pos, fname, fname_len);
        if (access (filepath, F_OK) == 0) break;
        /* find file in /usr/share/keytick/sound */
        memmove (filepath, "/usr/share/keytick/", sizeof ("/usr/share/keytick"));
        pos = filepath + sizeof ("/usr/share/keytick");
        memmove (pos, fname, fname_len);
        if (access (filepath, F_OK) == 0) break;
        g_free (filepath);
        filepath = NULL;
    } while (0);
    return filepath;
}

static gchar *
kt_player_get_conf_path()
{
    gchar *curpath;
    do 
    {
        gchar *pos;
        size_t psize;
        size_t conf_len;
        gchar *home;
        int fd;
        kt_player_get_cur_path(&curpath, &psize);
        pos = curpath + psize;
        conf_len = strlen(config) + 1;
        memmove(pos, config, conf_len);
        if (access (curpath, F_OK) == 0) break;
        memmove(curpath, "/etc/", sizeof ("/etc"));
        pos = curpath + sizeof("/etc");
        memmove(pos, config, conf_len);
        if (access (curpath, F_OK) == 0) break;
        home = getenv("HOME");
        psize = strlen(home);
        memmove(curpath, home, psize);
        pos = curpath + psize;
        memmove(pos, "/.config/keytick/", sizeof("/.config/keytick"));
        pos = pos + sizeof(".config/keytick/");
        memmove(pos, config, conf_len);
        if (access (curpath, F_OK) == 0) break;
        fd = open(curpath, O_CREAT | O_RDWR);
        write(fd, "[KeyTick]\n", sizeof("[KeyTick]\n"));
        close(fd);
        g_free(curpath);
        curpath = NULL;
    } while (0);
    return curpath;
}

gpointer
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
        /* gchar *curpath; */
        /* gchar *pos; */
        /* size_t psize; */
        /* kt_player_get_cur_path(&curpath, &psize); */
        /* pos = curpath + psize; */
        /* memmove(pos, config, strlen(config)+1); */
        /* if (access (curpath, F_OK) != 0) */
        /* { */
        /*     memmove(curpath, "/etc/", sizeof ("/etc")); */
        /*     pos = curpath + sizeof("/etc"); */
        /*     memmove(pos, config, strlen(config)+1); */
        /*     printf("curpath: %s\n", curpath); */
        /*     if (access (curpath, F_OK) != 0) */
        /*     { */
        /*         printf("Error: can not find the config file\n"); */
        /*         break; */
        /*     } */
        /* } */
        player->confpath = kt_player_get_conf_path();
        player->settings = g_key_file_new();
        g_key_file_load_from_file(player->settings, 
                                  player->confpath, 
                                  G_KEY_FILE_KEEP_COMMENTS,
                                  NULL);

        player->srcs    = g_malloc (KT_NSND * sizeof (ALuint));
        player->buffers = g_malloc (KT_NSND * sizeof (ALuint));
        player->cur_src = player->srcs;
        kt_player_load(player, NULL);
        player->device  = alcOpenDevice (NULL);
        if (!player->device)
        {
            kt_player_uninit (player, NULL);
            break;
        }
        player->context = alcCreateContext (player->device, NULL);
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
        tmp_val = g_key_file_get_uint64 (player->settings, "KeyTick", "vol", NULL);
        if (tmp_val > 100) tmp_val = 100;
        player->vol = tmp_val;
        tmp_val = g_key_file_get_uint64 (player->settings, "KeyTick", "style", NULL);
        if (tmp_val >= KT_NSND) tmp_val = 1;
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
        g_key_file_set_uint64 (player->settings, "KeyTick", "vol", player->vol);
        tmp_val = player->cur_src - player->srcs;
        g_key_file_set_uint64 (player->settings, "KeyTick", "style", tmp_val);
        g_key_file_save_to_file(player->settings, player->confpath, NULL);
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
                                     g_param_spec_uint ("sound_type",
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

KtPlayer *
kt_player_new(void)
{
    KtPlayer *player;
    player = g_object_new(KT_TYPE_PLAYER, NULL);
    return player;
}

void
kt_player_play (KtPlayer *player)
{
    alSourcePlay(*player->cur_src);
}

void
kt_player_set_sound_type (KtPlayer *player, guint data)
{
    player->cur_src = player->srcs + data;
}
