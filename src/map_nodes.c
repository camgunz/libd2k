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
#include "d2k/map_nodes.h"
#include "d2k/wad.h"

#define malformed_nodes_lump(status) status_error( \
  status,                                          \
  "d2k_map_nodes",                                 \
  D2K_MAP_NODES_MALFORMED_LUMP,                    \
  "malformed NODES lump"                           \
)

#define multiple_nodes_versions(status) status_error( \
  status,                                             \
  "d2k_map_nodes",                                    \
  D2K_MAP_NODES_MULTIPLE_VERSIONS,                    \
  "multiple nodes versions detected"                  \
)

#define unknown_nodes_version(status) status_error( \
  status,                                           \
  "d2k_map_nodes",                                  \
  D2K_MAP_NODES_UNKNOWN_NODES_VERSION,              \
  "unknown nodes version"                           \
)

#define zdoom_extended_compressed_nodes_not_implemented(status) status_error( \
  status,                                                                     \
  "d2k_map_nodes",                                                            \
  D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_NODES_NOT_IMPLEMENTED,              \
  "ZDoom extended compressed nodes not implemented"                           \
)

#define zdoom_extended_gl_nodes_not_implemented(status) status_error( \
  status,                                                             \
  "d2k_map_nodes",                                                    \
  D2K_MAP_NODES_ZDOOM_EXTENDED_GL_NODES_NOT_IMPLEMENTED,              \
  "ZDoom extended GL nodes not implemented"                           \
)

#define zdoom_extended_compressed_gl_nodes_not_implemented(status)    \
  status_error(                                                       \
    status,                                                           \
    "d2k_map_nodes",                                                  \
    D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_GL_NODES_NOT_IMPLEMENTED, \
    "ZDoom extended compressed GL nodes not implemented"              \
  )

#define zdoom_extended_gl_udmf_nodes_not_implemented(status) status_error( \
  status,                                                                  \
  "d2k_map_nodes",                                                         \
  D2K_MAP_NODES_ZDOOM_EXTENDED_GL_UDMF_NODES_NOT_IMPLEMENTED,              \
  "ZDoom extended GL UDMF nodes not implemented"                           \
)

#define zdoom_extended_compressed_gl_udmf_nodes_not_implemented(status)    \
  status_error(                                                            \
    status,                                                                \
    "d2k_map_nodes",                                                       \
    D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_GL_UDMF_NODES_NOT_IMPLEMENTED, \
    "ZDoom extended compressed GL UDMF nodes not implemented"              \
  )

#define VANILLA_SEG_SIZE 12
#define VANILLA_SUBSECTOR_SIZE 4

typedef struct D2KMapNodeVersionHeaderInfoStruct {
  D2KMapLump         lump;
  const char        *version_header;
  size_t             version_header_size;
} D2KMapNodeVersionHeaderInfo;

D2KMapNodeVersionHeaderInfo
d2k_map_node_version_headers[D2K_MAP_NODES_VERSION_MAX] = {
  {D2K_MAP_LUMP_NONE,     "",             0},
  {D2K_MAP_LUMP_NODES,    "xNd4\0\0\0\0", 8},
  {D2K_MAP_LUMP_NODES,    "XNOD",         4},
  {D2K_MAP_LUMP_NODES,    "ZNOD",         4},
  {D2K_MAP_LUMP_SSECTORS, "XGLN",         4},
  {D2K_MAP_LUMP_SSECTORS, "ZGLN",         4},
  {D2K_MAP_LUMP_ZNODES,   "XGL2",         4},
  {D2K_MAP_LUMP_ZNODES,   "ZGL2",         4},
  {D2K_MAP_LUMP_NONE,     "",             0},
  {D2K_MAP_LUMP_GL_VERT   "gNd2",         4},
  {D2K_MAP_LUMP_GL_SEGS   "gNd3",         4},
  {D2K_MAP_LUMP_GL_VERT   "gNd4",         4},
  {D2K_MAP_LUMP_GL_VERT   "gNd5",         4},
};

static inline bool lump_starts_with(D2KLump *lump,
                                    const char *data,
                                    size_t len,
                                    bool *starts_with,
                                    Status *status) {
  return slice_equals_data_at(&lump->data, 0, data, len, starts_with, status);
}

static bool load_subsectors(D2KMapLoader *map_loader, Status *status) {
  int  i;
  D2KLump *subsectors_lump = map_loader->map_lumps[D2K_MAP_LUMP_SSECT];
  size_t subsectors_count = subsectors_lump->data.len / VANILLA_SUBSECTOR_SIZE;

  if ((subsectors_lump->data.len % VANILLA_SUBSECTOR_SIZE) != 0) {
    return malformed_subsectors_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->subsectors, subsector_count,
                                                           status)) {
    return false;
  }

  for (size_t i = 0; i < subsector_count; i++) {
    D2KSubsector *subsector = array_append_fast(
      &map_loader->map->subsectors
    );
    char subsector_data[VANILLA_SUBSECTOR_SIZE];
    size_t seg_count;
    size_t first_seg;

    slice_read_fast(&subsectors_lump->data, i * VANILLA_SUBSECTOR_SIZE,
                                            VANILLA_SUBSECTOR_SIZE,
                                            (void *)subsector_data);

    seg_count = (size_t)cble16((subsector_data[0] << 8) | (subsector_data[1]));
    first_seg = (size_t)cble16((subsector_data[0] << 8) | (subsector_data[1]));

    if ((first_seg + seg_count) > map_loader->map->segs.len) {
      return out_of_range_subsector_seg_list(status);
    }

    subsector->seg_count = seg_count;
    subsector->first_seg = first_seg;
  }
}

static bool load_nodes(D2KMapLoader *map_loader, Status *status) {
}

static bool load_segs(D2KMapLoader *map_loader, Status *status) {
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
    size_t vertex_start_index;
    size_t vertex_end_index;
    D2KAngle angle;
    size_t linedef_index;
    int16_t side;      
    D2KFixedPoint offset;
    D2KSidedef *other_side = NULL;

    slice_read_fast(&segs_lump->data, i * VANILLA_SEG_SIZE, VANILLA_SEG_SIZE,
                                                            (void *)seg_data);
    start_vertex_index = (size_t)cble16((seg_data[0] << 8) | (seg_data[1]));

    end_vertex_index   = (size_t)cble16((seg_data[2] << 8) | (seg_data[3]));

    angle = cble16((seg_data[4] << 8) | (seg_data[5])) << 16;

    linedef_index = (size_t)cble16((seg_data[6]  << 8) | (seg_data[7]));

    side = cble16((seg_data[8] << 8) | (seg_data[9]));

    offset = d2k_int_to_fixed_point(cble16((seg_data[10] << 8) |
                                           (seg_data[11])));

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

    if (seg->linedef->flags & D2K_LINEDEF_FLAGS_TWO_SIDED) {
      if (!other_side) {
        seg->back_sector = GetSectorAtNullAddress(); /* this is wrong */
      }
      else {
        seg->back_sector = other_sidedef->sector;
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
}

static bool load_gl_subsectors(D2KMapLoader *map_loader, Status *status) {
}

static bool load_gl_nodes(D2KMapLoader *map_loader, Status *status) {
}

static bool load_gl_segs(D2KMapLoader *map_loader, Status *status) {
}

static bool load_deep_bsp_subsectors(D2KMapLoader *map_loader,
                                     Status *status) {
}

static bool load_deep_bsp_nodes(D2KMapLoader *map_loader, Status *status) {
}

static bool load_deep_bsp_segs(D2KMapLoader *map_loader, Status *status) {
}

/*
 * ZDoom extended nodes:
 * - Everything's in NODES
 * - vertexes:
 *   - header:
 *     - uint32_t used_vertex_count
 *     - uint32_t new_vertex_count
 *   - body (new_vertex_count times):
 *     - D2KFixedPoint x
 *     - D2KFixedPoint y
 * - subsectors:
 *   - header:
 *     - uint32_t subsector_count
 *   - body (subsector_count times)
 *     - uint32_t seg_count
 * - segs:
 *   - header:
 *     - uint32_t seg_count
 *   - body (seg_count times)
 *     - uint32_t first_vertex_index
 *     - uint32_t second_vertex_index
 *     - uint16_t linedef_index
 *     - uint8_t front_or_back (0 or 1)
 * - nodes:
 *   - header:
 *     - uint32_t node_count
 *   - body (node_count times)
 *     - int16_t x
 *     - int16_t y
 *     - int16_t dx
 *     - int16_t dy
 *     - int16_t top1
 *     - int16_t bottom1
 *     - int16_t left1
 *     - int16_t right1
 *     - int16_t top2
 *     - int16_t bottom2
 *     - int16_t left2
 *     - int16_t right2
 *     - uint32_t node_or_subsector_child_1
 *     - uint32_t node_or_subsector_child_2
 */

static bool load_zdoom_subsectors(D2KMapLoader *map_loader, Status *status) {
}

static bool load_zdoom_nodes(D2KMapLoader *map_loader, Status *status) {
}

static bool load_zdoom_segs(D2KMapLoader *map_loader, Status *status) {
}

bool d2k_map_loader_detect_nodes_version(D2KMapLoader *map_loader,
                                         Status *status) {
  bool               node_type_found[D2K_MAP_NODES_VERSION_MAX] = { 0 };
  size_t             node_types_found = 0;
  D2KMapNodesVersion nodes_version;


  for (size_t i = 0; i < D2K_MAP_NODES_VERSION_MAX; i++) {
    D2KLump *lump = NULL;
    D2KMapNodeVersionHeaderInfo &info = &d2k_map_node_version_headers[i];

    if (!info->version_header_size) {
      continue;
    }

    switch (info->lump) {
      case D2K_MAP_LUMP_NODES:
      case D2K_MAP_LUMP_SSECTORS:
        if (!slice_equals_data_at(&map_loader->map_lumps[info->lump]->data,
                                  0,
                                  info->version_header,
                                  info->version_header_length,
                                  &node_type_found[i],
                                  status)) {
          return false;
        }
        break;
      case D2K_MAP_LUMP_GL_VERT:
      case D2K_MAP_LUMP_GL_SEGS:
        if (!slice_equals_data_at(&map_loader->gl_map_lumps[info->lump]->data,
                                  0,
                                  info->version_header,
                                  info->version_header_length,
                                  &node_type_found[i],
                                  status)) {
          return false;
        }
        break;
      case D2K_MAP_LUMP_ZNODES:
        break; /* [TODO] UDMF */
      default:
        return unsupported_map_node_version_info_lump_location(status);
    }
  }

  for (size_t i = 0; i < D2K_MAP_NODES_VERSION_MAX; i++) {
    if (node_type_found[i]) {
      nodes_version = i;
      node_types_found++;
    }
  }

  if (node_types_found == 0) {
    if (d2k_map_loader_has_gl_lumps(map_loader)) {
      map_loader->nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_1;
    }
    else {
      map_loader->nodes_version = D2K_MAP_NODES_VERSION_VANILLA;
    }
  }
  else if (node_types_found > 1) {
    if (node_types_found == 2 &&
        node_type_found[D2K_MAP_NODES_VERSION_GL_NODES_2] &&
        node_type_found[D2K_MAP_NODES_VERSION_GL_NODES_3]) {
      map_loader->nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_3;
    }
    else {
      return multiple_map_node_types_found(status);
    }
  }
  else {
    map_loader->nodes_version = nodes_version;
  }

  return status_ok(status);
}

bool d2k_map_loader_load_nodes(D2KMapLoader *map_loader, Status *status) {
  switch (map_loader->nodes_version) {
    case D2K_MAP_NODES_VERSION_VANILLA:
      return (
        load_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)      &&
        load_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_1:
      return (
        load_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)      &&
        load_gl_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_2:
      return (
        load_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)      &&
        load_gl_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_3:
      return (
        load_gl_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)         &&
        load_gl_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_4:
    case D2K_MAP_NODES_VERSION_GL_NODES_5:
      return (
        load_gl_subsectors(map_loader, status) &&
        load_gl_nodes(map_loader, status)      &&
        load_gl_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_DEEP_BSP_4:
      return (
        load_deep_bsp_subsectors(map_loader, status) &&
        load_deep_bsp_nodes(map_loader, status)      &&
        load_deep_bsp_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED:
      return (
        load_zdoom_subsectors(map_loader, status) &&
        load_zdoom_nodes(map_loader, status) &&
        load_zdoom_segs(map_loader, status)
      );
      break;
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED:
      return zdoom_extended_compressed_nodes_not_implemented(status);
      break;
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_GL:
      return zdoom_extended_gl_nodes_not_implemented(status);
      break;
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED_GL:
      return zdoom_extended_compressed_gl_nodes_not_implemented(status);
      break;
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_GL_UDMF:
      return zdoom_extended_gl_udmf_nodes_not_implemented(status);
      break;
    case D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED_GL_UDMF:
      return zdoom_extended_compressed_gl_udmf_nodes_not_implemented(status);
      break;
    default:
      break;
  }

  return unknown_nodes_version(status);
}

/* vi: set et ts=2 sw=2: */
