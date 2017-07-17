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
#include "d2k/wad.h"

#define LINEDEF_SIZE 14

#define BOXTOP    0
#define BOXBOTTOM 1
#define BOXLEFT   2
#define BOXRIGHT  3

#define malformed_linedefs_lump(status) status_error( \
  status,                                             \
  "d2k_map_linedefs",                                 \
  D2K_MAP_LINEDEFS_MALFORMED_LUMP,                    \
  "malformed LINEDEFS lump"                           \
)

static inline float get_texel_distance(D2KFixedPoint dx, D2KFixedPoint dy) {
  float fx = d2k_fixed_point_to_float(dx);
  float fy = d2k_fixed_point_to_float(dy);
  return (float)((int)(0.5f + (float)sqrt(fx * fx + fy * fy)));
}

bool d2k_map_loader_load_linedefs(D2KMapLoader *map_loader, Status *status) {
  D2KLump *linedefs_lump = map_loader->map_lumps[D2K_MAP_LUMP_LINEDEFS];
  size_t linedef_count = linedefs_lump->data.len / LINEDEF_SIZE;

  if ((linedefs_lump->data.len % LINEDEF_SIZE) != 0) {
    return malformed_linedefs_lump(status);
  }

  if (!array_ensure_capacity(&map->linedefs, linedef_count, status)) {
    return false;
  }

  for (size_t i = 0; i < linedef_count; i++) {
    D2KLinedef *linedef = array_append_fast(&map->linedefs);
    char linedef_data[LINEDEF_SIZE];
    size_t start_vertex_index;
    size_t end_vertex_index;
    size_t front_sidedef_index;
    size_t back_sidedef_index;

    /* 65536 */

    slice_read_fast(&linedefs_lump->data, i * LINEDEF_SIZE,
                                          LINEDEF_SIZE,
                                          (void *)linedef_data);

    start_vertex_index = (size_t)cble16((linedef_data[0] << 8) |
                                        (linedef_data[1]));

    end_vertex_index = (size_t)cble16((linedef_data[2] << 8) |
                                      (linedef_data[3]));

    linedef->flags = cble16((linedef_data[4] << 8) | (linedef_data[5]));

    linedef->special = cble16((linedef_data[6] << 8) | (linedef_data[7]));

    tagged_sector_index = (size_t)cble16((linedef_data[8] << 8) |
                                         (linedef_data[9]));

    front_side_index = (size_t)cble16((linedef_data[10] << 8) |
                                      (linedef_data[11]));

    back_side_index = (size_t)cble16((linedef_data[12] << 8) |
                                     (linedef_data[13]));

    if (start_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_linedef_start_vertex_index(status);
    }

    if (end_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_linedef_end_vertex_index(status);
    }

    if (tagged_sector_index >= map_loader->map->sectors.len) {
      return invalid_linedef_tagged_sector_index(status);
    }

    if (front_side_index >= map_loader->map->sidedefs.len) {
      return invalid_linedef_front_side_index(status);
    }

    if (back_side_index >= map_loader->map->sidedefs.len) {
      return invalid_linedef_back_side_index(status);
    }

    linedef->start_vertex = array_index_fast(
      &map_loader->map->vertexes,
      start_vertex_index
    );

    linedef->end_vertex = array_index_fast(
      &map_loader->map->vertexes,
      end_vertex_index
    );

    linedef->tagged_sector = array_index_fast(
      &map_loader->map->sectors,
      tagged_sector_index
    );

    linedef->front_side = array_index_fast(
      &map_loader->map->sidedefs,
      front_side_index
    );

    linedef->back_side = array_index_fast(
      &map_loader->map->sidedefs,
      back_side_index
    );

    linedef->dx = linedef->end_vertex->x - linedef->start_vertex->x
    linedef->dy = linedef->end_vertex->y - linedef->start_vertex->y
    linedef->texel_length = get_texel-distance(linedef->dx, linedef->dy);
    linedef->tran_lump = -1;

    if (linedef->dx == 0) {
      linedef->slope = D2K_LINEDEF_SLOPE_TYPE_VERTICAL;
    }
    else if (linedef->dy == 0) {
      linedef->slope = D2K_LINEDEF_SLOPE_TYPE_HORIZONTAL;
    }
    else if (d2k_fixed_div(linedef->dy, linedef->dx) > 0) {
      linedef->slope = D2K_LINEDEF_SLOPE_TYPE_POSITIVE;
    }
    else {
      linedef->slope = D2K_LINEDEF_SLOPE_TYPE_NEGATIVE;
    }

    if (linedef->start_vertex->x < linedef->end_vertex->x) {
      linedef->bbox[BOXLEFT] = linedef->start_vertex->x;
      linedef->bbox[BOXRIGHT] = linedef->end_vertex->x;
    }
    else {
      linedef->bbox[BOXLEFT] = linedef->end_vertex->x;
      linedef->bbox[BOXRIGHT] = linedef->start_vertex->x;
    }

    if (linedef->start_vertex->y < linedef->end_vertex->y) {
      linedef->bbox[BOXBOTTOM] = linedef->start_vertex->y;
      linedef->bbox[BOXTOP] = linedef->end_vertex->y;
    }
    else if (linedef->start_vertex->y < linedef->end_vertex->y) {
      linedef->bbox[BOXBOTTOM] = linedef->end_vertex->y;
      linedef->bbox[BOXTOP] = linedef->start_vertex->y;
    }

    linedef->sound_origin.x = linedef->bbox[BOXLEFT] / 2 +
                              linedef->bbox[BOXRIGHT] / 2;
    linedef->sound_origin.y = linedef->bbox[BOXTOP] / 2 +
                              linedef->bbox[BOXBOTTOM] / 2;
    linedef->id = i;
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
