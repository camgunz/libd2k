
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

#ifndef D2K_MAP_SECTORS_H__
#define D2K_MAP_SECTORS_H__

#include "d2k/fixed_math.h"
#include "d2k/sound_origin.h"

struct D2KMapObjectStruct;
struct D2KMapSectorNodeStruct;

enum {
  D2K_MAP_SECTORS_MALFORMED_LUMP = 1,
};

typedef struct D2KSectorStruct {
  uint32_t                        id;
  uint32_t                        flags;
  D2KFixedPoint                   floor_height;
  D2KFixedPoint                   ceiling_height;
  int                             next_tag;
  int                             first_tag;
  int                             sound_traversed;
  struct D2KMapObjectStruct      *sound_target;
  int                             blockbox[4];
  int                             bbox[4];
  D2KSoundOrigin                  sound_origin;
  int                             valid_count;
  struct D2KMapObjectStruct      *things;
  int                             friction;
  int                             move_factor;
  void                           *floor_data;
  void                           *ceiling_data;
  void                           *lighting_data;
  int                             stair_lock;
  int                             previous_sector;
  int                             next_sector;
  int                             height_sector;
  int                             bottom_map;
  int                             mid_map;
  int                             top_map;
  struct D2KMapSectorNodeStruct  *touching_thinglist;
  PArray                          lines;
  int                             sky;
  D2KFixedPoint                   floor_x_offset;
  D2KFixedPoint                   floor_y_offset;
  D2KFixedPoint                   ceiling_x_offset;
  D2KFixedPoint                   ceiling_y_offset;
  int                             floor_light_sector;
  int                             ceiling_light_sector;
  size_t                          floor_texture;
  size_t                          ceiling_texture;
  short                           light_level;
  short                           special;
  short                           old_special;
  short                           tag;
  int                             sector_floor_interpolation;
  int                             sector_ceiling_interpolation;
  int                             floor_panning_interpolation;
  int                             ceiling_panning_interpolation;
  int                             fakegroup[2];
} D2KSector;

bool d2k_map_loader_load_sectors(struct D2KMapLoaderStruct *map_loader,
                                 Status *status);

#endif

/* vi: set et ts=2 sw=2: */
