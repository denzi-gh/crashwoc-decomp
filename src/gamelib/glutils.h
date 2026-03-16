#pragma once

#include "../types.h"
#include "../nu.h"
#include "../nu3dx/numtl.h"
#include "../nu3dx/nutex.h"


/*
  8008c560 000118 8008c560  4 CreateAlphaBlendTexture256_32 	Global
  8008c678 000134 8008c678  4 CreateAlphaBlendTexture256128_32 	Global
  8008c7ac 000110 8008c7ac  4 CreateAlphaBlendTexture64 	Global
*/

extern s32 DoNotSiwzzle;

struct numtl_s *CreateAlphaBlendTexture256_32(char *fname, s32 uvmode, s32 alpha, s32 pri, s32 zmode);
struct numtl_s *CreateAlphaBlendTexture64(char *fname, s32 uvmode, s32 alpha, s32 pri);
struct numtl_s *CreateAlphaBlendTexture256128_32(char *fname, s32 uvmode, s32 alpha, s32 pri, s32 zmode);
