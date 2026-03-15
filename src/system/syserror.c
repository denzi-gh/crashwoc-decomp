#include "system/gs.h"

#define halt_baddata() //asm(".long 1")

char Lockupbuffer1[256];
char Lockupbuffer2[256];
char Lockupbuffer3[256];

void DisplayErrorAndLockup(char *nufile,int line,char *txt) {
    int i;

    sprintf(Lockupbuffer1,"FILE: %s\n",nufile);
    sprintf(Lockupbuffer2,"LINE: %d\n",line);
    sprintf(Lockupbuffer3,"MSG:  %s\n",txt);
    for (i = 0; i < 10; i++) {
        GS_BeginScene();
        GS_EndScene();
        GS_FlipScreen();
    }
    halt_baddata();
}
