#ifndef COOP_H
#define COOP_H

#include "coop_protocol_generated.h"

#define COOP_TEXT __attribute__((section(".coop_text")))
#define COOP_DATA __attribute__((section(".coop_data")))

#define COOP_AVATAR_USED 0x00000001u
#define COOP_AVATAR_DEAD 0x00000002u
#define COOP_AVATAR_INVISIBLE 0x00000004u

#define COOP_MOVE_SPIN 0x00000001u

struct creature_s;

extern struct CoopMailbox gCoopMailbox;

void CoopFrameUpdate(struct creature_s *plr);
void CoopDrawRemotePlayer(void);
void CoopUpdatePlayerStatsWrapper(struct creature_s *plr);
void CoopDrawCreaturesWrapper(struct creature_s *c, int count, int render, int shadow);
void CoopDrawPanelTextWrapper(int bypass);

#endif
