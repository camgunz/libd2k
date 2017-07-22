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

#ifndef D2K_MAP_LINEDEFS_H__
#define D2K_MAP_LINEDEFS_H__

#include "d2k/fixed_math.h"
#include "d2k/sound_origin.h"

enum {
  D2K_MAP_LINEDEFS_MALFORMED_LUMP = 1,
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_START_VERTEX_INDEX,
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_END_VERTEX_INDEX,
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_TAGGED_SECTOR_INDEX,
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_FRONT_SIDEDEF_INDEX,
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_BACK_SIDEDEF_INDEX,
  D2K_MAP_SEGS_TWO_SIDED_SEG_MISSING_OTHER_SIDE,
};

typedef enum {
  D2K_LINEDEF_FLAG_BLOCKING        = (1 << 0),
  D2K_LINEDEF_FLAG_BLOCK_MONSTERS  = (1 << 1),
  D2K_LINEDEF_FLAG_TWO_SIDED       = (1 << 2),
  D2K_LINEDEF_FLAG_DONT_PEG_TOP    = (1 << 3),
  D2K_LINEDEF_FLAG_DONT_PEG_BOTTOM = (1 << 4),
  D2K_LINEDEF_FLAG_SECRET          = (1 << 5),
  D2K_LINEDEF_FLAG_SOUND_BLOCK     = (1 << 6),
  D2K_LINEDEF_FLAG_DONT_DRAW       = (1 << 7),
  D2K_LINEDEF_FLAG_MAPPED          = (1 << 8),
  D2K_LINEDEF_FLAG_PASS_USE        = (1 << 9),
} D2KLinedefFlag;

typedef enum {
  D2K_LINEDEF_SLOPE_TYPE_HORIZONTAL,
  D2K_LINEDEF_SLOPE_TYPE_VERTICAL,
  D2K_LINEDEF_SLOPE_TYPE_POSITIVE,
  D2K_LINEDEF_SLOPE_TYPE_NEGATIVE
} D2KLinedefSlopeType;

typedef enum {
  D2K_LINEDEF_RENDER_FLAG_TOP_TILE =  1,
  D2K_LINEDEF_RENDER_FLAG_MID_TILE =  2,
  D2K_LINEDEF_RENDER_FLAG_BOT_TILE =  4,
  D2K_LINEDEF_RENDER_FLAG_IGNORE   =  8,
  D2K_LINEDEF_RENDER_FLAG_CLOSED   = 16,
  D2K_LINEDEF_RENDER_FLAG_ISOLATED = 32,
} D2KLinedefRenderFlags;

typedef struct D2KLinedefStruct {
  uint32_t                     id;
  struct D2KFixedVertexStruct *v1;
  struct D2KFixedVertexStruct *v2;
  D2KFixedPoint                dx;
  D2KFixedPoint                dy;
  float                        texel_length;
  uint16_t                     flags;
  int16_t                      special;
  struct D2KSectorStruct      *tagged_sector;
  struct D2KSidedefStruct     *front_side;
  struct D2KSidedefStruct     *back_side;
  D2KFixedPoint                bbox[4];
  D2KLinedefSlopeType          slope;
  struct D2KSectorStruct      *front_sector;
  struct D2KSectorStruct      *back_sector;
  int                          valid_count;
  void                        *special_data;
  int                          tran_lump;
  int                          first_tag;
  int                          next_tag;
  int                          r_valid_count;
  D2KLinedefRenderFlags        r_flags;
  D2KSoundOrigin               sound_origin;
} D2KLinedef;

bool d2k_map_loader_load_linedefs(struct D2KMapLoaderStruct *map_loader,
                                  Status *status);

#endif

/* vi: set et ts=2 sw=2: */
