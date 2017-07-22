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
#include "d2k/map_segs.h"
#include "d2k/map_subsectors.h"
#include "d2k/wad.h"

#define malformed_nodes_lump(status) status_error( \
  status,                                          \
  "d2k_map_nodes",                                 \
  D2K_MAP_NODES_MALFORMED_LUMP,                    \
  "malformed NODES lump"                           \
)

#define multiple_map_node_types_found(status) status_error( \
  status,                                                   \
  "d2k_map_nodes",                                          \
  D2K_MAP_NODES_MULTIPLE_TYPES_FOUND,                       \
  "multiple nodes types detected"                           \
)

#define unknown_nodes_version(status) status_error( \
  status,                                           \
  "d2k_map_nodes",                                  \
  D2K_MAP_NODES_UNKNOWN_NODES_VERSION,              \
  "unknown nodes version"                           \
)

#define invalid_child_index(status) status_error( \
  status,                                         \
  "d2k_map_nodes",                                \
  D2K_MAP_NODES_INVALID_CHILD_INDEX,              \
  "invalid child index"                           \
)

#define unsupported_map_node_version_info_lump_location(status) status_error( \
  status,                                                                     \
  "d2k_map_nodes",                                                            \
  D2K_MAP_NODES_UNSUPPORTED_MAP_NODE_VERSION_INFO_LUMP_LOCATION,              \
  "unsupported map node version info lump location"                           \
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

#define VANILLA_NODE_SIZE 28

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
  {D2K_MAP_LUMP_GL_VERT,  "gNd2",         4},
  {D2K_MAP_LUMP_GL_SEGS,  "gNd3",         4},
  {D2K_MAP_LUMP_GL_VERT,  "gNd4",         4},
  {D2K_MAP_LUMP_GL_VERT,  "gNd5",         4},
};

static inline bool lump_starts_with(D2KLump *lump,
                                    const char *data,
                                    size_t len,
                                    bool *starts_with,
                                    Status *status) {
  return slice_equals_data_at(&lump->data, 0, data, len, starts_with, status);
}

static bool load_nodes(D2KMapLoader *map_loader, Status *status) {
  D2KLump *nodes_lump = map_loader->map_lumps[D2K_MAP_LUMP_NODES];
  size_t nodes_count = nodes_lump->data.len / VANILLA_NODE_SIZE;

  if ((nodes_lump->data.len % VANILLA_NODE_SIZE) != 0) {
    return malformed_nodes_lump(status);
  }

  if (!array_ensure_capacity(&map_loader->map->nodes, nodes_count, status)) {
    return false;
  }

  for (size_t i = 0; i < nodes_count; i++) {
    D2KMapNode *node = array_append_fast(&map_loader->map->nodes);
    char node_data[VANILLA_NODE_SIZE];
    uint16_t children[2];

    slice_read_fast(&nodes_lump->data, i * VANILLA_NODE_SIZE,
                                       VANILLA_NODE_SIZE,
                                       (void *)node_data);

    node->x  = LUMP_DATA_SHORT_TO_FIXED(node_data, 0);
    node->y  = LUMP_DATA_SHORT_TO_FIXED(node_data, 2);
    node->dx = LUMP_DATA_SHORT_TO_FIXED(node_data, 4);
    node->dy = LUMP_DATA_SHORT_TO_FIXED(node_data, 6);
    for (size_t j = 0; j < 2; j++) {
      for (size_t k = 0; k < 4; k++) {
        node->bbox[j][k] = LUMP_DATA_SHORT_TO_FIXED(node_data, (j * 4) + k);
      }
    }
    children[0] = LUMP_DATA_SHORT_TO_USHORT(node_data, 24);
    children[1] = LUMP_DATA_SHORT_TO_USHORT(node_data, 26);

    for (size_t j = 0; j < 2; j++) {
      node->children[j] = children[j];

      if (node->children[j] == 0xFFFF) {
        node->children[j] = -1;
      }
      else if ((node->children[j] & 0x8000) == 0x8000) {
        int new_child_value = node->children[j] &= ~0x8000;

        node->children[j] = new_child_value;

        if ((size_t)node->children[j] >= map_loader->map->nodes.len) {
          /* PrBoom+ sets this to 0 */
          return invalid_child_index(status);
        }

        node->children[j] |= D2K_MAP_NODE_FLAGS_SUBSECTOR;
      }
    }
  }

  return status_ok(status);
}

static bool load_gl_subsectors(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_gl_nodes(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_gl_segs(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_deep_bsp_subsectors(D2KMapLoader *map_loader,
                                     Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_deep_bsp_nodes(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_deep_bsp_segs(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
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
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_zdoom_nodes(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

static bool load_zdoom_segs(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;
  return status_ok(status);
}

bool d2k_map_loader_detect_nodes_version(D2KMapLoader *map_loader,
                                         Status *status) {
  bool               node_type_found[D2K_MAP_NODES_VERSION_MAX] = { 0 };
  size_t             node_types_found = 0;
  D2KMapNodesVersion nodes_version;


  for (size_t i = 0; i < D2K_MAP_NODES_VERSION_MAX; i++) {
    D2KMapNodeVersionHeaderInfo *info = &d2k_map_node_version_headers[i];

    if (!info->version_header_size) {
      continue;
    }

    switch (info->lump) {
      case D2K_MAP_LUMP_NODES:
      case D2K_MAP_LUMP_SSECTORS:
        if (!slice_equals_data_at(&map_loader->map_lumps[info->lump]->data,
                                  0,
                                  info->version_header,
                                  info->version_header_size,
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
                                  info->version_header_size,
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
        d2k_map_loader_load_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)      &&
        d2k_map_loader_load_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_1:
      return (
        d2k_map_loader_load_subsectors(map_loader, status) &&
        load_nodes(map_loader, status)      &&
        load_gl_segs(map_loader, status)
      );
    case D2K_MAP_NODES_VERSION_GL_NODES_2:
      return (
        d2k_map_loader_load_subsectors(map_loader, status) &&
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
