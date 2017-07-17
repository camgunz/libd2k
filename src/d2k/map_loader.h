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

#ifndef D2K_MAP_LOADER_H__
#define D2K_MAP_LOADER_H__

#include "d2k/map.h"
#include "d2k/wad.h"

struct D2KMapStruct;

typedef struct D2KMapLoaderStruct {
  struct D2KMapStruct *map;
  D2KLump             *map_lumps[D2K_MAP_LUMP_VANILLA_MAX];
  D2KLump             *glmap_lumps[D2K_MAP_LUMP_GL_MAX];
  D2KLump             *udmf_start_map_lump;
  D2KLump             *udmf_end_map_lump;
  D2KMapNodesVersion   nodes_version;
} D2KMapLoader;

bool d2k_map_loader_load_map(D2KMapLoader *map_loader,
                             struct D2KMapStruct *map,
                             D2KLumpDirectory *lump_directory,
                             const char *map_name,
                             Status *status);

bool d2k_map_loader_has_gl_lumps(D2KMapLoader *map_loader);

static size_t d2k_map_loader_vanilla_lump_offset(D2KMapLoader *map_loader) {
  return map_loader->map_lumps[D2K_MAP_LUMP_VANILLA_MAP]->index;
}

static size_t d2k_map_loader_gl_lump_offset(D2KMapLoader *map_loader) {
  return map_loader->gl_map_lumps[D2K_MAP_LUMP_GL_MAP]->index;
}

#endif

/* vi: set et ts=2 sw=2: */
