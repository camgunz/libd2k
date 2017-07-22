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

#include <math.h>

#include "d2k/fixed_math.h"
#include "d2k/fixed_vertex.h"
#include "d2k/map_loader.h"
#include "d2k/map_linedefs.h"
#include "d2k/wad.h"

#define malformed_linedefs_lump(status) status_error( \
  status,                                             \
  "d2k_map_linedefs",                                 \
  D2K_MAP_LINEDEFS_MALFORMED_LUMP,                    \
  "malformed LINEDEFS lump"                           \
)

#define invalid_linedef_start_vertex_index(status) status_error( \
  status,                                                        \
  "d2k_map_linedefs",                                            \
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_START_VERTEX_INDEX,           \
  "invalid linedef start vertex index"                           \
)

#define invalid_linedef_end_vertex_index(status) status_error( \
  status,                                                      \
  "d2k_map_linedefs",                                          \
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_END_VERTEX_INDEX,           \
  "invalid linedef end vertex index"                           \
)

#define invalid_linedef_tagged_sector_index(status) status_error( \
  status,                                                         \
  "d2k_map_linedefs",                                             \
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_TAGGED_SECTOR_INDEX,           \
  "invalid linedef tagged sector index"                           \
)

#define invalid_linedef_front_sidedef_index(status) status_error( \
  status,                                                         \
  "d2k_map_linedefs",                                             \
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_FRONT_SIDEDEF_INDEX,           \
  "invalid linedef front sidedef index"                           \
)

#define invalid_linedef_back_sidedef_index(status) status_error( \
  status,                                                        \
  "d2k_map_linedefs",                                            \
  D2K_MAP_LINEDEFS_INVALID_LINEDEF_BACK_SIDEDEF_INDEX,           \
  "invalid linedef back sidedef index"                           \
)

#define LINEDEF_SIZE 14

#define BOXTOP    0
#define BOXBOTTOM 1
#define BOXLEFT   2
#define BOXRIGHT  3

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

  if (!array_ensure_capacity(&map_loader->map->linedefs, linedef_count,
                                                         status)) {
    return false;
  }

  for (size_t i = 0; i < linedef_count; i++) {
    D2KLinedef *linedef = array_append_fast(&map_loader->map->linedefs);
    char linedef_data[LINEDEF_SIZE];
    size_t start_vertex_index;
    size_t end_vertex_index;
    size_t tagged_sector_index;
    size_t front_sidedef_index;
    size_t back_sidedef_index;

    /* 65536 */

    slice_read_fast(&linedefs_lump->data, i * LINEDEF_SIZE,
                                          LINEDEF_SIZE,
                                          (void *)linedef_data);

    start_vertex_index  = LUMP_DATA_SHORT_TO_INDEX(linedef_data, 0);
    end_vertex_index    = LUMP_DATA_SHORT_TO_INDEX(linedef_data, 2);
    linedef->flags      = LUMP_DATA_SHORT_TO_USHORT(linedef_data, 4);
    linedef->special    = LUMP_DATA_SHORT_TO_SHORT(linedef_data, 6);
    tagged_sector_index = LUMP_DATA_SHORT_TO_INDEX(linedef_data, 8);
    front_sidedef_index = LUMP_DATA_SHORT_TO_INDEX(linedef_data, 10);
    back_sidedef_index  = LUMP_DATA_SHORT_TO_INDEX(linedef_data, 12);

    if (start_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_linedef_start_vertex_index(status);
    }

    if (end_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_linedef_end_vertex_index(status);
    }

    if (tagged_sector_index >= map_loader->map->sectors.len) {
      return invalid_linedef_tagged_sector_index(status);
    }

    if (front_sidedef_index >= map_loader->map->sidedefs.len) {
      return invalid_linedef_front_sidedef_index(status);
    }

    if (back_sidedef_index >= map_loader->map->sidedefs.len) {
      return invalid_linedef_back_sidedef_index(status);
    }

    linedef->v1 = array_index_fast(
      &map_loader->map->vertexes,
      start_vertex_index
    );

    linedef->v2 = array_index_fast(
      &map_loader->map->vertexes,
      end_vertex_index
    );

    linedef->tagged_sector = array_index_fast(
      &map_loader->map->sectors,
      tagged_sector_index
    );

    linedef->front_side = array_index_fast(
      &map_loader->map->sidedefs,
      front_sidedef_index
    );

    linedef->back_side = array_index_fast(
      &map_loader->map->sidedefs,
      back_sidedef_index
    );

    linedef->dx = linedef->v2->x - linedef->v1->x;
    linedef->dy = linedef->v2->y - linedef->v1->y;
    linedef->texel_length = get_texel_distance(linedef->dx, linedef->dy);
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

    if (linedef->v1->x < linedef->v2->x) {
      linedef->bbox[BOXLEFT] = linedef->v1->x;
      linedef->bbox[BOXRIGHT] = linedef->v2->x;
    }
    else {
      linedef->bbox[BOXLEFT] = linedef->v2->x;
      linedef->bbox[BOXRIGHT] = linedef->v1->x;
    }

    if (linedef->v1->y < linedef->v2->y) {
      linedef->bbox[BOXBOTTOM] = linedef->v1->y;
      linedef->bbox[BOXTOP] = linedef->v2->y;
    }
    else if (linedef->v1->y < linedef->v2->y) {
      linedef->bbox[BOXBOTTOM] = linedef->v2->y;
      linedef->bbox[BOXTOP] = linedef->v1->y;
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
