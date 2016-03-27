AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=keytick
keytick_SOURCES=kt_main.c kt_keyevt.c kt_key_listener.c kt_player.c kt_settings_ui.c
AM_CFLAGS=$(shell pkg-config gtk+-3.0 --cflags)
keytick_LDFLAGS=$(shell pkg-config gtk+-3.0 --libs) -lopenal -lalut -lX11

configdir=/etc/keytick
config_DATA=../bin/keytick.ini

sounddir=/usr/share/keytick/sound
sound_DATA=../sound/*
