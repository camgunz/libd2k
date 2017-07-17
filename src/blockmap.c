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

#define map_missing_blockmap_lump(status) status_error( \
  status,                                               \
  "d2k_map",                                            \
  D2K_MAP_MISSING_BLOCKMAP_LUMP,                        \
  "map missing BLOCKMAP lump"                           \
)

#define negative_blockmap_width(status) status_error( \
  status,                                             \
  "d2k_blockmap",                                     \
  D2K_MAP_BLOCKMAP_NEGATIVE_WIDTH,                    \
  "blockmap width is negative"                        \
)

#define negative_blockmap_height(status) status_error( \
  status,                                              \
  "d2k_blockmap",                                      \
  D2K_MAP_BLOCKMAP_NEGATIVE_HEIGHT,                    \
  "blockmap height is negative"                        \
)

#define truncated_blockmap_header(status) status_error( \
  status,                                               \
  "d2k_blockmap",                                       \
  D2K_MAP_BLOCKMAP_TRUNCATED_HEADER,                    \
  "truncated blockmap lump header"                      \
)

#define truncated_blockmap_line_list_directory(status) status_error( \
  status,                                                            \
  "d2k_blockmap",                                                    \
  D2K_MAP_BLOCKMAP_TRUNCATED_LINE_LIST_DIRECTORY,                    \
  "truncated blockmap line list directory"                           \
)

#define invalid_offset_in_blockmap_directory(status) status_error( \
  status,                                                          \
  "d2k_blockmap",                                                  \
  D2K_MAP_BLOCKMAP_INVALID_OFFSET_IN_LINE_LIST_DIRECTORY,          \
  "invalid offset in line list directory"                          \
)

/* places to shift rel position for cell num */
#define BLKSHIFT 7

/* mask for rel position within cell */
#define BLKMASK ((1 << BLKSHIFT) - 1)

static inline bool add_line(D2KBlockmap *bmap, Array *done, size_t block_index,
                                                            size_t line_index,
                                                            Status *status) {
  bool *skip = array_index_fast(done, block_index);

  if (!(*skip)) {
    size_t *block_line_index = NULL;

    if (!array_append(&bmap->blocks, (void **)block_line_index, status)) {
      return false;
    }

    *block_line_index = line_index;
  }

  return status_ok(status);
}

static inline void cleanup_blockmap(D2KBlockmap *blockmap, Array *done) {
  for (size_t i = 0; i < blockmap->blocks.len; i--) {
    array_free(array_index_fast(&blockmap->blocks, i));
  }

  array_free(&blockmap->blocks);
  array_free(done);
}

  d2k_blockmap_build
}

bool d2k_blockmap_build(D2KBlockmap *bmap, Array *vertexes, Array *linedefs,
                                                            Status *status) {
  int xorg;
  int yorg;
  int map_minx = INT_MAX;
  int map_miny = INT_MAX;
  int map_maxx = INT_MIN;
  int map_maxy = INT_MIN;
  D2KMap *map = map_loader->map;
  D2KBlockmap *bmap = &map->blockmap;
  Array done;
  size_t width;

  for (size_t i = 0; i < vertexes->len; i++) {
    D2KFixedVertex *v = array_index_fast(vertexes, i);

    if (v->x < map_minx) {
      map_minx = v->x;
    }
    else if (v->x > map_maxx) {
      map_maxx = v->x;
    }

    if (v->y < map_miny) {
      map_miny = v->y;
    }
    else if (v->y > map_maxy) {
      map_maxy = v->y;
    }
  }

  bmap->origin_x = map_minx;
  bmap->origin_y = map_miny;

  map_minx = d2k_fixed_point_to_int(map_minx);
  map_maxx = d2k_fixed_point_to_int(map_maxx);
  map_miny = d2k_fixed_point_to_int(map_miny);
  map_maxy = d2k_fixed_point_to_int(map_maxy);

  xorg = map_minx;
  yorg = map_miny;

  bmap->width  = (map_maxx - xorg + 1 + BLKMASK) >> BLKSHIFT;
  bmap->height = (map_maxy - yorg + 1 + BLKMASK) >> BLKSHIFT;

  array_init(&bmap->blocks, sizeof(Array));

  /*
   * No need to check for overflow on `bmap->width * bmap-height` because the
   * right shift makes it impossible.
   */

  if (!array_set_size(&bmap->blocks, bmap->width * bmap->height, status)) {
    array_free(&bmap->blocks);
    return false;
  }

  array_init(&done, sizeof(bool));

  if (!array_set_size(&done, bmap->blocks.len, status)) {
    array_free(&bmap->blocks);
    array_free(&done);
    return false;
  }

  for (size_t i = 0; i < bmap->blocks.len; i++) {
    Array *line_list = array_index_fast(&bmap->blocks, i);

    array_init(line_list, sizeof(size_t));
  }

  width = bmap->width;

  // For each linedef in the wad, determine all blockmap blocks it touches,
  // and add the linedef number to the blocklists for those blocks
  for (size_t i = 0; i < linedefs->len; i++) {
    D2KLinedef *line = array_index_fast(linedefs, i);
    int x1 = line->v1->x >> FRACBITS;
    int y1 = line->v1->y >> FRACBITS;
    int x2 = line->v2->x >> FRACBITS;
    int y2 = line->v2->y >> FRACBITS;
    int dx = x2 - x1;
    int dy = y2 - y1;
    bool vert = dx == 0;
    bool horiz = dy == 0;
    bool spos = (dx ^ dy) > 0;
    bool sneg = (dx ^ dy) < 0;
    int bx;
    int by;
    int minx = x1 > x2 ? x2 : x1;
    int maxx = x1 > x2 ? x1 : x2;
    int miny = y1 > y2 ? y2 : y1;
    int maxy = y1 > y2 ? y1 : y2;

    if (!array_zero_elements(&done, 0, done.len, status)) {
      cleanup_blockmap(bmap, &done);
      return false;
    }

    // The line always belongs to the blocks containing its endpoints

    bx = (x1 - xorg) >> BLKSHIFT;
    by = (y1 - yorg) >> BLKSHIFT;

    if (!add_line(bmap, &done, by * width + bx, i, status)) {
      cleanup_blockmap(bmap, &done);
      return false;
    }

    bx = (x2 - xorg) >> BLKSHIFT;
    by = (y2 - yorg) >> BLKSHIFT;

    if (!add_line(bmap, &done, by * width + bx, i, status)) {
      cleanup_blockmap(bmap, &done);
      return false;
    }

    // For each column, see where the line along its left edge, which
    // it contains, intersects the Linedef i. Add i to each corresponding
    // blocklist.

    if (!vert) { // don't interesect vertical lines with columns
      for (size_t j = 0; j < width; j++) {
        // intersection of Linedef with x = xorg + (j << BLKSHIFT)
        // (y - y1)  dx = dy * (x - x1)
        // y = dy * (x - x1) + y1 * dx;

        int x = xorg + (j << BLKSHIFT);       // (x,y) is intersection
        int y = (dy * (x - x1)) / dx + y1;
        int yb = (y - yorg) >> BLKSHIFT;      // block row number
        int yp = (y - yorg) & BLKMASK;        // y position within block

        if (yb < 0 || (unsigned int)yb > bmap->height - 1) {
          continue;
        }

        if (x < minx || x > maxx) {     // line doesn't touch column
          continue;
        }

        // The cell that contains the intersection point is always added
        if (!add_line(bmap, &done, width * yb + j, i, status)) {
          cleanup_blockmap(bmap, &done);
          return false;
        }

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which
        // blocks are hit

        if (yp == 0) {      // intersection at a corner
          if (sneg) {       //   \ - blocks x,y-, x-,y
            if (yb > 0 && miny < y) {
              if (!add_line(bmap, &done, width * (yb - 1) + j, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
            if (j > 0 && minx < x) {
              if (!add_line(bmap, &done, width * yb + j - 1, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
          else if (spos) { //   / - block x-,y-
            if (yb > 0 && j > 0 && minx < x) {
              if (!add_line(bmap, &done, width * (yb - 1) + j - 1, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
          else if (horiz) { //   - - block x-,y
            if (j > 0 && minx < x) {
              if (!add_line(bmap, &done, width * yb + j - 1, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
        }
        else if (j > 0 && minx < x) { // else not at corner: x-,y
          if (!add_line(bmap, &done, width * yb + j - 1, i, status)) {
            cleanup_blockmap(bmap, &done);
            return false;
          }
        }
      }
    }

    // For each row, see where the line along its bottom edge, which
    // it contains, intersects the Linedef i. Add i to all the corresponding
    // blocklists.

    if (!horiz) {
      for (size_t j = 0; j < bmap->height; j++) {
        // intersection of Linedef with y = yorg + (j << BLKSHIFT)
        // (x,y) on Linedef i satisfies: (y - y1) * dx = dy * (x - x1)
        // x = dx * (y - y1) / dy + x1;

        int y = yorg + (j << BLKSHIFT);       // (x,y) is intersection
        int x = (dx * (y - y1)) / dy + x1;
        int xb = (x - xorg) >> BLKSHIFT;      // block column number
        int xp = (x - xorg) & BLKMASK;        // x position within block

        if (xb < 0 || (size_t)xb > width - 1) {
          continue; // outside blockmap, continue
        }

        if (y < miny || y > maxy) {   // line doesn't touch row
          continue;
        }

        // The cell that contains the intersection point is always added

        if (add_line(bmap, &done, width * j + xb, i, status)) {
          cleanup_blockmap(bmap, &done);
          return false;
        }

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which
        // blocks are hit

        if (xp == 0) { // intersection at a corner
          if (sneg) { //   \ - blocks x,y-, x-,y
            if (j > 0 && miny < y) {
              if (!add_line(bmap, &done, width * (j - 1) + xb, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
            if (xb > 0 && minx < x) {
              if (!add_line(bmap, &done, width * j + xb - 1, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
          else if (vert) { //   | - block x,y-
            if (j > 0 && miny < y) {
              if (!add_line(bmap, &done, width * (j - 1) + xb, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
          else if (spos) { //   / - block x-,y-
            if (xb > 0 && j > 0 && miny < y) {
              if (!add_line(bmap, &done, width * (j - 1) + xb - 1, i, status)) {
                cleanup_blockmap(bmap, &done);
                return false;
              }
            }
          }
        }
        else if (j > 0 && miny < y) { // else not on a corner: x,y-
          if (!add_line(blockmap, &done, width * (j - 1) + xb, i, status)) {
            cleanup_blockmap(bmap, &done);
            return false;
          }
        }
      }
    }
  }

  array_free(&done);

  return status_ok(status);
}

bool d2k_blockmap_load_from_lump(D2KBlockmap *bmap, D2KLump *lump,
                                                    Status *status) {
  char header[BLOCKMAP_HEADER_SIZE];

  slice_read_fast(&lump->data, 
  int16_t bmaporgx;
  int16_t bmaporgy;
  int16_t bmapwidth;
  int16_t bmapheight;
  size_t  block_count;
  size_t  dir_start = 8;
  size_t  list_start;
  size_t  previous_offset;

  if (dir_start >= lump->data.len) {
    return truncated_blockmap_header(status);
  }

  if (!slice_read(&lump->data, 0, sizeof(int16_t), (void *)&bmaporgx,
                                                   status)) {
    return false;
  }

  if (!slice_read(&lump->data, 2, sizeof(int16_t), (void *)&bmaporgy,
                                                   status)) {
    return false;
  }

  if (!slice_read(&lump->data, 4, sizeof(int16_t), (void *)&bmapwidth,
                                                   status)) {
    return false;
  }

  if (!slice_read(&lump->data, 6, sizeof(int16_t), (void *)&bmapheight,
                                                   status)) {
    return false;
  }

  bmaporgx   = cble16(bmaporgx);
  bmaporgy   = cble16(bmaporgy);
  bmapwidth  = cble16(bmapwidth);
  bmapheight = cble16(bmapheight);

  if (bmapwidth < 0) {
    return negative_blockmap_width(status);
  }

  if (bmapheight < 0) {
    return negative_blockmap_height(status);
  }

  block_count = ((size_t)bmapwidth) * ((size_t)bmapheight);
  list_start = (dir_start + (2 * block_count));

  if (list_start >= lump->data.len) {
    return truncated_blockmap_line_list_directory(status);
  }

  previous_offset = list_start;

  array_init(&bmap->blocks, sizeof(Array));

  if (!array_set_size(&bmap->blocks, block_count, status)) {
    return false;
  }

  for (size_t i = 0; i < block_count; i++) {
    Array   *line_list;
    size_t   delta = 0;
    size_t   line_count = 0;
    size_t   line_list_pos = 8 + (i * 2);
    uint16_t offset = (
      (((uint8_t)(lump->data.data[line_list_pos    ])) << 8) +
       ((uint8_t)(lump->data.data[line_list_pos + 1]))
    );

    offset = cble16(offset);

    if ((offset < list_start) ||
        (offset > (lump->data.len - 4)) ||
        (offset < (previous_offset + 4))) {
      for (size_t j = i; j > 0; j--) {
        array_free(array_index_fast(&bmap->blocks, j - 1));
      }

      array_free(&bmap->blocks);

      return invalid_offset_in_blockmap_directory(status);
    }

    delta = offset - previous_offset;

    if ((delta % 2) != 0) {
      for (size_t j = i; j > 0; j--) {
        array_free(array_index_fast(&bmap->blocks, j - 1));
      }

      array_free(&bmap->blocks);

      return invalid_offset_in_blockmap_directory(status);
    }

    line_list = array_index_fast(&bmap->blocks, i);

    array_init(line_list, sizeof(size_t));
    line_count = delta / 2;

    if (!array_set_size(line_list, line_count, status)) {
      array_free(line_list);

      for (size_t j = i; j > 0; j--) {
        array_free(array_index_fast(&bmap->blocks, j - 1));
      }

      array_free(&bmap->blocks);

      return false;
    }

    for (size_t j = 0; j < line_count; j++) {
      size_t   *line_index_slot = array_index_fast(line_list, j);
      size_t    pos = offset + (j * 2);
      uint16_t  line_index = (
        (((uint8_t)(lump->data.data[pos    ])) << 8) +
         ((uint8_t)(lump->data.data[pos + 1]))
      );

      *line_index_slot = cble16(line_index);
    }

    previous_offset = offset;
  }

  bmap->width    = (size_t)bmapwidth;
  bmap->height   = (size_t)bmapheight;
  bmap->origin_x = bmaporgx << FRACBITS;
  bmap->origin_y = bmaporgy << FRACBITS;

  return status_ok(status);
}

bool d2k_map_loader_build_blockmap(D2KMapLoader *map_loader, Status *status) {
  return d2k_blockmap_build(&map_loader->map->blockmap,
                            &map_loader->map->vertexes,
                            &map_loader->map->linedefs,
                            status);
}

bool d2k_map_loader_load_blockmap(D2KMapLoader *map_loader, Status *status) {
  return d2k_blockmap_load_from_lump(
    &map_loader->map->blockmap,
    map_loader->map_lumps[D2K_MAP_LUMP_VANILLA_BLOCKMAP],
    status
  );
}
/* vi: set et ts=2 sw=2: */
