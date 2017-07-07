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

#ifndef D2K_BLOCKMAP_H__
#define D2K_BLOCKMAP_H__

struct D2KMapStruct;
typedef struct D2KMapStruct D2KMap;

struct D2KLumpStruct;
typedef struct D2KLumpStruct D2KLump;

enum {
  D2K_BLOCKMAP_NEGATIVE_WIDTH,
  D2K_BLOCKMAP_NEGATIVE_HEIGHT,
  D2K_BLOCKMAP_TRUNCATED_HEADER,
  D2K_BLOCKMAP_TRUNCATED_LINE_LIST_DIRECTORY,
  D2K_BLOCKMAP_INVALID_OFFSET_IN_LINE_LIST_DIRECTORY,
};

typedef struct D2KBlockmapStruct {
  size_t        width;
  size_t        height;
  D2KFixedPoint origin_x;
  D2KFixedPoint origin_y;
  Array         blocks;
} D2KBlockmap;

bool d2k_blockmap_init_from_map(D2KBlockmap *bmap, D2KMap *map,
                                                   Status *status);
bool d2k_blockmap_init_from_lump(D2KBlockmap *bmap, D2KLump *lump,
                                                    Status *status);

#endif

/* vi: set et ts=2 sw=2: */