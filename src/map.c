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

#define multiple_nodes_versions(status) status_error( \
  status,                                             \
  "d2k_map",                                          \
  D2K_MAP_MULTIPLE_NODES_VERSIONS,                    \
  "map has multiple nodes versions"                   \
)

#define truncated_nodes_lump(status) status_error( \
  status, \
  "d2k_map", \
  D2K_MAP_TRUNCATED_NODES_LUMP, \
  "map has truncated NODES lump" \
)

#define truncated_ssectors_lump(status) status_error( \
  status, \
  "d2k_map", \
  D2K_MAP_TRUNCATED_SSECTORS_LUMP, \
  "map has truncated SSECTORS lump" \
)

typedef enum {
  D2K_MAP_NODES_VERSION_VANILLA = 1,                  /* No marker */
  D2K_MAP_NODES_VERSION_DEEP_BSP_4,                   /* xNd40000, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM,                        /* XNOD, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM_COMPRESSED,             /* ZNOD, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM_GL,                     /* XGLN, SSECTORS */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_COMPRESSED,          /* ZGLN, SSECTORS */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_EXTENDED,            /* XGL2, ZNODES (UDMF) */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_EXTENDED_COMPRESSED, /* ZGL2, ZNODES (UDMF) */
  D2K_MAP_NODES_VERSION_GL_NODES_2,                   /* gNd2, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_3,                   /* gNd3, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_4,                   /* gNd4, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_5,                   /* gNd5, GL_NODES */
} D2KMapNodesVersion;

#define D2K_MAP_LUMP_OFFSET_MIN D2K_MAP_LUMP_OFFSET_THINGS
#define D2K_MAP_LUMP_OFFSET_MAX D2K_MAP_LUMP_OFFSET_BEHAVIOR

typedef enum {
  D2K_MAP_LUMP_OFFSET_THINGS = 1,
  D2K_MAP_LUMP_OFFSET_LINEDEFS,
  D2K_MAP_LUMP_OFFSET_SIDEDEFS,
  D2K_MAP_LUMP_OFFSET_VERTEXES,
  D2K_MAP_LUMP_OFFSET_SEGS,
  D2K_MAP_LUMP_OFFSET_SSECTORS,
  D2K_MAP_LUMP_OFFSET_NODES,
  D2K_MAP_LUMP_OFFSET_SECTORS,
  D2K_MAP_LUMP_OFFSET_REJECT,
  D2K_MAP_LUMP_OFFSET_BLOCKMAP,
  D2K_MAP_LUMP_OFFSET_BEHAVIOR,
} D2KMapLumpOffset;

typedef enum {
  D2K_MAP_GL_LUMP_OFFSET_VERTS = 1,
  D2K_MAP_GL_LUMP_OFFSET_SEGS,
  D2K_MAP_GL_LUMP_OFFSET_SSECT,
  D2K_MAP_GL_LUMP_OFFSET_NODES,
} D2KMapGLLumpOffset;

static bool get_nodes_version_from_nodes(D2KLumpDirectory *lump_directory,
                                         size_t index,
                                         D2KMapNodesVersion *nodes_version,
                                         Status *status) {
  D2KLump *nodes_lump = NULL;
  bool equal = false;

  if (!d2k_lump_directory_index(lump_directory,
                                index + D2K_MAP_LUMP_OFFSET_NODES,
                                &nodes_lump,
                                status)) {
    return map_missing_nodes_lump(status);
  }

  if (!slice_equals_data_at(&nodes_lump->data, 0, "xNd4\0\0\0\0", 8, &equal,
                                                                     status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_DEEP_BSP_4;
    return status_ok(status);
  }

  if (!slice_equals_data_at(&nodes_lump->data, 0, "XNOD", 4, &equal, status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_ZDOOM;
    return status_ok(status);
  }

  if (!slice_equals_data_at(&nodes_lump->data, 0, "ZNOD", 4, &equal, status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_ZDOOM_COMPRESSED;
    return status_ok(status);
  }

  return not_found(status);
}

static bool get_nodes_version_from_ssectors(D2KLumpDirectory *lump_directory,
                                            size_t index,
                                            D2KMapNodesVersion *nodes_version,
                                            Status *status) {
  D2KLump *ssectors_lump = NULL;
  bool equal = false;

  if (!d2k_lump_directory_index(lump_directory,
                                index + D2K_MAP_LUMP_OFFSET_SSECTORS,
                                &ssectors_lump,
                                status)) {
    return map_missing_ssectors_lump(status);
  }

  if (!slice_equals_data_at(&ssectors_lump->data, 0, "XGLN", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_ssectors_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_ZDOOM_GL;
    return status_ok(status);
  }

  if (!slice_equals_data_at(&ssectors_lump->data, 0, "ZGLN", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_ssectors_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_ZDOOM_GL_COMPRESSED;
    return status_ok(status);
  }

  return not_found(status);
}

static bool get_nodes_version_from_gl_nodes(D2KLumpDirectory *lump_directory,
                                            size_t index,
                                            D2KMapNodesVersion *nodes_version,
                                            Status *status) {
  D2KLump *gl_nodes_lump = NULL;
  bool equal = false;

  if (!d2k_lump_directory_index(lump_directory,
                                index + D2K_MAP_GL_LUMP_OFFSET_NODES,
                                &gl_nodes_lump,
                                status)) {
    return map_missing_gl_nodes_lump(status);
  }

  if (!slice_equals_data_at(&gl_nodes_lump->data, 0, "gNd2", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    D2KLump *gl_segs_lump = NULL;

    if (!d2k_lump_directory_index(lump_directory,
                                  index + D2K_MAP_GL_LUMP_OFFSET_SEGS,
                                  &gl_segs_lump,
                                  status)) {
      return map_missing_gl_segs_lump(status);
    }

    if (!slice_equals_data_at(&gl_segs_lump->data, 0, "gNd3", 4, &equal,
                                                                 status)) {
      if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
        return truncated_nodes_lump(status);
      }

      return false;
    }

    if (equal) {
      *nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_2;
    }
    else {
      *nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_3;
    }

    return status_ok(status);
  }

  if (!slice_equals_data_at(&gl_nodes_lump->data, 0, "gNd3", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_3;
    return status_ok(status);
  }

  if (!slice_equals_data_at(&gl_nodes_lump->data, 0, "gNd4", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_4;
    return status_ok(status);
  }

  if (!slice_equals_data_at(&gl_nodes_lump->data, 0, "gNd5", 4, &equal,
                                                                status)) {
    if (status_match(status, "base", ERROR_OUT_OF_BOUNDS)) {
      return truncated_nodes_lump(status);
    }

    return false;
  }

  if (equal) {
    *nodes_version = D2K_MAP_NODES_VERSION_GL_NODES_5;
    return status_ok(status);
  }

  return not_found(status);
}

static bool get_nodes_version(D2KLumpDirectory *lump_directory,
                              const char *map_name,
                              const char *gl_map_name,
                              D2KMapNodesVersion *nodes_version,
                              Status *status) {
  D2KLump *map_marker_lump = NULL;
  D2KLump *gl_map_marker_lump = NULL;
  size_t nodes_version_count = 0;
  D2KMapNodesVersion nodes_version_nodes = D2K_MAP_NODES_VERSION_VANILLA;
  D2KMapNodesVersion nodes_version_ssectors = D2K_MAP_NODES_VERSION_VANILLA;
  D2KMapNodesVersion nodes_version_gl_nodes = D2K_MAP_NODES_VERSION_VANILLA;

  if (!d2k_lump_directory_lookup(lump_directory, map_name, &map_marker_lump,
                                                           status)) {
    if (status_match(status, "base", ERROR_NOT_FOUND)) {
      return map_not_found(status);
    }

    return false;
  }

  if (!get_nodes_version_from_nodes(lump_directory, map_marker_lump->index,
                                                    &nodes_version_nodes,
                                                    status)) {
    if (!status_match(status, "base", ERROR_NOT_FOUND)) {
      return false;
    }
  }

  if (!get_nodes_version_from_ssectors(lump_directory, map_marker_lump->index,
                                                       &nodes_version_ssectors,
                                                       status)) {
    if (!status_match(status, "base", ERROR_NOT_FOUND)) {
      return false;
    }
  }

  if (gl_map_name) {
    if (!d2k_lump_directory_lookup(lump_directory, gl_map_name,
                                                   &gl_map_marker_lump,
                                                   status)) {
      if (status_match(status, "base", ERROR_NOT_FOUND)) {
        return map_not_found(status);
      }

      return false;
    }

    if (!get_nodes_version_from_gl_nodes(lump_directory,
                                         gl_map_marker_lump->index,
                                         &nodes_version_gl_nodes,
                                         status)) {
      if (!status_match(status, "base", ERROR_NOT_FOUND)) {
        return false;
      }
    }
  }

  if (nodes_version_nodes != D2K_MAP_NODES_VERSION_VANILLA) {
    nodes_version_count++;
  }

  if (nodes_version_ssectors != D2K_MAP_NODES_VERSION_VANILLA) {
    nodes_version_count++;
  }

  if (nodes_version_gl_nodes != D2K_MAP_NODES_VERSION_VANILLA) {
    nodes_version_count++;
  }

  if (nodes_version_count > 1) {
    return multiple_nodes_versions(status);
  }

  if (nodes_version_nodes != D2K_MAP_NODES_VERSION_VANILLA) {
    *nodes_version = nodes_version_nodes;
  }
  else if (nodes_version_ssectors != D2K_MAP_NODES_VERSION_VANILLA) {
    *nodes_version = nodes_version_ssectors;
  }
  else if (nodes_version_gl_nodes != D2K_MAP_NODES_VERSION_VANILLA) {
    *nodes_version = nodes_version_gl_nodes;
  }
  else {
    *nodes_version = D2K_MAP_NODES_VERSION_VANILLA;
  }

  return status_ok(status);
}

static bool map_load_vertexes2(D2KMap *map, D2KLump *vertexes_lump,
                                            D2KLump *gl_vert_lump,
                                            Status *status) {
  (void)map;
  (void)vertexes_lump;
  (void)gl_vert_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_vertexes(D2KMap *map, D2KLump *vertexes_lump,
                                           Status *status) {
  (void)map;
  (void)vertexes_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_sectors(D2KMap *map, D2KLump *sectors_lump,
                                          Status *status) {
  (void)map;
  (void)sectors_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_sidedefs(D2KMap *map, D2KLump *sidedefs_lump,
                                          Status *status) {
  (void)map;
  (void)sidedefs_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_linedefs(D2KMap *map, D2KLump *linedefs_lump,
                                           Status *status) {
  (void)map;
  (void)linedefs_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_sidedefs2(D2KMap *map, D2KLump *sidedefs_lump,
                                            Status *status) {
  (void)map;
  (void)sidedefs_lump;
  (void)status;
  return status_ok(status);
}

static bool map_load_linedefs2(D2KMap *map, D2KLump *linedefs_lump,
                                            Status *status) {
  (void)map;
  (void)linedefs_lump;
  (void)status;
  return status_ok(status);
}

bool d2k_map_init(D2KMap *map, D2KLumpDirectory *lump_directory,
                               const char *map_name,
                               const char *gl_map_name,
                               Status *status) {
  D2KLump *map_marker_lump = NULL;
  D2KLump *gl_map_marker_lump = NULL;
  D2KLump *things_lump = NULL;
  D2KLump *linedefs_lump = NULL;
  D2KLump *sidedefs_lump = NULL;
  D2KLump *vertexes_lump = NULL;
  D2KLump *segs_lump = NULL;
  D2KLump *ssectors_lump = NULL;
  D2KLump *nodes_lump = NULL;
  D2KLump *sectors_lump = NULL;
  D2KLump *reject_lump = NULL;
  D2KLump *blockmap_lump = NULL;
  D2KLump *gl_vert_lump = NULL;
  D2KLump *gl_segs_lump = NULL;
  D2KLump *gl_ssect_lump = NULL;
  D2KLump *gl_nodes_lump = NULL;
  D2KMapNodesVersion nodes_version = D2K_MAP_NODES_VERSION_VANILLA;
  size_t map_index = 0;
  size_t gl_map_index = 0;

  if (!d2k_lump_directory_lookup(lump_directory, map_name, &map_marker_lump,
                                                           status)) {
    if (status_match(status, "base", ERROR_NOT_FOUND)) {
      return map_not_found(status);
    }

    return false;
  }

  map_index = map_marker_lump->index;

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_THINGS,
                                &things_lump,
                                status)) {
    return map_missing_things_lump(status);
  }

  if (strcmp(things_lump->name, "THINGS") != 0) {
    return map_missing_things_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_LINEDEFS,
                                &linedefs_lump,
                                status)) {
    return map_missing_linedefs_lump(status);
  }

  if (strcmp(linedefs_lump->name, "LINEDEFS") != 0) {
    return map_missing_linedefs_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_SIDEDEFS,
                                &sidedefs_lump,
                                status)) {
    return map_missing_sidedefs_lump(status);
  }

  if (strcmp(sidedefs_lump->name, "SIDEDEFS") != 0) {
    return map_missing_sidedefs_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_VERTEXES,
                                &vertexes_lump,
                                status)) {
    return map_missing_vertexes_lump(status);
  }

  if (strcmp(vertexes_lump->name, "VERTEXES") != 0) {
    return map_missing_vertexes_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_SEGS,
                                &segs_lump,
                                status)) {
    return map_missing_segs_lump(status);
  }

  if (strcmp(segs_lump->name, "SEGS") != 0) {
    return map_missing_segs_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_SSECTORS,
                                &ssectors_lump,
                                status)) {
    return map_missing_ssectors_lump(status);
  }

  if (strcmp(ssectors_lump->name, "SSECTORS") != 0) {
    return map_missing_ssectors_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_NODES,
                                &nodes_lump,
                                status)) {
    return map_missing_nodes_lump(status);
  }

  if (strcmp(nodes_lump->name, "NODES") != 0) {
    return map_missing_nodes_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_SECTORS,
                                &sectors_lump,
                                status)) {
    return map_missing_sectors_lump(status);
  }

  if (strcmp(sectors_lump->name, "SECTORS") != 0) {
    return map_missing_sectors_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_REJECT,
                                &reject_lump,
                                status)) {
    return map_missing_reject_lump(status);
  }

  if (strcmp(reject_lump->name, "REJECT") != 0) {
    return map_missing_reject_lump(status);
  }

  if (!d2k_lump_directory_index(lump_directory,
                                map_index + D2K_MAP_LUMP_OFFSET_BLOCKMAP,
                                &blockmap_lump,
                                status)) {
    return map_missing_blockmap_lump(status);
  }

  if (strcmp(blockmap_lump->name, "BLOCKMAP") != 0) {
    return map_missing_blockmap_lump(status);
  }

  if (gl_map_name) {
    if (!d2k_lump_directory_lookup(lump_directory, gl_map_name,
                                                   &gl_map_marker_lump,
                                                   status)) {
      if (status_match(status, "base", ERROR_NOT_FOUND)) {
        return map_not_found(status);
      }

      return false;
    }

    if (!d2k_lump_directory_index(lump_directory,
                                  gl_map_index + D2K_MAP_GL_LUMP_OFFSET_VERTS,
                                  &gl_vert_lump,
                                  status)) {
      return map_missing_gl_vert_lump(status);
    }

    if (strcmp(gl_vert_lump->name, "GL_VERTS") != 0) {
      return map_missing_gl_vert_lump(status);
    }

    if (!d2k_lump_directory_index(lump_directory,
                                  gl_map_index + D2K_MAP_GL_LUMP_OFFSET_SEGS,
                                  &gl_segs_lump,
                                  status)) {
      return map_missing_gl_segs_lump(status);
    }

    if (strcmp(gl_segs_lump->name, "GL_SEGS") != 0) {
      return map_missing_gl_segs_lump(status);
    }

    if (!d2k_lump_directory_index(lump_directory,
                                  gl_map_index + D2K_MAP_GL_LUMP_OFFSET_SSECT,
                                  &gl_ssect_lump,
                                  status)) {
      return map_missing_gl_ssect_lump(status);
    }

    if (strcmp(gl_ssect_lump->name, "GL_SSECT") != 0) {
      return map_missing_gl_ssect_lump(status);
    }

    if (!d2k_lump_directory_index(lump_directory,
                                  gl_map_index + D2K_MAP_GL_LUMP_OFFSET_NODES,
                                  &gl_nodes_lump,
                                  status)) {
      return map_missing_gl_nodes_lump(status);
    }

    if (strcmp(gl_nodes_lump->name, "GL_NODES") != 0) {
      return map_missing_gl_nodes_lump(status);
    }
  }

  if (!get_nodes_version(lump_directory, map_name, gl_map_name, &nodes_version,
                                                                status)) {
    return false;
  }

  array_init(&map->vertexes, sizeof(D2KFixedVertex));
  array_init(&map->segs, sizeof(D2KSeg));
  array_init(&map->sectors, sizeof(D2KSector));
  array_init(&map->subsectors, sizeof(D2KSubSector));
  array_init(&map->nodes, sizeof(D2KMapNode));
  array_init(&map->lines, sizeof(D2KLinedef));
  array_init(&map->sides, sizeof(D2KSidedef));
  array_init(&map->sslines, sizeof(D2KSegLine));

  if ((nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_2) ||
      (nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_3) ||
      (nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_4) ||
      (nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_5)) {
    if (!map_load_vertexes2(map, vertexes_lump, gl_vert_lump, status)) {
      return false;
    }
  }
  else {
    if (!map_load_vertexes(map, vertexes_lump, status)) {
      return false;
    }
  }

  if (!map_load_sectors(map, sectors_lump, status)) {
    return false;
  }

  if (!map_load_sidedefs(map, sidedefs_lump, status)) {
    return false;
  }

  if (!map_load_linedefs(map, linedefs_lump, status)) {
    return false;
  }

  if (!map_load_sidedefs2(map, sidedefs_lump, status)) {
    return false;
  }

  if (!map_load_linedefs2(map, linedefs_lump, status)) {
    return false;
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
