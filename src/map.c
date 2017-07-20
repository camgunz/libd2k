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

#include "d2k/internal.h"
#include "d2k/map_blockmap.h"
#include "d2k/fixed_vertex.h"
#include "d2k/map.h"
#include "d2k/map_linedefs.h"
#include "d2k/map_nodes.h"
#include "d2k/map_sectors.h"
#include "d2k/map_segs.h"
#include "d2k/map_sidedefs.h"
#include "d2k/map_subsectors.h"
#include "d2k/wad.h"

#define map_not_found(status) status_error( \
  status,                                   \
  "d2k_map",                                \
  D2K_MAP_NOT_FOUND,                        \
  "map not found"                           \
)

#define map_missing_things_lump(status) status_error( \
  status,                                             \
  "d2k_map",                                          \
  D2K_MAP_MISSING_THINGS_LUMP,                        \
  "map missing THINGS lump"                           \
)

#define map_missing_linedefs_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_LINEDEFS_LUMP,                        \
  "map missing LINEDEFS lump"                           \
)

#define map_missing_sidedefs_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_SIDEDEFS_LUMP,                        \
  "map missing SIDEDEFS lump"                           \
)

#define map_missing_vertexes_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_VERTEXES_LUMP,                        \
  "map missing VERTEXES lump"                           \
)

#define map_missing_segs_lump(status) status_error( \
  status,                                           \
  "d2k_map",                                        \
  D2K_MAP_MISSING_SEGS_LUMP,                        \
  "map missing SEGS lump"                           \
)

#define map_missing_ssectors_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_SSECTORS_LUMP,                        \
  "map missing SSECTORS lump"                           \
)

#define map_missing_nodes_lump(status) status_error( \
  status,                                            \
  "d2k_map",                                         \
  D2K_MAP_MISSING_NODES_LUMP,                        \
  "map missing NODES lump"                           \
)

#define map_missing_sectors_lump(status) status_error( \
  status,                                              \
  "d2k_map",                                           \
  D2K_MAP_MISSING_SECTORS_LUMP,                        \
  "map missing SECTORS lump"                           \
)

#define map_missing_reject_lump(status) status_error( \
  status,                                             \
  "d2k_map",                                          \
  D2K_MAP_MISSING_REJECT_LUMP,                        \
  "map missing REJECT lump"                           \
)

#define map_missing_blockmap_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_BLOCKMAP_LUMP,                        \
  "map missing BLOCKMAP lump"                           \
)

#define map_missing_gl_vert_lump(status) status_error( \
  status,                                              \
  "d2k_map",                                           \
  D2K_MAP_GL_VERT_LUMP,                                \
  "map missing GL_VERT lump"                           \
)

#define map_missing_gl_segs_lump(status) status_error( \
  status,                                              \
  "d2k_map",                                           \
  D2K_MAP_MISSING_GL_SEGS_LUMP,                        \
  "map missing GL_SEGS lump"                           \
)

#define map_missing_gl_ssect_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_GL_SSECT_LUMP,                        \
  "map missing GL_SSECT lump"                           \
)

#define map_missing_gl_nodes_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_GL_NODES_LUMP,                        \
  "map missing GL_NODES lump"                           \
)

#define truncated_ssectors_lump(status) status_error( \
  status,                                             \
  "d2k_map",                                          \
  D2K_MAP_TRUNCATED_SSECTORS_LUMP,                    \
  "map has truncated SSECTORS lump"                   \
)

void d2k_map_init(D2KMap *map) {
  memset(map->wad_name, 0, sizeof(map->wad_name));
  memset(map->gl_wad_name, 0, sizeof(map->gl_wad_name));
  array_init(&map->vertexes, sizeof(D2KFixedVertex));
  array_init(&map->segs, sizeof(D2KSeg));
  array_init(&map->sectors, sizeof(D2KSector));
  array_init(&map->subsectors, sizeof(D2KSubsector));
  array_init(&map->nodes, sizeof(D2KMapNode));
  array_init(&map->linedefs, sizeof(D2KLinedef));
  array_init(&map->sidedefs, sizeof(D2KSidedef));
  array_init(&map->sslines, sizeof(D2KSegLine));
  d2k_blockmap_init(&map->blockmap);
}

void d2k_map_clear(D2KMap *map) {
  memset(map->wad_name, 0, sizeof(map->wad_name));
  memset(map->gl_wad_name, 0, sizeof(map->gl_wad_name));
  array_clear(&map->vertexes);
  array_clear(&map->segs);
  array_clear(&map->sectors);
  array_clear(&map->subsectors);
  array_clear(&map->nodes);
  array_clear(&map->linedefs);
  array_clear(&map->sidedefs);
  array_clear(&map->sslines);
  d2k_blockmap_clear(&map->blockmap);
}

/* vi: set et ts=2 sw=2: */
