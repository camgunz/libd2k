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

#ifndef D2K_MAP_THINGS_H__
#define D2K_MAP_THINGS_H__

enum {
  D2K_MAP_THINGS_MALFORMED_LUMP = 1,
};

typedef struct D2KMapThingStruct {
  int16_t x;
  int16_t y;
  int16_t angle;
  int16_t type;
  int16_t options;
} D2KMapThing;

bool d2k_map_loader_load_things(struct D2KMapLoaderStruct *map_loader,
                                Status *status);

#endif

/* vi: set et ts=2 sw=2: */
