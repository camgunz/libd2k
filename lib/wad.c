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

#include "internal.h"

#include "wad.h"

#define invalid_lump_directory_location_in_wad_header(status) status_error( \
  status,                                                                   \
  "d2k_wad",                                                                \
  D2K_WAD_ERROR_INVALID_LUMP_DIRECTORY_LOCATION_IN_WAD_HEADER,              \
  "invalid lump directory location in WAD header"                           \
)

#define invalid_wad_source(status) status_error( \
  status,                                        \
  "d2k_wad",                                     \
  D2K_WAD_ERROR_INVALID_WAD_SOURCE,              \
  "invalid WAD source"                           \
)

static size_t get_lump_hash(const void *key, size_t seed) {
  return hash32(key, strlen((const char *)key), seed);
}

static void* get_lump_key_from_name(const void *obj) {
  return (void *)((Lump *)obj)->name
}

static void* get_lump_key_from_name_and_ns(const void *obj) {
  return (void *)&((Lump *)obj)->nskey[0];
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
                                    LumpNamespace ns,
                                    Status *status) {
  size_t   marked_count = 0;
  bool     in_marked = false;
  D2kLump *ns_lump = NULL;
  D2kLump *start_marker = NULL;
  D2kLump *end_marker = NULL;

  for (size_t i = 0; i < lump_directory->lumps->len; i++) {
    D2KLump *lump = &array_index_fast(lump_directory->lumps, i);

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
      if (ns == LUMP_NAMESPACE_SPRITES && lump->size <= 8) {
        continue;
      }

      D2KLump ns_lump = *lump;

      ns_lump.ns = ns;

      if (!array_append(lump_directory->lumps, (void **)&ns_lump, status)) {
        return false;
      }

      array_delete_fast(lump_directory->lumps, i);
      marked_count++;
    }
  }

  if (!marked_count) {
    return;
  }

  if (!array_prepend(lump_directory->lumps, (void **)&start_marker, status)) {
    return false;
  }

  if (!array_insert(lump_directory->lumps, marked_count + 1,
                                           (void **)&end_marker,
                                           status)) {
    return false;
  }

  strncpy(start_marker->name, start_marker_name, 8);
  start_marker->ns = LUMP_NAMESPACE_GLOBAL;
  start_marker->wad = NULL;
  start_marker->size = 0;
  start_marker->data = NULL;

  strncpy(end_marker->name, end_marker_name, 8);
  end_marker->ns = LUMP_NAMESPACE_GLOBAL;
  end_marker->wad = NULL;
  end_marker->size = 0;
  end_marker->data = NULL;
}

static bool load_wad_lumps(D2KWad *wad, Status *status) {
  char    identification[4];
  int32_t numlumps;
  int32_t infotableofs;
  size_t  lump_count;
  size_t  info_table_offset;

  if (wad->data.len < ((sizeof(char) * 4) + (sizeof(int32_t) * 2))) {
    return wad_too_small(status);
  }

  cbmemmove(&identification[0], wad->data.data, 4);

  if (((identification[0] != 'P') && (identification[0] != 'I')) ||
                                     (identification[1] != 'W')  ||
                                     (identification[2] != 'A')  ||
                                     (identification[3] != 'D')) {
    return invalid_wad_identification(status);
  }

  cbmemmove((void *)&numlumps, wad->data.data + 4, 4);
  lump_count = (size_t)cble32(numlumps);

  if (lump_count == 0) {
    return empty_wad(status);
  }

  if (lump_count < 0) {
    return invalid_lump_count_in_wad(status);
  }

  cbmemmove((void *)&infotableofs, wad->data.data + 8, 4);
  info_table_offset = (size_t)cble32(infotableofs);

  if (info_table_offset < 28) {
    return invalid_info_table_offset_in_wad(status);
  }

  if (!array_init_alloc_zero(&wad->lumps, sizeof(D2KLump), lump_count,
                                                           status)) {
    return false;
  }

  for (size_t i = 0; i < lump_count; i++) {
    int32_t  filepos;
    int32_t  size;
    size_t   entry_start = info_table_offset + (i * 12);
    Lump    *lump = NULL;

    if (!array_append(&wad->lumps, (void **)&lump, status)) {
      return false;
    }

    cbmemmove((void *)&filepos, wad->data.data[entry_start], 4);
    lump_data_start = (size_t)cble32(filepos);

    cbmemmove((void *)&size, wad->data.data[entry_start + 4], 4);
    lump_data_len = (size_t)cble32(size);

    if (!buffer_slice(&wad->data, lump_data_start, lump_data_len, &lump->data,
                                                                  status)) {
      return false;
    }

    lump->ns = D2K_LUMP_NAMESPACE_GLOBAL;
    lump->wad = wad;
    strncpy(lump->name, wad->data.data[entry_start + 8], 8);
    lump->name[8] = '\0';

    // IWAD file used as resource PWAD must not override TEXTURE1 or PNAMES
    if ((wad->source != WAD_SOURCE_IWAD) &&
        ((wad->source == WAD_SOURCE_LUMP) ||
         (!memcmp(wad->data.data, "IWAD", 4))) &&
        ((!strnicmp(wad_lump_entry->name, "TEXTURE1", 8)) ||
         (!strnicmp(wad_lump_entry->name, "PNAMES", 6)))) {
      strncpy(lump->name, "-IGNORE-", 8);
    }
  }

  return true;
}

bool d2k_wad_init_from_path(D2KWad *wad, D2KWadSource source, Path *path,
                                                              Status *status) {
  if (!buffer_init(&wad->data, status)) {
    return false;
  }

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

  if (lump_data->len > ((1 << 31) - 1)) {
    return lump_too_large(status);
  }

  if (!buffer_init_alloc(&wad->buffer, lump_data->len + 28, status)) {
    return false;
  }

  buffer_append_bytes_fast(&wad->data, "PWAD", 4);
  buffer_append_bytes_fast(&wad->data, (void *)lump_count, 4);
  buffer_append_bytes_fast(&wad->data, (void *)info_table_offset, 4);
  buffer_append_bytes_fast(&wad->data, lump_data->data, lump_data->len);
  buffer_append_bytes_fast(&wad->data, (void *)&file_position, 4);
  buffer_append_bytes_fast(&wad->data, (void *)&lump_data->len, 4);
  buffer_append_bytes_fast(&wad->data, (void *)lump_name->data, 8);

  return load_wad_lumps(wad, status);
}

bool d2k_wad_init_from_lump_file(D2KWad *wad, Path *lump_file_path,
                                              Status *status) {
  SSlice lump_name;
  Buffer lump_data;

  if (!buffer_init(&buffer, status)) {
    return false;
  }

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

  if (!path_basename(resource_file_path, &lump_name, status)) {
    return false;
  }

  while (lump_name.byte_len > 8) {
    if (!sslice_truncate_rune(&lump_name, status)) {
      return false;
    }
  }

  if (!buffer_init(&lump_data, status)) {
    return false;
  }

  if (!path_read_file(lump_file_path, &lump_data, status)) {
    buffer_free(&lump_data);
    return false;
  }

  if (!init_wad_from_lump_file_data(wad, &lump_name, &lump_data, status)) {
    buffer_free(&lump_data);
    return false;
  }

  buffer_free(&lump_data);
  return true;
}

bool d2k_lump_directory_init(D2KLumpDirectory *lump_directory,
                             PArray *wads,
                             Status *status) {
  size_t lump_count = 0;

  if (!table_init(&lump_directory->lump_lookup[0], get_lump_hash,
                                                   get_lump_key_from_name,
                                                   lump_names_equal,
                                                   0,
                                                   status)) {
    return false;
  }

  if (!table_init(&lump_directory->lump_lookup[0],
                  get_lump_hash,
                  get_lump_key_from_name_and_ns,
                  lump_names_and_ns_equal,
                  0,
                  status)) {
    return false;
  }

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
      D2KLump *lump = array_index_fast(wad->lumps, i);

      if (!parray_append(&lump_directory->lumps, (void *)lump, status)) {
        return false;
      }
    }
  }

  coalesce_and_mark_lumps(
    lump_directory,
    "S_START",
    "S_END",
    LUMP_NAMESPACE_SPRITES
  );

  coalesce_and_mark_lumps(
    lump_directory,
    "F_START",
    "F_END",
    LUMP_NAMESPACE_FLATS
  );

  coalesce_and_mark_lumps(
    lump_directory,
    "C_START",
    "C_END",
    LUMP_NAMESPACE_COLORMAPS
  );

  coalesce_and_mark_lumps(
    lump_directory,
    "B_START",
    "B_END",
    LUMP_NAMESPACE_PRBOOM
  );

  
  coalesce_and_mark_lumps(
    lump_directory,
    "HI_START",
    "HI_END",
    LUMP_NAMESPACE_HIRES
  );

  for (size_t i = 0; i < wad->lumps->len; i++) {
    Lump *lump = array_index_fast(lump_directory->lumps.len, i);

    if (!table_insert(&lump_directory->lookups[0], (void *)lump, status)) {
      return false;
    }

    if (lump->ns != LUMP_NAMESPACE_GLOBAL) {
      if (!table_insert(&lump_directory->lookups[1], (void *)lump, status)) {
        return false;
      }
    }
  }
}

bool d2k_lump_directory_lookup(D2KLumpDirectory *lump_directory,
                               const char *lump_name,
                               Lump **lump,
                               Status *status) {
  return table_lookup(&lump_directory->lookups[0], (void *)lump_name,
                                                   (void **)lump,
                                                   status);
}

bool d2k_lump_directory_lookup_ns(D2KLumpDirectory *lump_directory,
                                  const char *lump_name,
                                  D2KLumpNamespace ns,
                                  Status *status) {
  const char nskey[NSKEY_SIZE];

  memcpy(&nskey[0], (void *)&ns, sizeof(D2KLumpNamespace));
  memcpy(&nskey[sizeof(D2KLumpNamespace)], lump_name, 8);

  return table_lookup(&lump_directory->lookups[1], (void *)&nskey[0],
                                                   (void **)lump,
                                                   status);
}

/* vi: set et ts=2 sw=2: */
