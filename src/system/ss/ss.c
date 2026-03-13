#include "ss.h"
#include <string.h>
#include <stdio.h>

extern char sfxpath[];
extern int curstream;
extern int SS_TrackDone;

static int trackcountforintro = 2;
static int tracknext = 0;
static int tracknextvol = 0;
static int tracknextchan = 0;

void PlayStream(int num, int vol, int chan) {
    char str[64];
    char *cp;

    while ((cp = strchr(sfxpath, '\\')) != NULL) {
        *cp = '/';
    }

    if (tracknext != 0) {
        trackcountforintro = 0;
    }

    if (trackcountforintro == 1) {
        tracknext = num;
        tracknextvol = vol;
        tracknextchan = chan;
        return;
    }

    sprintf(str, "/sfx/%s.adp", sfxpath);
    SS_TrackStop(-1);
    SS_TrackFlushAll();
    SS_TrackAdd(str, 0);

    if (chan == 0) {
        SS_TrackSetRepeat(1);
    } else {
        SS_TrackSetRepeat(0);
    }

    SS_TrackPlay(chan);
    DTKSetVolume(vol, vol);

    if (trackcountforintro > 0) {
        trackcountforintro--;
    }

    curstream = num;
}

void StreamClear(void) {
    SS_TrackStop(-1);
    SS_TrackFlushAll();
}

void SS_Update(int vol) {
    if (trackcountforintro == 1 && SS_TrackDone != 0) {
        trackcountforintro = 0;
        PlayStream(tracknext, vol, tracknextchan);
    }

    DTKSetVolume(vol, vol);
    SS_UpdateSFX();
}