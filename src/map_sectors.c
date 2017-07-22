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
#include "d2k/map_sectors.h"
#include "d2k/wad.h"

#define malformed_sectors_lump(status) status_error( \
  status,                                            \
  "d2k_map_sectors",                                 \
  D2K_MAP_SECTORS_MALFORMED_LUMP,                    \
  "malformed SECTORS lump"                           \
)

#define SECTOR_SIZE 26

bool d2k_map_loader_load_sectors(D2KMapLoader *map_loader, Status *status) {
  D2KLump *sectors_lump = map_loader->map_lumps[D2K_MAP_LUMP_SECTORS];
  size_t sector_count = sectors_lump->data.len / SECTOR_SIZE;

  if ((sectors_lump->data.len % SECTOR_SIZE) != 0) {
    return malformed_sectors_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->sectors, sector_count,
                                                        status)) {
    return false;
  }

  for (size_t i = 0; i < sector_count; i++) {
    D2KSector *sector = array_append_fast(&map_loader->map->sectors);
    char sector_data[SECTOR_SIZE];
    char floor_texture[9] = { 0 };
    char ceiling_texture[9] = { 0 };
    D2KLump *flat_lump = NULL;

    sector->id = i;

    slice_read_fast(&sectors_lump->data, i * SECTOR_SIZE,
                                         SECTOR_SIZE,
                                         (void *)sector_data);

    sector->floor_height = LUMP_DATA_SHORT_TO_FIXED(sector_data, 0);
    sector->ceiling_height = LUMP_DATA_SHORT_TO_FIXED(sector_data, 2);
    cbmemmove((void *)floor_texture, (void *)&sector_data[4], 8);
    cbmemmove((void *)ceiling_texture, (void *)&sector_data[12], 8);
    sector->light_level = LUMP_DATA_SHORT_TO_FIXED(sector_data, 20);
    sector->special = LUMP_DATA_SHORT_TO_FIXED(sector_data, 22);
    sector->tag = LUMP_DATA_SHORT_TO_FIXED(sector_data, 24);

    if (!d2k_lump_directory_lookup_ns(map_loader->lump_directory,
                                      floor_texture,
                                      D2K_LUMP_NAMESPACE_FLATS,
                                      &flat_lump,
                                      status)) {
      return false;
    }

    sector->floor_texture = flat_lump->index;

    if (!d2k_lump_directory_lookup_ns(map_loader->lump_directory,
                                      ceiling_texture,
                                      D2K_LUMP_NAMESPACE_FLATS,
                                      &flat_lump,
                                      status)) {
      return false;
    }

    sector->ceiling_texture = flat_lump->index;
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
