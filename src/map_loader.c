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
#include "d2k/map_blockmap.h"
#include "d2k/map_linedefs.h"
#include "d2k/map_loader.h"
#include "d2k/map_nodes.h"
#include "d2k/map_sectors.h"
#include "d2k/map_sidedefs.h"
#include "d2k/map_vertexes.h"
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
  D2K_MAP_MISSING_GL_VERT_LUMP,                        \
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

#define jumbled_lumps(status) status_error( \
  status,                                   \
  "d2k_map",                                \
  D2K_MAP_JUMBLED_LUMPS,                    \
  "map lumps are jumbled"                   \
)

#define multiple_nodes_versions(status) status_error( \
  status,                                             \
  "d2k_map",                                          \
  D2K_MAP_MULTIPLE_NODES_VERSIONS,                    \
  "map has multiple nodes versions"                   \
)

static bool load_vanilla_lump(D2KMapLoader *map_loader, size_t index,
                                                        Status *status) {
  return d2k_map_loader_get_vanilla_lump(
    map_loader,
    index,
    &map_loader->map_lumps[index],
    status
  );
}

static bool load_gl_lump_by_index(D2KMapLoader *map_loader, size_t index,
                                                            Status *status) {
  return d2k_map_loader_get_gl_lump(
    map_loader,
    index,
    &map_loader->map_lumps[index],
    status
  );
}

static bool load_vanilla_lumps(D2KMapLoader *map_loader, Status *status) {
  size_t i;

  for (i = D2K_VANILLA_MAP_LUMP_MAP + 1; i < D2K_VANILLA_MAP_LUMP_MAX; i++) {
    if (!load_vanilla_lump(map_loader, i, status)) {
      switch (i) {
        case D2K_VANILLA_MAP_LUMP_THINGS:
          return map_missing_things_lump(status);
        case D2K_VANILLA_MAP_LUMP_LINEDEFS:
          return map_missing_linedefs_lump(status);
        case D2K_VANILLA_MAP_LUMP_SIDEDEFS:
          return map_missing_sidedefs_lump(status);
        case D2K_VANILLA_MAP_LUMP_VERTEXES:
          if (!map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_GL_VERT]) {
            return map_missing_vertexes_lump(status);
          }
          break;
        case D2K_VANILLA_MAP_LUMP_SEGS:
          if (!map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_GL_SEGS]) {
            return map_missing_segs_lump(status);
          }
          break;
        case D2K_VANILLA_MAP_LUMP_SSECTORS:
          if (!map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_GL_SSECT]) {
            return map_missing_ssectors_lump(status);
          }
          break;
        case D2K_VANILLA_MAP_LUMP_NODES:
          if (!map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_GL_NODES]) {
            return map_missing_nodes_lump(status);
          }
          break;
        case D2K_VANILLA_MAP_LUMP_SECTORS:
          return map_missing_sectors_lump(status);
        case D2K_VANILLA_MAP_LUMP_REJECT:
          return map_missing_reject_lump(status);
        case D2K_VANILLA_MAP_LUMP_BLOCKMAP:
          return map_missing_blockmap_lump(status);
        case D2K_VANILLA_MAP_LUMP_BEHAVIOR:
          break;
        case D2K_VANILLA_MAP_LUMP_SCRIPTS:
          break;
        default:
          break;
      }
    }

    if (strcmp(map_loader->map_lumps[i]->name, d2k_map_lump_vanilla_names[i]) != 0) {
      return jumbled_lumps(status);
    }
  }

  return status_ok(status);
}

static bool load_gl_lumps(D2KMapLoader *map_loader, Status *status) {
  for (size_t i = D2K_GL_MAP_LUMP_MAP; i < D2K_GL_MAP_LUMP_MAX; i++) {
    if (!load_gl_lump_by_index(map_loader, i, status)) {
      return false;
    }
  }

  return status_ok(status);
}

static bool load_binary_map(D2KMapLoader *map_loader, Status *status) {
  snprintf(
    map_loader->map->gl_wad_name,
    sizeof(map_loader->map->gl_wad_name),
    "GL_%s", map_loader->map->wad_name
  );

  if (!d2k_lump_directory_lookup(
        map_loader->lump_directory,
        map_loader->map->gl_wad_name,
        &map_loader->gl_map_lumps[D2K_GL_MAP_LUMP_MAP],
        status)) {
    if (!status_match(status, "base", ERROR_NOT_FOUND)) {
      return false;
    }

    status_clear(status);
  }

  if (d2k_map_loader_has_gl_lumps(map_loader)) {
    if (!load_gl_lumps(map_loader, status)) {
      return false;
    }
  }

  return (
    load_vanilla_lumps(map_loader, status)                  &&
    d2k_map_loader_detect_nodes_version(map_loader, status) &&
    d2k_map_loader_load_vertexes(map_loader, status)        &&
    d2k_map_loader_load_sectors(map_loader, status)         &&
    d2k_map_loader_load_sidedefs(map_loader, status)        &&
    d2k_map_loader_load_linedefs(map_loader, status)        &&
    // d2k_map_loader_load_sidedefs2(map_loader, status)       &&
    // d2k_map_loader_load_linedefs2(map_loader, status)       &&
    d2k_map_loader_load_blockmap(map_loader, status)        &&
    d2k_map_loader_load_nodes(map_loader, status)
  );
}

static bool load_udmf_map(D2KMapLoader *map_loader, Status *status) {
  (void)map_loader;
  (void)status;

  return status_ok(status);
}

bool d2k_map_loader_load_map(D2KMapLoader *map_loader,
                             D2KMap *map,
                             D2KLumpDirectory *lump_directory,
                             const char *map_name,
                             Status *status) {
  bool first_lump_is_textmap = false;

  map_loader->map = map;
  map_loader->lump_directory = lump_directory;
  map_loader->nodes_version = D2K_MAP_NODES_VERSION_VANILLA;

  if (!d2k_lump_directory_lookup(
        lump_directory,
        map_name,
        &map_loader->map_lumps[D2K_VANILLA_MAP_LUMP_MAP],
        status)) {
    if (status_match(status, "base", ERROR_NOT_FOUND)) {
      return map_not_found(status);
    }

    return false;
  }

  strncpy(map->wad_name, map_name, sizeof(map->wad_name) - 1);
  map->wad_name[sizeof(map->wad_name) - 1] = '\0';

  if (!d2k_lump_directory_index_check_name(
        map_loader->lump_directory,
        map_loader->map_lumps[D2K_VANILLA_MAP_LUMP_MAP]->index + 1,
        "TEXTMAP",
        &first_lump_is_textmap,
        status)) {
    return false;
  }

  if (first_lump_is_textmap) {
    return load_udmf_map(map_loader, status);
  }

  return load_binary_map(map_loader, status);
}

/* vi: set et ts=2 sw=2: */
