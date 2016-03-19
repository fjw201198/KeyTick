#include <unistd.h>
#include "../kt_player.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    kt_player_alut_init(&argc, argv);
    KtPlayer *player = kt_player_new();
    while (1)
    {
        kt_player_play(player);
        usleep (400);
    }
    gtk_main();
    return 0;
}
