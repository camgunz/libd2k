/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/

#ifndef D2K_LINE_H__
#define D2K_LINE_H__

#include "d2k/sound_origin.h"

struct D2KSectorStruct;

typedef enum {
  D2K_SLOPE_TYPE_HORIZONTAL,
  D2K_SLOPE_TYPE_VERTICAL,
  D2K_SLOPE_TYPE_POSITIVE,
  D2K_SLOPE_TYPE_NEGATIVE
} D2KSlopeType;

typedef enum {
  D2K_LINE_RENDER_FLAG_TOP_TILE =  1,
  D2K_LINE_RENDER_FLAG_MID_TILE =  2,
  D2K_LINE_RENDER_FLAG_BOT_TILE =  4,
  D2K_LINE_RENDER_FLAG_IGNORE   =  8,
  D2K_LINE_RENDER_FLAG_CLOSED   = 16,
  D2K_LINE_RENDER_FLAG_ISOLATED = 32,
} D2KLineRenderFlags;

typedef struct D2KLineStruct {
  uint32_t                id;
  D2KFixedVertex         *v1;
  D2KFixedVertex         *v2;
  D2KFixedPoint           dx;
  D2KFixedPoint           dy;
  float                   texel_length;
  unsigned short          flags;
  short                   special;
  short                   tag;
  unsigned short          sidenum[2];
  D2KFixedPoint           bbox[4];
  D2KSlopeType            slope;
  struct D2KSectorStruct *front_sector;
  struct D2KSectorStruct *back_sector;
  int                     valid_count;
  void                   *special_data;
  int                     tran_lump;
  int                     first_tag;
  int                     next_tag;
  int                     r_valid_count;
  D2KLineRenderFlags      r_flags;
  D2KSoundOrigin          sound_origin;
} D2KLine;

#endif

/* vi: set et ts=2 sw=2: */
