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
#include "d2k/map.h"
#include "d2k/map_loader.h"
#include "d2k/map_linedefs.h"
#include "d2k/map_segs.h"
#include "d2k/map_sidedefs.h"
#include "d2k/wad.h"

#define malformed_segs_lump(status) status_error( \
  status,                                         \
  "d2k_map_segs",                                 \
  D2K_MAP_SEGS_MALFORMED_LUMP,                    \
  "malformed SEGS lump"                           \
)

#define invalid_seg_start_vertex_index(status) status_error( \
  status,                                                    \
  "d2k_map_segs",                                            \
  D2K_MAP_SEGS_INVALID_SEG_START_VERTEX_INDEX,               \
  "invalid seg start vertex index"                           \
)

#define invalid_seg_end_vertex_index(status) status_error( \
  status,                                                  \
  "d2k_map_segs",                                          \
  D2K_MAP_SEGS_INVALID_SEG_END_VERTEX_INDEX,               \
  "invalid seg end vertex index"                           \
)

#define invalid_seg_linedef_index(status) status_error( \
  status,                                               \
  "d2k_map_segs",                                       \
  D2K_MAP_SEGS_INVALID_SEG_LINEDEF_INDEX,               \
  "invalid seg linedef index"                           \
)

#define invalid_seg_line_side(status) status_error(    \
  status,                                              \
  "d2k_map_segs",                                      \
  D2K_MAP_SEGS_INVALID_SEG_LINE_SIDE,                  \
  "invalid seg line side value (only 0 or 1 expected)" \
)

#define two_sided_seg_missing_other_side(status) status_error( \
  status,                                                      \
  "d2k_map_segs",                                              \
  D2K_MAP_SEGS_TWO_SIDED_SEG_MISSING_OTHER_SIDE,               \
  "2-sided seg missing the other side"                         \
)

#define VANILLA_SEG_SIZE 12

static inline D2KFixedPoint get_offset(D2KFixedVertex *v1,
                                       D2KFixedVertex *v2) {
  float a = d2k_fixed_point_to_float(v1->x - v2->x);
  float b = d2k_fixed_point_to_float(v2->y - v2->y);

  return d2k_float_to_fixed_point(sqrt(a * a + b * b));
}

bool d2k_map_loader_load_segs(D2KMapLoader *map_loader, Status *status) {
  D2KLump *segs_lump = map_loader->map_lumps[D2K_MAP_LUMP_SEGS];
  size_t seg_count = segs_lump->data.len / VANILLA_SEG_SIZE;

  if ((segs_lump->data.len % VANILLA_SEG_SIZE) != 0) {
    return malformed_segs_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->segs, seg_count, status)) {
    return false;
  }

  for (size_t i = 0; i < seg_count; i++) {
    D2KSeg *seg = array_append_fast(&map_loader->map->segs);
    char seg_data[VANILLA_SEG_SIZE];
    size_t start_vertex_index;
    size_t end_vertex_index;
    D2KAngle angle;
    size_t linedef_index;
    int16_t side;      
    D2KFixedPoint offset;
    D2KSidedef *other_sidedef = NULL;

    slice_read_fast(&segs_lump->data, i * VANILLA_SEG_SIZE, VANILLA_SEG_SIZE,
                                                            (void *)seg_data);
    start_vertex_index = LUMP_DATA_SHORT_TO_INDEX(seg_data,  0);
    end_vertex_index   = LUMP_DATA_SHORT_TO_INDEX(seg_data,  2);
    angle              = LUMP_DATA_SHORT_TO_ANGLE(seg_data,  4);
    linedef_index      = LUMP_DATA_SHORT_TO_INDEX(seg_data,  6);
    side               = LUMP_DATA_SHORT_TO_SHORT(seg_data,  8);
    offset             = LUMP_DATA_SHORT_TO_SHORT(seg_data, 10);

#if 0
    /* This is the code PrBoom+ uses to fix out-of-range vertex indices */
    if ((start_vertex_index >= map_loader->map->vertexes.len) ||
        (end_vertex_index >= map_loader->map->vertexes.len)) {
      if (seg->sidedef == &seg->linedef->front_side) {
        seg->v1 = seg->linedef->v1;
        seg->v2 = seg->linedef->v2;
      }
      else {
        seg->v1 = seg->linedef->v2;
        seg->v2 = seg->linedef->v1;
      }
    }
#endif

    if (start_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_seg_start_vertex_index(status);
    }

    if (end_vertex_index >= map_loader->map->vertexes.len) {
      return invalid_seg_end_vertex_index(status);
    }

    if (linedef_index >= map_loader->map->linedefs.len) {
      return invalid_seg_linedef_index(status);
    }

    if ((side != 0) && (side != 1)) {
      return invalid_seg_line_side(status);
    }

    seg->v1 = array_index_fast(&map_loader->map->vertexes, start_vertex_index);
    seg->v2 = array_index_fast(&map_loader->map->vertexes, end_vertex_index);
    seg->linedef = array_index_fast(&map_loader->map->linedefs, linedef_index);

    if (side == 0) {
      seg->sidedef = seg->linedef->front_side;
      other_sidedef = seg->linedef->back_side;
    }
    else if (side == 1) {
      seg->sidedef = seg->linedef->back_side;
      other_sidedef = seg->linedef->front_side;
    }

    seg->offset = offset;
    seg->angle = angle;

    /* figgi -- there are no minisegs in classic BSP nodes */
    seg->mini_seg = false;

    if (seg->sidedef) {
      seg->front_sector = seg->sidedef->sector;
    }
    else {
      seg->front_sector = NULL; /* [TODO] Warn? */
    }

    if (seg->linedef->flags & D2K_LINEDEF_FLAG_TWO_SIDED) {
      if (other_sidedef) {
        seg->back_sector = other_sidedef->sector;
      }
      else {
        /*
         * PrBoom+ does this to emulate vanilla behavior
         * 
         * seg->back_sector = GetSectorAtNullAddress();
         *
         */
        return two_sided_seg_missing_other_side(status);
      }
    }
    else {
      seg->back_sector = NULL;
    }

    /*
     * Recalculate seg offsets that are sometimes incorrect with certain
     * nodebuilders. Fixes among others, line 20365 of DV.wad, map 5
     */

    if (side == 0) {
      seg->offset = get_offset(seg->v1, seg->linedef->v1);
    }
    else {
      seg->offset = get_offset(seg->v1, seg->linedef->v2);
    }
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
