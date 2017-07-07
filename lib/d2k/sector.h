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

#ifndef D2K_SECTOR_H__
#define D2K_SECTOR_H__

typedef struct D2KSectorStruct {
  size_t             id;
  uint32_t           flags;
  D2KFixedPoint      floor_height;
  D2KFixedPoint      ceiling_height;
  int                next_tag;
  int                first_tag;
  int                sound_traversed;
  D2KMapObject      *sound_target;
  int                blockbox[4];
  int                bbox[4];
  degenmobj_t        sound_origin;
  int                valid_count;
  D2KMapObject      *things;
  int                friction;
  int                move_factor;
  void              *floor_data;
  void              *ceiling_data;
  void              *lighting_data;
  int                stair_lock;
  int                previous_sector;
  int                next_sector;
  int                height_sector;
  int                bottom_map;
  int                mid_map;
  int                top_map;
  D2KMapSectorNodes *touching_thinglist;
  PArray             lines;
  int                sky;
  D2KFixedPoint      floor_x_offset;
  D2KFixedPoint      floor_y_offset;
  D2KFixedPoint      ceiling_x_offset;
  D2KFixedPoint      ceiling_y_offset;
  int                floor_light_sector;
  int                ceiling_light_sector;
  short              floor_pic;
  short              ceiling_pic;
  short              light_level;
  short              special;
  short              old_special;
  short              tag;
  int                sector_floor_interpolation;
  int                sector_ceiling_interpolation;
  int                floor_panning_interpolation;
  int                ceiling_panning_interpolation;
  int                fakegroup[2];
} D2KSector;

#endif

/* vi: set et ts=2 sw=2: */