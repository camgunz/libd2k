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

#include <limits.h>

#include "d2k/internal.h"
#include "d2k/wad.h"

#define too_small(status) status_error( \
  status,                               \
  "d2k_wad",                            \
  D2K_WAD_TOO_SMALL,                    \
  "WAD is too small"                    \
)

#define invalid_identification(status) status_error(            \
  status,                                                       \
  "d2k_wad",                                                    \
  D2K_WAD_INVALID_IDENTIFICATION,                               \
  "invalid WAD identification (must be either 'IWAD' or 'PWAD'" \
)

#define wad_empty(status) status_error( \
  status,                               \
  "d2k_wad",                            \
  D2K_WAD_EMPTY,                        \
  "WAD is empty"                        \
)

#define invalid_lump_count(status) status_error( \
  status,                                        \
  "d2k_wad",                                     \
  D2K_WAD_INVALID_LUMP_COUNT,                    \
  "invalid lump count in WAD"                    \
)

#define invalid_info_table_offset(status) status_error( \
  status,                                               \
  "d2k_wad",                                            \
  D2K_WAD_INVALID_INFO_TABLE_OFFSET_IN_WAD,             \
  "invalid info table offset in WAD"                    \
)

#define lump_too_large(status) status_error(             \
  status,                                                \
  "d2k_wad",                                             \
  D2K_WAD_LUMP_TOO_LARGE,                                \
  "lump size exceeds maximum size (2,147,483,647 bytes)" \
)

static size_t get_lump_hash(const void *key, size_t seed) {
  return hash32(key, strlen((const char *)key), seed);
}

static void* get_lump_key_from_name(const void *obj) {
  return (void *)((D2KLump *)obj)->name;
}

static void* get_lump_key_from_name_and_ns(const void *obj) {
  return (void *)&((D2KLump *)obj)->nskey[0];
}

static bool lump_names_equal(const void *key1, const void *key2) {
  return strcmp((const char *)key1, (const char *)key2) == 0;
}

static bool lump_names_and_ns_equal(const void *key1, const void *key2) {
  return memcmp(key1, key2, sizeof(D2KLumpNamespace) + 8) == 0;
}

static inline bool is_marker(const char *marker, const char *name) {
  // doubled first character test for single-character prefixes only
  // FF_* is valid alias for F_*, but HI_* should not allow HHI_*
  return !strnicmp(name, marker, 8) || (marker[1] == '_' &&
                                        *name == *marker &&
                                        !strnicmp(name + 1, marker, 7));
}

static bool coalesce_and_mark_lumps(D2KLumpDirectory *lump_directory,
                                    const char *start_marker_name,
                                    const char *end_marker_name,
                                    D2KLumpNamespace ns,
                                    Status *status) {
  size_t   marked_count = 0;
  bool     in_marked = false;
  D2KLump *start_marker = NULL;
  D2KLump *end_marker = NULL;

  for (size_t i = 0; i < lump_directory->lumps.len; i++) {
    D2KLump *lump = parray_index_fast(&lump_directory->lumps, i);

    if (is_marker(start_marker_name, lump->name)) {
      in_marked = true;
    }
    else if (is_marker(end_marker_name, lump->name)) {
      in_marked = false;
    }
    else if (in_marked || lump->ns == ns) {
      // sf 26/10/99:
      // ignore sprite lumps smaller than 8 bytes (the smallest possible)
      // in size -- this was used by some dmadds wads
      // as an 'empty' graphics resource
      if (ns == D2K_LUMP_NAMESPACE_SPRITES && lump->data.len <= 8) {
        continue;
      }

      D2KLump ns_lump = *lump;

      ns_lump.ns = ns;

      if (!parray_append(&lump_directory->lumps, (void **)&ns_lump, status)) {
        return false;
      }

      parray_delete_fast(&lump_directory->lumps, i);
      marked_count++;
    }
  }

  if (!marked_count) {
    return status_ok(status);
  }

  if (!parray_prepend(&lump_directory->lumps, (void **)&start_marker,
                                              status)) {
    return false;
  }

  if (!parray_insert(&lump_directory->lumps, marked_count + 1,
                                             (void **)&end_marker,
                                             status)) {
    return false;
  }

  strncpy(start_marker->name, start_marker_name, 8);
  start_marker->ns = D2K_LUMP_NAMESPACE_GLOBAL;
  start_marker->wad = NULL;
  start_marker->data.data = NULL;
  start_marker->data.len = 0;

  strncpy(end_marker->name, end_marker_name, 8);
  end_marker->ns = D2K_LUMP_NAMESPACE_GLOBAL;
  end_marker->wad = NULL;
  end_marker->data.data = NULL;
  end_marker->data.len = 0;

  return status_ok(status);
}

static bool load_wad_lumps(D2KWad *wad, Status *status) {
  char    identification[4];
  int32_t numlumps;
  int32_t infotableofs;
  size_t  lump_count;
  size_t  info_table_offset;

  if (wad->data.len < ((sizeof(char) * 4) + (sizeof(int32_t) * 2))) {
    return too_small(status);
  }

  if (!buffer_read(&wad->data, 0, sizeof(char) * 4, (void *)&identification[0],
                                                    status)) {
    return false;
  }

  if (((identification[0] != 'P') && (identification[0] != 'I')) ||
                                     (identification[1] != 'W')  ||
                                     (identification[2] != 'A')  ||
                                     (identification[3] != 'D')) {
    return invalid_identification(status);
  }

  if (!buffer_read(&wad->data, 4, sizeof(int32_t), (void *)&numlumps,
                                                   status)) {
    return false;
  }

  numlumps = cble32(numlumps);

  if (numlumps == 0) {
    return wad_empty(status);
  }

  if (numlumps < 0) {
    return invalid_lump_count(status);
  }

  lump_count = (size_t)numlumps;

  if (!buffer_read(&wad->data, 8, sizeof(int32_t), (void *)&infotableofs,
                                                   status)) {
    return false;
  }

  infotableofs = cble32(infotableofs);

  if (infotableofs < 28) {
    return invalid_info_table_offset(status);
  }

  info_table_offset = (size_t)infotableofs;

  if (!array_init_alloc_zero(&wad->lumps, sizeof(D2KLump), lump_count,
                                                           status)) {
    return false;
  }

  for (size_t i = 0; i < lump_count; i++) {
    int32_t  filepos;
    int32_t  size;
    size_t   entry_start = info_table_offset + (i * 12);
    size_t   lump_data_start;
    size_t   lump_data_len;
    D2KLump *lump = NULL;

    if (!array_append(&wad->lumps, (void **)&lump, status)) {
      return false;
    }

    if (!buffer_read(&wad->data, entry_start, sizeof(int32_t),
                                              (void *)&filepos,
                                              status)) {
      array_free(&wad->lumps);
      return false;
    }

    lump_data_start = (size_t)cble32(filepos);

    if (!buffer_read(&wad->data, entry_start + 4, sizeof(int32_t),
                                                  (void *)&size,
                                                  status)) {
      array_free(&wad->lumps);
      return false;
    }

    lump_data_len = (size_t)cble32(size);

    if (!buffer_slice(&wad->data, lump_data_start, lump_data_len, &lump->data,
                                                                  status)) {
      return false;
    }

    lump->ns = D2K_LUMP_NAMESPACE_GLOBAL;
    lump->wad = wad;
    strncpy(lump->name, wad->data.data + entry_start + 8, 8);
    lump->name[8] = '\0';

    // IWAD file used as resource PWAD must not override TEXTURE1 or PNAMES
    if ((wad->source != WAD_SOURCE_IWAD) &&
        ((wad->source == WAD_SOURCE_LUMP) ||
         (!memcmp(wad->data.data, "IWAD", 4))) &&
        ((!strnicmp(lump->name, "TEXTURE1", 8)) ||
         (!strnicmp(lump->name, "PNAMES", 6)))) {
      strncpy(lump->name, "-IGNORE-", 8);
    }
  }

  return status_ok(status);
}

bool d2k_wad_init_from_path(D2KWad *wad, D2KWadSource source, Path *path,
                                                              Status *status) {
  buffer_init(&wad->data);

  if (!path_file_read(path, &wad->data, status)) {
    return false;
  }

  wad->source = source;

  return load_wad_lumps(wad, status);
}

bool d2k_wad_init_from_data(D2KWad *wad, D2KWadSource source, Buffer *buffer,
                                                              Status *status) {
  if (!buffer_init_alloc(&wad->data, buffer->len, status)) {
    return false;
  }

  wad->source = source;

  buffer_copy_fast(&wad->data, buffer);

  return load_wad_lumps(wad, status);
}

bool d2k_wad_init_from_lump_file_data(D2KWad *wad, SSlice *lump_name,
                                                   Buffer *lump_data,
                                                   Status *status) {
  int32_t lump_count = 1;
  int32_t info_table_offset = lump_data->len + 12;
  int32_t file_position = 12;

  if (lump_data->len > INT_MAX) {
    return lump_too_large(status);
  }

  if (!buffer_init_alloc(&wad->data, lump_data->len + 28, status)) {
    return false;
  }

  buffer_append_fast(&wad->data, "PWAD", 4);
  buffer_append_fast(&wad->data, (void *)&lump_count, 4);
  buffer_append_fast(&wad->data, (void *)&info_table_offset, 4);
  buffer_append_fast(&wad->data, lump_data->data, lump_data->len);
  buffer_append_fast(&wad->data, (void *)&file_position, 4);
  buffer_append_fast(&wad->data, (void *)&lump_data->len, 4);
  buffer_append_fast(&wad->data, (void *)lump_name->data, 8);

  return load_wad_lumps(wad, status);
}

bool d2k_wad_init_from_lump_file(D2KWad *wad, Path *lump_file_path,
                                              Status *status) {
  SSlice lump_name;
  Buffer lump_data;

  /*
   * [FIXME]
   *
   * The name of the lump here would be dependent on the local filesystem
   * encoding in Boom (and maybe earlier), but we leave it as UTF-8 here.
   * In order to reproduce this behavior we'd have to transcode the basename
   * (which is in UTF-8) to the local filesystem encoding and then use the
   * first 8 bytes.  But this code clearly never considered non-ASCII
   * filesystem encodings, so this case is more or less an error condition.  If
   * it ends up that something depends on this, it's relatively simple to
   * reproduce the "bug".
   */

  if (!path_basename(lump_file_path, &lump_name, status)) {
    return false;
  }

  while (lump_name.byte_len > 8) {
    if (!sslice_truncate_rune(&lump_name, status)) {
      return false;
    }
  }

  buffer_init(&lump_data);

  if (!path_file_read(lump_file_path, &lump_data, status)) {
    buffer_free(&lump_data);
    return false;
  }

  if (!d2k_wad_init_from_lump_file_data(wad, &lump_name, &lump_data, status)) {
    buffer_free(&lump_data);
    return false;
  }

  buffer_free(&lump_data);

  return status_ok(status);
}

bool d2k_lump_directory_init(D2KLumpDirectory *lump_directory,
                             PArray *wads,
                             Status *status) {
  size_t lump_count = 0;

  for (size_t i = 0; i < wads->len; i++) {
    D2KWad *wad = parray_index_fast(wads, i);
      
    lump_count += wad->lumps.len;
  }

  if (!parray_init_alloc(&lump_directory->lumps, lump_count, status)) {
    return false;
  }

  for (size_t i = 0; i < wads->len; i++) {
    D2KWad *wad = parray_index_fast(wads, i);

    for (size_t j = 0; j < wad->lumps.len; i++) {
      D2KLump *lump = array_index_fast(&wad->lumps, i);

      if (!parray_append(&lump_directory->lumps, (void *)lump, status)) {
        parray_free(&lump_directory->lumps);
        return false;
      }
    }
  }

  if (!coalesce_and_mark_lumps(lump_directory, "S_START",
                                               "S_END",
                                               D2K_LUMP_NAMESPACE_SPRITES,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!coalesce_and_mark_lumps(lump_directory, "F_START",
                                               "F_END",
                                               D2K_LUMP_NAMESPACE_FLATS,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!coalesce_and_mark_lumps(lump_directory, "C_START",
                                               "C_END",
                                               D2K_LUMP_NAMESPACE_COLORMAPS,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!coalesce_and_mark_lumps(lump_directory, "B_START",
                                               "B_END",
                                               D2K_LUMP_NAMESPACE_PRBOOM,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!coalesce_and_mark_lumps(lump_directory, "HI_START",
                                               "HI_END",
                                               D2K_LUMP_NAMESPACE_HIRES,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!table_init(&lump_directory->lookups[0], get_lump_hash,
                                               get_lump_key_from_name,
                                               lump_names_equal,
                                               0,
                                               status)) {
    parray_free(&lump_directory->lumps);
    return false;
  }

  if (!table_init(&lump_directory->lookups[0],
                  get_lump_hash,
                  get_lump_key_from_name_and_ns,
                  lump_names_and_ns_equal,
                  0,
                  status)) {
    parray_free(&lump_directory->lumps);
    table_free(&lump_directory->lookups[0]);
    return false;
  }

  for (size_t i = 0; i < lump_directory->lumps.len; i++) {
    D2KLump *lump = parray_index_fast(&lump_directory->lumps, i);

    if (!table_insert(&lump_directory->lookups[0], (void *)lump, status)) {
      parray_free(&lump_directory->lumps);
      table_free(&lump_directory->lookups[0]);
      table_free(&lump_directory->lookups[1]);
      return false;
    }

    if (lump->ns != D2K_LUMP_NAMESPACE_GLOBAL) {
      if (!table_insert(&lump_directory->lookups[1], (void *)lump, status)) {
        parray_free(&lump_directory->lumps);
        table_free(&lump_directory->lookups[0]);
        table_free(&lump_directory->lookups[1]);
        return false;
      }
    }

    lump->index = i;
  }

  return status_ok(status);
}

bool d2k_lump_directory_lookup(D2KLumpDirectory *lump_directory,
                               const char *lump_name,
                               D2KLump **lump,
                               Status *status) {
  return table_lookup(&lump_directory->lookups[0], (void *)lump_name,
                                                   (void **)lump,
                                                   status);
}

bool d2k_lump_directory_lookup_ns(D2KLumpDirectory *lump_directory,
                                  const char *lump_name,
                                  D2KLumpNamespace ns,
                                  D2KLump **lump,
                                  Status *status) {
  char nskey[sizeof(D2KLumpNamespace) + 8];

  cbmemmove(&nskey[0], (void *)&ns, sizeof(D2KLumpNamespace));
  cbmemmove(&nskey[sizeof(D2KLumpNamespace)], lump_name, 8);

  return table_lookup(&lump_directory->lookups[1], (void *)&nskey[0],
                                                   (void **)lump,
                                                   status);
}

/* vi: set et ts=2 sw=2: */
