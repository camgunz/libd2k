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
#include "d2k/map_nodes.h"
#include "d2k/wad.h"

enum {
  D2K_MAP_NOT_FOUND = 1,
  D2K_MAP_MISSING_THINGS_LUMP,
  D2K_MAP_MISSING_LINEDEFS_LUMP,
  D2K_MAP_MISSING_SIDEDEFS_LUMP,
  D2K_MAP_MISSING_VERTEXES_LUMP,
  D2K_MAP_MISSING_SEGS_LUMP,
  D2K_MAP_MISSING_SSECTORS_LUMP,
  D2K_MAP_MISSING_NODES_LUMP,
  D2K_MAP_MISSING_SECTORS_LUMP,
  D2K_MAP_MISSING_REJECT_LUMP,
  D2K_MAP_MISSING_BLOCKMAP_LUMP,
  D2K_MAP_MISSING_GL_VERT_LUMP,
  D2K_MAP_MISSING_GL_SEGS_LUMP,
  D2K_MAP_MISSING_GL_SSECT_LUMP,
  D2K_MAP_MISSING_GL_NODES_LUMP,
  D2K_MAP_JUMBLED_LUMPS,
  D2K_MAP_MULTIPLE_NODES_VERSIONS,
};

typedef struct D2KMapLoaderStruct {
  D2KMap             *map;
  D2KLumpDirectory   *lump_directory;
  D2KLump            *map_lumps[D2K_VANILLA_MAP_LUMP_MAX];
  D2KLump            *gl_map_lumps[D2K_GL_MAP_LUMP_MAX];
  D2KLump            *udmf_start_map_lump;
  D2KLump            *udmf_end_map_lump;
  D2KMapNodesVersion  nodes_version;
} D2KMapLoader;

bool d2k_map_loader_load_map(D2KMapLoader *map_loader,
                             D2KMap *map,
                             D2KLumpDirectory *lump_directory,
                             const char *map_name,
                             Status *status);

static inline
size_t d2k_map_loader_vanilla_lump_offset(D2KMapLoader *map_loader) {
  return map_loader->map_lumps[D2K_VANILLA_MAP_LUMP_MAP]->index;
}

static inline
size_t d2k_map_loader_gl_lump_offset(D2KMapLoader *map_loader) {
  return map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_MAP]->index;
}

static inline
bool d2k_map_loader_get_vanilla_lump(D2KMapLoader *map_loader,
                                     size_t vanilla_index,
                                     D2KLump **lump,
                                     Status *status) {
  return d2k_lump_directory_index(
    map_loader->lump_directory,
    d2k_map_loader_vanilla_lump_offset(map_loader) + vanilla_index,
    lump,
    status
  );
}

static inline
bool d2k_map_loader_get_gl_lump(D2KMapLoader *map_loader, size_t gl_index,
                                                          D2KLump **lump,
                                                          Status *status) {
  return d2k_lump_directory_index(
    map_loader->lump_directory,
    d2k_map_loader_gl_lump_offset(map_loader) + gl_index,
    lump,
    status
  );
}

static inline
bool d2k_map_loader_has_gl_lumps(D2KMapLoader *map_loader) {
    return map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_MAP] != NULL;
}

static inline
bool d2k_map_loader_has_udmf_lumps(D2KMapLoader *map_loader) {
    return map_loader->udmf_start_map_lump != NULL;
}

#endif

/* vi: set et ts=2 sw=2: */
