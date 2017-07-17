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

#ifndef D2K_MAP_BLOCKMAP_H__
#define D2K_MAP_BLOCKMAP_H__

#include "d2k/fixed_math.h"

struct D2KLumpStruct;

enum {
  D2K_MAP_BLOCKMAP_NEGATIVE_WIDTH = 1,
  D2K_MAP_BLOCKMAP_NEGATIVE_HEIGHT,
  D2K_MAP_BLOCKMAP_TRUNCATED_HEADER,
  D2K_MAP_BLOCKMAP_TRUNCATED_LINE_LIST_DIRECTORY,
  D2K_MAP_BLOCKMAP_INVALID_OFFSET_IN_LINE_LIST_DIRECTORY,
};

typedef struct D2KBlockmapStruct {
  size_t        width;
  size_t        height;
  D2KFixedPoint origin_x;
  D2KFixedPoint origin_y;
  Array         blocks;
} D2KBlockmap;

bool d2k_blockmap_build(D2KBlockmap *bmap, Array *vertexes, Array *linedefs,
                                                            Status *status);
bool d2k_blockmap_load_from_lump(D2KBlockmap *bmap, struct D2KLumpStruct *lump,
                                                    Status *status);
bool d2k_map_loader_build_blockmap(struct D2KMapLoaderStruct *map_loader,
                                   Status *status);
bool d2k_map_loader_load_blockmap(struct D2KMapLoaderStruct *map_loader,
                                  Status *status);

#endif

/* vi: set et ts=2 sw=2: */
