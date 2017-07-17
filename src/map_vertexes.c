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

#define malformed_vertexes_lump(status) status_error( \
  status,                                             \
  "d2k_map_vertexes",                                 \
  D2K_MAP_VERTEXES_MALFORMED_LUMP,                    \
  "malformed VERTEXES lump"                           \
)

#define malformed_gl_vert_lump(status) status_error( \
  status,                                            \
  "d2k_map_vertexes",                                \
  D2K_MAP_VERTEXES_MALFORMED_GL_VERT_LUMP,           \
  "malformed GL_VERT lump"                           \
)

#define VANILLA_VERTEX_SIZE 4
#define GL_VERT_HEADER_SIZE 4
#define GL_VERT_VERTEX_SIZE (sizeof(D2KFixedPoint) * 2)

bool d2k_map_loader_load_vertexes(D2KMapLoader *map_loader, Status *status) {
  D2KLump *vertexes_lump = map_loader->map_lumps[D2K_MAP_LUMP_VERTEXES];
  size_t vertex_count = vertexes_lump->data.len / VANILLA_VERTEX_SIZE;

  if ((vertexes_lump->data.len % VANILLA_VERTEX_SIZE) != 0) {
    return malformed_vertexes_lump(status);
  }

  if (!array_ensure_capacity(&map->vertexes, vertex_count, status)) {
    return false;
  }

  for (size_t i = 0; i < vertex_count; i++) {
    D2KFixedVertex *v = array_append_fast(&map->vertexes);
    char vertex_data[VANILLA_VERTEX_SIZE];
    int16_t x;
    int16_t y;

    slice_read_fast(&vertexes_lump->data, i * VANILLA_VERTEX_SIZE,
                                          VANILLA_VERTEX_SIZE,
                                          (void *)vertex_data);

    v->x = LUMP_DATA_SHORT_TO_FIXED(vertex_data, 0);
    v->y = LUMP_DATA_SHORT_TO_FIXED(vertex_data, 2);
  }

  if (d2k_map_loader_has_gl_lumps(map_loader)) {
    if (map_loader->nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_2 ||
        map_loader->nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_3 ||
        map_loader->nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_4 ||
        map_loader->nodes_version == D2K_MAP_NODES_VERSION_GL_NODES_5) {
      D2KLump *gl_vert_lump = map_loader->map_lumps[D2K_MAP_LUMP_VERTEXES];

      vertex_count = (gl_vert_lump->data.len - GL_VERT_HEADER_SIZE) /
                     GL_VERT_VERTEX_SIZE;

      if (((gl_vert_lump->data.len - GL_VERT_HEADER_SIZE) %
           GL_VERT_VERTEX_SIZE) != 0) {
        return malformed_gl_vert_lump(status);
      }

      if (!array_ensure_capacity(&map->vertexes,
                                 map->vertexes.len + vertex_count,
                                 status)) {
        return false;
      }

      for (size_t i = 0; i < vertex_count; i++) {
        D2KFixedVertex *v = array_append_fast(&map->vertexes);
        char vertex_data[GL_VERT_VERTEX_SIZE];

        slice_read_fast(&gl_vert_lump->data, i * GL_VERT_VERTEX_SIZE,
                                             GL_VERT_VERTEX_SIZE,
                                             (void *)vertex_data);

        v->x = LUMP_DATA_INT_TO_FIXED(vertex_data, 0);
        v->y = LUMP_DATA_INT_TO_FIXED(vertex_data, 4);
      }
    }
  }

  return status_ok(status);
}

/* vi: set et ts=2 sw=2: */
