#ifndef KEY_TICK_KT_PLAYER_H_
#define KEY_TICK_KT_PLAYER_H_

#include <gtk/gtk.h>
#define KT_TYPE_PLAYER              (kt_player_get_type ())
#define KT_PLAYER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), KT_TYPE_PLAYER, KtPlayer))
#define KT_IS_PLAYER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), KT_TYPE_PLAYER))
#define KT_PLAYER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((obj), KT_TYPE_PLAYER, KtPlayerClass))
#define KT_IS_PLAYER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((obj), KT_TYPE_PLAYER))
#define KT_PLAYER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), KT_TYPE_PLAYER, KtPlayerClass))

typedef struct _KtPlayer        KtPlayer;
typedef struct _KtPlayerClass   KtPlayerClass;

typedef enum
{
    KT_TOCK,
    KT_WATER,
    KT_TICK,
    KT_NSND
} KtSndKind;

GType           kt_player_get_type          (void);
void            kt_player_play              (KtPlayer *player);
KtPlayer *      kt_player_new               (void);
void            kt_player_save_config       (KtPlayer *player);
void            kt_player_alut_init         (int *argc, char*argv[]);

#endif
