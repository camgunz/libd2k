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

#ifndef D2K_MAP_SEGS_H__
#define D2K_MAP_SEGS_H__

#include "d2k/fixed_math.h"

enum {
  D2K_MAP_SEGS_MALFORMED_LUMP = 1,
  D2K_MAP_SEGS_INVALID_SEG_START_VERTEX_INDEX,
  D2K_MAP_SEGS_INVALID_SEG_END_VERTEX_INDEX,
  D2K_MAP_SEGS_INVALID_SEG_LINEDEF_INDEX,
  D2K_MAP_SEGS_INVALID_SEG_LINE_SIDE,
};

typedef struct D2KSegStruct {
  struct D2KFixedVertexStruct *v1;
  struct D2KFixedVertexStruct *v2;
  D2KFixedPoint                offset;
  D2KAngle                     angle;
  D2KAngle                     pangle;
  int64_t                      length;
  struct D2KSidedefStruct     *sidedef;
  struct D2KLinedefStruct     *linedef;
  bool                         mini_seg;
  struct D2KSectorStruct      *front_sector;
  struct D2KSectorStruct      *back_sector;
} D2KSeg;

bool d2k_map_loader_load_segs(struct D2KMapLoaderStruct *map_loader,
                              Status *status);

#endif

/* vi: set et ts=2 sw=2: */
