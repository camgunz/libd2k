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
struct D2KMapObject;
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
  D2K_MAP_GL_VERT_LUMP,
  D2K_MAP_MISSING_GL_SEGS_LUMP,
  D2K_MAP_MISSING_GL_SSECT_LUMP,
  D2K_MAP_MISSING_GL_NODES_LUMP,
  D2K_MAP_MULTIPLE_NODES_VERSIONS,
  D2K_MAP_TRUNCATED_NODES_LUMP,
  D2K_MAP_TRUNCATED_SSECTORS_LUMP,
  D2K_MAP_BLOCKMAP_NEGATIVE_WIDTH,
  D2K_MAP_BLOCKMAP_NEGATIVE_HEIGHT,
  D2K_MAP_BLOCKMAP_TRUNCATED_HEADER,
  D2K_MAP_BLOCKMAP_TRUNCATED_LINE_LIST_DIRECTORY,
  D2K_MAP_BLOCKMAP_INVALID_OFFSET_IN_LINE_LIST_DIRECTORY,
  D2K_MAP_MALFORMED_VERTEXES_LUMP,
  D2K_MAP_MALFORMED_GL_VERT_LUMP,
};

typedef enum {
  D2K_MAP_NODES_VERSION_VANILLA = 1,                  /* No marker */
  D2K_MAP_NODES_VERSION_DEEP_BSP_4,                   /* xNd40000, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM,                        /* XNOD, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM_COMPRESSED,             /* ZNOD, NODES */
  D2K_MAP_NODES_VERSION_ZDOOM_GL,                     /* XGLN, SSECTORS */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_COMPRESSED,          /* ZGLN, SSECTORS */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_EXTENDED,            /* XGL2, ZNODES (UDMF) */
  D2K_MAP_NODES_VERSION_ZDOOM_GL_EXTENDED_COMPRESSED, /* ZGL2, ZNODES (UDMF) */
  D2K_MAP_NODES_VERSION_GL_NODES_1,                   /* No marker */
  D2K_MAP_NODES_VERSION_GL_NODES_2,                   /* gNd2, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_3,                   /* gNd3, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_4,                   /* gNd4, GL_NODES */
  D2K_MAP_NODES_VERSION_GL_NODES_5,                   /* gNd5, GL_NODES */
} D2KMapNodesVersion;

typedef enum {
  D2K_LINEDEF_SLOPE_TYPE_HORIZONTAL,
  D2K_LINEDEF_SLOPE_TYPE_VERTICAL,
  D2K_LINEDEF_SLOPE_TYPE_POSITIVE,
  D2K_LINEDEF_SLOPE_TYPE_NEGATIVE
} D2KLinedefSlopeType;

typedef enum {
  D2K_LINEDEF_RENDER_FLAG_TOP_TILE =  1,
  D2K_LINEDEF_RENDER_FLAG_MID_TILE =  2,
  D2K_LINEDEF_RENDER_FLAG_BOT_TILE =  4,
  D2K_LINEDEF_RENDER_FLAG_IGNORE   =  8,
  D2K_LINEDEF_RENDER_FLAG_CLOSED   = 16,
  D2K_LINEDEF_RENDER_FLAG_ISOLATED = 32,
} D2KLinedefRenderFlags;

typedef struct D2KFixedVertexStruct {
  D2KFixedPoint x;
  D2KFixedPoint y;
  D2KAngle      view_angle;
  int           angle_time;
} D2KFixedVertex;

typedef struct D2KMapNodeStruct {
  D2KFixedPoint x;
  D2KFixedPoint y;
  D2KFixedPoint dx;
  D2KFixedPoint dy;
  D2KFixedPoint bbox[2][4];
  int           children[2];
} D2KMapNode;

typedef struct D2KSectorStruct {
  uint32_t                        id;
  uint32_t                        flags;
  D2KFixedPoint                   floor_height;
  D2KFixedPoint                   ceiling_height;
  int                             next_tag;
  int                             first_tag;
  int                             sound_traversed;
  struct D2KMapObjectStruct      *sound_target;
  int                             blockbox[4];
  int                             bbox[4];
  D2KSoundOrigin                  sound_origin;
  int                             valid_count;
  struct D2KMapObjectStruct      *things;
  int                             friction;
  int                             move_factor;
  void                           *floor_data;
  void                           *ceiling_data;
  void                           *lighting_data;
  int                             stair_lock;
  int                             previous_sector;
  int                             next_sector;
  int                             height_sector;
  int                             bottom_map;
  int                             mid_map;
  int                             top_map;
  struct D2KMapSectorNodeStruct  *touching_thinglist;
  PArray                          lines;
  int                             sky;
  D2KFixedPoint                   floor_x_offset;
  D2KFixedPoint                   floor_y_offset;
  D2KFixedPoint                   ceiling_x_offset;
  D2KFixedPoint                   ceiling_y_offset;
  int                             floor_light_sector;
  int                             ceiling_light_sector;
  size_t                          floor_pic;
  size_t                          ceiling_pic;
  short                           light_level;
  short                           special;
  short                           old_special;
  short                           tag;
  int                             sector_floor_interpolation;
  int                             sector_ceiling_interpolation;
  int                             floor_panning_interpolation;
  int                             ceiling_panning_interpolation;
  int                             fakegroup[2];
} D2KSector;

typedef struct D2KLinedefStruct {
  uint32_t                id;
  D2KFixedVertex         *v1;
  D2KFixedVertex         *v2;
  D2KFixedPoint           dx;
  D2KFixedPoint           dy;
  float                   texel_length;
  unsigned short          flags;
  short                   special;
  short                   tag;
  unsigned short          sidenum[2];
  D2KFixedPoint           bbox[4];
  D2KLinedefSlopeType     slope;
  struct D2KSectorStruct *front_sector;
  struct D2KSectorStruct *back_sector;
  int                     valid_count;
  void                   *special_data;
  int                     tran_lump;
  int                     first_tag;
  int                     next_tag;
  int                     r_valid_count;
  D2KLinedefRenderFlags   r_flags;
  D2KSoundOrigin          sound_origin;
} D2KLinedef;

typedef struct D2KSidedefStruct {
  D2KFixedPoint  texture_offset;
  D2KFixedPoint  row_offset;
  short          top_texture;
  short          bottom_texture;
  short          mid_texture;
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

typedef struct D2KSubSectorStruct {
  D2KSector *sector;
  int        line_count;
  size_t     first_line;
} D2KSubSector;

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

typedef struct D2KBlockmapStruct {
  size_t        width;
  size_t        height;
  D2KFixedPoint origin_x;
  D2KFixedPoint origin_y;
  Array         blocks;
} D2KBlockmap;

typedef struct D2KMapStruct {
  D2KMapNodesVersion nodes_version;
  Array              vertexes;
  Array              segs;
  Array              sectors;
  Array              subsectors;
  Array              nodes;
  Array              lines;
  Array              sides;
  Array              sslines;
  D2KBlockmap        blockmap;
} D2KMap;

bool d2k_map_init(D2KMap *map, struct D2KLumpDirectoryStruct *lump_directory,
                               const char *map_name,
                               const char *gl_map_name,
                               Status *status);

bool d2k_blockmap_init_from_map(D2KBlockmap *bmap, D2KMap *map,
                                                   Status *status);
bool d2k_blockmap_init_from_lump(D2KBlockmap *bmap, struct D2KLumpStruct *lump,
                                                    Status *status);

static inline bool d2k_map_has_gl_nodes(D2KMap *map) {
  switch (map->nodes_version) {
    case D2K_MAP_NODES_VERSION_GL_NODES_1:
    case D2K_MAP_NODES_VERSION_GL_NODES_2:
    case D2K_MAP_NODES_VERSION_GL_NODES_3:
    case D2K_MAP_NODES_VERSION_GL_NODES_4:
    case D2K_MAP_NODES_VERSION_GL_NODES_5:
      return true;
    default:
      break;
  }

  return false;
}

#endif

/* vi: set et ts=2 sw=2: */
