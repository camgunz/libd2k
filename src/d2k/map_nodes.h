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

#ifndef D2K_MAP_NODES_H__
#define D2K_MAP_NODES_H__

#include "d2k/fixed_math.h"

#define D2K_MAP_NODE_FLAGS_SUBSECTOR 0x80000000

enum {
  D2K_MAP_NODES_MALFORMED_LUMP = 1,
  D2K_MAP_NODES_MULTIPLE_TYPES_FOUND,
  D2K_MAP_NODES_UNKNOWN_NODES_VERSION,
  D2K_MAP_NODES_INVALID_CHILD_INDEX,
  D2K_MAP_NODES_UNSUPPORTED_MAP_NODE_VERSION_INFO_LUMP_LOCATION,
  D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_NODES_NOT_IMPLEMENTED,
  D2K_MAP_NODES_ZDOOM_EXTENDED_GL_NODES_NOT_IMPLEMENTED,
  D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_GL_NODES_NOT_IMPLEMENTED,
  D2K_MAP_NODES_ZDOOM_EXTENDED_GL_UDMF_NODES_NOT_IMPLEMENTED,
  D2K_MAP_NODES_ZDOOM_EXTENDED_COMPRESSED_GL_UDMF_NODES_NOT_IMPLEMENTED,
};

typedef enum {
  D2K_MAP_NODES_VERSION_VANILLA,
  D2K_MAP_NODES_VERSION_GL_NODES_1,
  D2K_MAP_NODES_VERSION_GL_NODES_2,
  D2K_MAP_NODES_VERSION_GL_NODES_3,
  D2K_MAP_NODES_VERSION_GL_NODES_4,
  D2K_MAP_NODES_VERSION_GL_NODES_5,
  D2K_MAP_NODES_VERSION_DEEP_BSP_4,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_GL,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED_GL,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_GL_UDMF,
  D2K_MAP_NODES_VERSION_ZDOOM_EXTENDED_COMPRESSED_GL_UDMF,
  D2K_MAP_NODES_VERSION_MAX,
} D2KMapNodesVersion;

typedef struct D2KMapNodeStruct {
  D2KFixedPoint x;
  D2KFixedPoint y;
  D2KFixedPoint dx;
  D2KFixedPoint dy;
  D2KFixedPoint bbox[2][4];
  int           children[2];
} D2KMapNode;

bool d2k_map_loader_detect_nodes_version(struct D2KMapLoaderStruct *map_loader,
                                         Status *status);
bool d2k_map_loader_load_nodes(struct D2KMapLoaderStruct *map_loader,
                               Status *status);

#endif

/* vi: set et ts=2 sw=2: */
