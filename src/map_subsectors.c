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
#include "d2k/map.h"
#include "d2k/map_loader.h"
#include "d2k/map_nodes.h"
#include "d2k/wad.h"

#define malformed_subsectors_lump(status) status_error( \
  status,                                               \
  "d2k_map_subsectors",                                 \
  D2K_MAP_SUBSECTORS_MALFORMED_LUMP,                    \
  "malformed SUBSECTORS lump"                           \
)

#define VANILLA_SUBSECTOR_SIZE  4

bool d2k_map_loader_load_subsectors(D2KMapLoader *map_loader, Status *status) {
  D2KLump *subsectors_lump = map_loader->map_lumps[D2K_VANILLA_MAP_LUMP_SSECT];
  size_t subsectors_count = subsectors_lump->data.len / VANILLA_SUBSECTOR_SIZE;

  if ((subsectors_lump->data.len % VANILLA_SUBSECTOR_SIZE) != 0) {
    return malformed_subsectors_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->subsectors, subsector_count,
                                                           status)) {
    return false;
  }

  for (size_t i = 0; i < subsector_count; i++) {
    D2KSubsector *subsector = array_append_fast(&map_loader->map->subsectors);
    char subsector_data[VANILLA_SUBSECTOR_SIZE];
    size_t seg_count;
    size_t first_seg;

    slice_read_fast(&subsectors_lump->data, i * VANILLA_SUBSECTOR_SIZE,
                                            VANILLA_SUBSECTOR_SIZE,
                                            (void *)subsector_data);


    seg_count = LUMP_DATA_SHORT_TO_COUNT(subsector_data, 0);
    first_seg = LUMP_DATA_SHORT_TO_INDEX(subsector_data, 2);

    if ((first_seg + seg_count) > map_loader->map->segs.len) {
      return out_of_range_subsector_seg_list(status);
    }

    subsector->seg_count = seg_count;
    subsector->first_seg = first_seg;
  }
}

/* vi: set et ts=2 sw=2: */
