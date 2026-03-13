#ifndef SS_H
#define SS_H

#include "../../types.h"

/*
  800cc180 000110 800cc180  4 PlayStream 	Global
  800cc290 00002c 800cc290  4 StreamClear 	Global
  800cc2bc 00006c 800cc2bc  4 SS_Update 	Global
*/

void PlayStream(int num, int vol, int chan);
void StreamClear(void);
void SS_Update(int vol);

#endif // !SS_H
