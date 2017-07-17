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

#ifndef D2K_MAP_H__
#define D2K_MAP_H__

#include "d2k/fixed_math.h"
#include "d2k/angle.h"
#include "d2k/sound_origin.h"

struct D2KLumpStruct;
struct D2KMapObjectStruct;
struct D2KLumpDirectoryStruct;

enum {
  D2K_MAP_NOT_FOUND = 1,
  D2K_MAP_MISSING_THINGS_LUMP,
  D2K_MAP_MISSING_LINEDEFS_LUMP,
  D2K_MAP_MISSING_SIDEDEFS_LUMP,
  D2K_MAP_MISSING_VERTEXES_LUMP,
  D2K_MAP_MISSING_SEGS_LUMP,
  D2K_MAP_MISSING_SSECTORS_LUMP,
  D2K_MAP_MISSING_NODES_LUMP,
  D2K_MAP_MISSING_SECTORS_LUMP,
  D2K_MAP_MISSING_REJECT_LUMP,
  D2K_MAP_MISSING_BLOCKMAP_LUMP,
  D2K_MAP_MISSING_GL_VERT_LUMP,
  D2K_MAP_MISSING_GL_SEGS_LUMP,
  D2K_MAP_MISSING_GL_SSECT_LUMP,
  D2K_MAP_MISSING_GL_NODES_LUMP,
  D2K_MAP_TRUNCATED_SSECTORS_LUMP,
};

typedef enum {
  D2K_MAP_LUMP_SECTION_VANILLA,
  D2K_MAP_LUMP_SECTION_GL,
  D2K_MAP_LUMP_SECTION_UDMF,
} D2KMapLumpSection;

typedef enum {
  D2K_MAP_LUMP_VANILLA_MAP,
  D2K_MAP_LUMP_VANILLA_THINGS,
  D2K_MAP_LUMP_VANILLA_LINEDEFS,
  D2K_MAP_LUMP_VANILLA_SIDEDEFS,
  D2K_MAP_LUMP_VANILLA_VERTEXES,
  D2K_MAP_LUMP_VANILLA_SEGS,
  D2K_MAP_LUMP_VANILLA_SSECTORS,
  D2K_MAP_LUMP_VANILLA_NODES,
  D2K_MAP_LUMP_VANILLA_SECTORS,
  D2K_MAP_LUMP_VANILLA_REJECT,
  D2K_MAP_LUMP_VANILLA_BLOCKMAP,
  D2K_MAP_LUMP_VANILLA_BEHAVIOR,
  D2K_MAP_LUMP_VANILLA_SCRIPTS,
  D2K_MAP_LUMP_VANILLA_MAX,
} D2KMapLumpVanilla;

const char d2k_map_lump_vanilla_names[D2K_MAP_LUMP_VANILLA_MAX] = {
  "",
  "THINGS",
  "LINEDEFS",
  "SIDEDEFS",
  "VERTEXES",
  "SEGS",
  "SSECTORS",
  "NODES",
  "SECTORS",
  "REJECT",
  "BLOCKMAP",
  "BEHAVIOR",
  "SCRIPTS",
}

typedef enum {
  D2K_MAP_LUMP_GL_MAP,
  D2K_MAP_LUMP_GL_VERT,
  D2K_MAP_LUMP_GL_SEGS,
  D2K_MAP_LUMP_GL_SSECT,
  D2K_MAP_LUMP_GL_NODES,
  D2K_MAP_LUMP_GL_PVS,
  D2K_MAP_LUMP_GL_MAX,
} D2KMapLumpGL;

const char d2k_map_lump_gl_names[D2K_MAP_LUMP_GL_MAX] = {
  "",
  "GL_VERT",
  "GL_SEGS",
  "GL_SSECT",
  "GL_NODES",
  "GL_PVS",
}

typedef enum {
  D2K_MAP_LUMP_UDMF_TEXTMAP = 1,
  D2K_MAP_LUMP_UDMF_MAX,
} D2KMapLumpUDMF;

const char d2k_map_lump_udmf_names[D2K_MAP_LUMP_UDMF_MAX] = {
  "TEXTMAP",
};

typedef enum {
  D2K_MAP_LUMP_NONE,
  D2K_MAP_LUMP_THINGS,
  D2K_MAP_LUMP_LINEDEFS,
  D2K_MAP_LUMP_SIDEDEFS,
  D2K_MAP_LUMP_VERTEXES,
  D2K_MAP_LUMP_SEGS,
  D2K_MAP_LUMP_SSECTORS,
  D2K_MAP_LUMP_NODES,
  D2K_MAP_LUMP_SECTORS,
  D2K_MAP_LUMP_REJECT,
  D2K_MAP_LUMP_BLOCKMAP,
  D2K_MAP_LUMP_BEHAVIOR,
  D2K_MAP_LUMP_SCRIPTS,
  D2K_MAP_LUMP_GL_VERT,
  D2K_MAP_LUMP_GL_SEGS,
  D2K_MAP_LUMP_GL_SSECT,
  D2K_MAP_LUMP_GL_NODES,
  D2K_MAP_LUMP_GL_PVS,
  D2K_MAP_LUMP_DIALOGUE,
  D2K_MAP_LUMP_ZNODES,
  D2K_MAP_LUMP_TEXTMAP,
  D2K_MAP_LUMP_ENDMAP,
} D2KMapLump;

typedef struct D2KSidedefStruct {
  D2KFixedPoint  texture_offset;
  D2KFixedPoint  row_offset;
  size_t         top_texture;
  size_t         bottom_texture;
  size_t         mid_texture;
  D2KSector     *sector;
  int            special;
  int            wall_panning_interpolation;
  int            skybox_index;
} D2KSidedef;

typedef struct D2KMapSectorNodeStruct {
  D2KSector                     *sector;
  struct D2KMapObjectStruct     *mobj;
  struct D2KMapSectorNodeStruct *previous_thing_map_sector_node;
  struct D2KMapSectorNodeStruct *next_thing_map_sector_node;
  struct D2KMapSectorNodeStruct *previous_sector_map_sector_node;
  struct D2KMapSectorNodeStruct *next_sector_map_sector_node;
  bool                           visited;
} D2KMapSectorNode;

typedef struct D2KSubsectorStruct {
  D2KSector *sector;
  size_t     seg_count;
  size_t     first_seg;
} D2KSubsector;

typedef struct D2KSegStruct {
  D2KFixedVertex *v1;
  D2KFixedVertex *v2;
  D2KFixedPoint   offset;
  D2KAngle        angle;
  D2KAngle        pangle;
  int64_t         length;
  D2KSidedef     *sidedef;
  D2KLinedef     *linedef;
  bool            mini_seg;
  D2KSector      *front_sector;
  D2KSector      *back_sector;
} D2KSeg;

typedef struct D2KSegLineStruct {
  D2KSeg        *seg;
  D2KLinedef    *linedef;
  D2KFixedPoint  x1;
  D2KFixedPoint  y1;
  D2KFixedPoint  x2;
  D2KFixedPoint  y2;
  D2KFixedPoint  bbox[4];
} D2KSegLine;

typedef struct D2KMapStruct {
  char        wad_name[6];
  char        gl_wad_name[9];
  Array       vertexes;
  Array       segs;
  Array       sectors;
  Array       subsectors;
  Array       nodes;
  Array       linedefs;
  Array       sidedefs;
  Array       sslines;
  D2KBlockmap blockmap;
} D2KMap;

void d2k_map_init(D2KMap *map);
void d2k_map_clear(D2KMap *map);

#endif

/* vi: set et ts=2 sw=2: */
