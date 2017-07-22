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
#include "d2k/map_loader.h"
#include "d2k/map_sidedefs.h"
#include "d2k/texture.h"
#include "d2k/wad.h"

#define malformed_sidedefs_lump(status) status_error( \
  status,                                             \
  "d2k_map_sidedefs",                                 \
  D2K_MAP_SIDEDEFS_MALFORMED_LUMP,                    \
  "malformed SIDEDEFS lump"                           \
)

#define invalid_sidedef_sector_index(status) status_error( \
  status,                                                  \
  "d2k_map_sidedefs",                                      \
  D2K_MAP_SIDEDEFS_INVALID_SECTOR_INDEX,                   \
  "invalid sidedef index"                                  \
)

#define SIDEDEF_SIZE 30

bool d2k_map_loader_load_sidedefs(D2KMapLoader *map_loader, Status *status) {
  D2KLump *sidedefs_lump = map_loader->map_lumps[D2K_MAP_LUMP_SIDEDEFS];
  size_t sidedef_count = sidedefs_lump->data.len / SIDEDEF_SIZE;

  if ((sidedefs_lump->data.len % SIDEDEF_SIZE) != 0) {
    return malformed_sidedefs_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->sidedefs, sidedef_count,
                                                         status)) {
    return false;
  }

  for (size_t i = 0; i < sidedef_count; i++) {
    D2KSidedef *sidedef = array_append_fast(&map_loader->map->sidedefs);
    D2KTexture *texture = NULL;
    char top_texture[9];
    char bottom_texture[9];
    char mid_texture[9];
    char sidedef_data[SIDEDEF_SIZE];
    size_t sector_index;

    slice_read_fast(&sidedefs_lump->data, i * SIDEDEF_SIZE,
                                          SIDEDEF_SIZE,
                                          (void *)sidedef_data);

    sidedef->texture_offset = LUMP_DATA_SHORT_TO_FIXED(sidedef_data, 0);
    sidedef->row_offset = LUMP_DATA_SHORT_TO_FIXED(sidedef_data, 2);
    cbmemmove((void *)top_texture, (void *)&sidedef_data[4], 8);
    cbmemmove((void *)bottom_texture, (void *)&sidedef_data[12], 8);
    cbmemmove((void *)mid_texture, (void *)&sidedef_data[20], 8);
    sector_index = LUMP_DATA_SHORT_TO_INDEX(sidedef_data, 28);

    if (sector_index >= map_loader->map->sectors.len) {
      return invalid_sidedef_sector_index(status);
    }

    sidedef->sector = array_index_fast(
      &map_loader->map->sectors,
      sector_index
    );

    if (!d2k_lump_directory_lookup_texture(map_loader->lump_directory,
                                           top_texture,
                                           &texture,
                                           status)) {
      return false;
    }

    sidedef->top_texture = texture->index;

    if (!d2k_lump_directory_lookup_texture(map_loader->lump_directory,
                                           bottom_texture,
                                           &texture,
                                           status)) {
      return false;
    }

    sidedef->bottom_texture = texture->index;

    if (!d2k_lump_directory_lookup_texture(map_loader->lump_directory,
                                           mid_texture,
                                           &texture,
                                           status)) {
      return false;
    }

    sidedef->mid_texture = texture->index;
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
