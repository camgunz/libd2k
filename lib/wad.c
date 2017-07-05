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

#include "d2k_internal.h"

#include "wad.h"

#define invalid_lump_directory_location_in_wad_header(status) status_error( \
  status,                                                                   \
  "d2k_wad",                                                                \
  INVALID_LUMP_DIRECTORY_LOCATION_IN_WAD_HEADER,                            \
  "invalid lump directory location in WAD header"                           \
)

#define invalid_wad_source(status) status_error( \
  status,                                        \
  "d2k_wad",                                     \
  INVALID_WAD_SOURCE,                            \
  "invalid WAD source"                           \
)

typedef struct wad_lump_entry_s {
  int filepos;
  int size;
  char name[8];
} wad_lump_entry_t;

static inline bool is_marker(const char *marker, const char *name) {
  // doubled first character test for single-character prefixes only
  // FF_* is valid alias for F_*, but HI_* should not allow HHI_*
  return !strnicmp(name, marker, 8) || (marker[1] == '_' &&
                                        *name == *marker &&
                                        !strnicmp(name + 1, marker, 7));
}

static bool coalesce_marked_lumps(Wad *wad, const char *start_marker_name,
                                            const char *end_marker_name,
                                            LumpNamespace ns,
                                            Status *status) {
  size_t  marked_count = 0;
  bool    in_marked = false;
  Lump   *ns_lump = NULL;
  Lump   *start_marker = NULL;
  Lump   *end_marker = NULL;

  for (size_t i = 0; i < wad->lumps->len; i++) {
    Lump *lump = &array_index(wad->lumps, Lump, i);

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

      Lump ns_lump = *lump;

      ns_lump.ns = ns;

      if (!array_append(wad->lumps, (void **)&ns_lump, status)) {
        return false;
      }

      array_delete_fast(wad->lumps, i);
      marked_count++;
    }
  }

  if (!marked_count) {
    return;
  }

  if (!array_prepend(wad->lumps, (void **)&start_marker, status)) {
    return false;
  }

  if (!array_insert(wad->lumps, marked_count + 1, (void **)&end_marker,
                                                  status)) {
    return false;
  }

  strncpy(start_marker->name, start_marker_name, 8);
  start_marker->ns = LUMP_NAMESPACE_GLOBAL;
  start_marker->wad = wad;
  start_marker->size = 0;
  start_marker->data = NULL;

  strncpy(end_marker->name, end_marker_name, 8);
  end_marker->ns = LUMP_NAMESPACE_GLOBAL;
  end_marker->wad = wad;
  end_marker->size = 0;
  end_marker->data = NULL;
}

static void free_lump(gpointer data) {
  Lump *lump = (Lump *)data;

  memset(lump->name, 0, sizeof(lump->name));
  lump->ns = LUMP_NAMESPACE_GLOBAL;
  lump->wad = NULL;
  lump->size = 0;
  if (lump->data) {
    free(lump->data);
  }
  lump->data = NULL;
}

static bool load_resource(WadSource source, Path *resource_file,
                                            Status *status) {
  wad_header_t      header;
  wad_lump_entry_t *wad_lump;
  File             *fobj;
  Lump             *lump;
  char              identification[4]; // Should be "IWAD" or "PWAD".
  int               numlumps;
  int               infotableofs;
  SSlice            basename;
  size_t            name_byte_count;

  if (!path_file_open(resource_file, &fobj, "rb", status)) {
    return false;
  }

  previous_lump_count = wad->lumps->len;

  switch (source) {
    case WAD_SOURCE_IWAD:
    case WAD_SOURCE_PWAD:
      if (!file_read_raw(file, (void *)&identification[0],
                               sizeof(identification),
                               status)) {
        return false;
      }

      if (((identification[0] != 'P') && (identification[0] != 'I')) ||
                                         (identification[1] != 'W')  ||
                                         (identification[1] != 'A')  ||
                                         (identification[1] != 'D')) {
        file_close(fobj, NULL);
        return false;
      }

      if (!file_read_raw(file, (void *)&numlumps, 4, status)) {
        return false;
      }

      if (!file_read_raw(file, (void *)&infotableofs, 4, status)) {
        return false;
      }

      numlumps = cble32(numlumps);
      infotableofs = cble32(infotableofs);

      if (!file_seek(fobj, (size_t)infotableofs, SEEK_SET)) {
        file_close(fobj, NULL);
        return invalid_lump_directory_location_in_wad_header(status);
      }

      if (!array_ensure_capacity(wad->lumps, wad->lumps->len + header.numlumps,
                                             status)) {
        file_close(fobj, NULL);
        array_set_size(wad->lumps, previous_lump_count, NULL);
        return false;
      }

      for (size_t i = 0; i < header.numlumps; i++) {
        if (!array_append(wad->lumps, (void **)&lump, status)) {
          file_close(fobj, NULL);
          array_set_size(wad->lumps, previous_lump_count, NULL);
          return false;
        }

        if (!file_read_raw(file, (void *)lump, sizeof(wad_lump_entry_t), status)) {
          file_close(fobj, NULL);
          array_set_size(wad->lumps, previous_lump_count, NULL);
          return false;
        }
      }

      break;
    case WAD_SOURCE_LUMP:
      if (!array_append(wad->lumps, (void **)&lump, status)) {
        file_close(fobj, NULL);
        array_set_size(wad->lumps, previous_lump_count, NULL);
        return false;
      }

      if (!file_read_raw(file, (void *)lump, sizeof(lump_entry_t), status)) {
        file_close(fobj, NULL);
        array_set_size(wad->lumps, previous_lump_count, NULL);
        return false;
      }

      /*
       * [FIXME]
       *
       * The name of the lump here would be dependent on the local filesystem
       * encoding in Boom (and maybe earlier), but we leave it as UTF-8 here.
       * In order to reproduce this behavior, we'd have to transcode the
       * basename (which is in UTF-8) to the local filesystem encoding and then
       * use the first 8 bytes.  But this code clearly never considered
       * non-ASCII filesystem encodings, so this case is more or less an error
       * condition.  If it ends up that something depends on this, it's
       * relatively simple to reproduce the "bug".
       */

      if (!path_basename(resource_file_path, &basename, status)) {
        file_close(fobj, NULL);
        return false;
      }

      while (basename.byte_len > sizeof(lump->name)) {
        if (!sslice_truncate_rune(&basename, status)) {
          file_close(fobj, NULL);
          return false;
        }
      }

      memcpy(lump->name, basename, basename.byte_len);
    default:
      file_close(fobj, NULL);
      return invalid_wad_source(status);
  }

  for (size_t i = 0; i < lump_count; i++) {
  bool read_lump(Lump *lump, File *fobj, Lump *lump, Status *status) {
    wad_lump_entry_t *wad_lump_entry;
    Lump             *lump;
    
    wad_lump_entry = lumps + i;
    lump = array_index_fast(wad->lumps, previous_lump_count - 1 + i);

    memcpy(lump->name, wad_lump_entry->name, sizeof(wad_lump_entry->name));

    if (wad->source == WAD_SOURCE_LUMP) {
      lump->ns = LUMP_NAMESPACE_DEMOS;
    }
    else {
      lump->ns = LUMP_NAMESPACE_GLOBAL;
    }

    lump->wad = wad;
    lump->size = cble32(wad_lump_entry->size);

    if (!d2k_malloc((void **)&lump->data, lump->size, status)) {
      file_close(fobj, NULL);
      d2k_free(lumps);
      array_set_size(wad->lumps, previous_lump_count, NULL);
      return false;
    }

    if (!file_seek(fobj, wad_lump_entry->filepos, SEEK_SET)) {
      file_close(fobj, NULL);
      d2k_free(lumps);
      array_set_size(wad->lumps, previous_lump_count, NULL);
      return false;
    }

    if (file_read_raw(fobj, lump->data, lump->size, status)) {
      file_close(fobj, NULL);
      d2k_free(lumps);
      array_set_size(wad->lumps, previous_lump_count, NULL);
      return false;
    }

    file_close(fobj, NULL);
    free(lumps);

    // IWAD file used as recource PWAD must not override TEXTURE1 or PNAMES
    if ((source != WAD_SOURCE_IWAD) &&
        ((source == WAD_SOURCE_LUMP) ||
         (!strncmp(header.identification, "IWAD" 4))) &&
        ((!strnicmp(wad_lump_entry->name, "TEXTURE1", 8)) ||
         (!strnicmp(wad_lump_entry->name, "PNAMES", 6)))) {
      strncpy(lump->name, "-IGNORE-", 8);
    }
  }
}

Wad* WadNew(WadSource source, GPtrArray *resource_file_paths) {
  Wad *wad = d2k_malloc(sizeof(Wad));
  size_t loaded_resources_count = WadInit(wad, source, resource_file_paths);

  if (loaded_resources_count != resource_file_paths->len) {
    d2k_error("Building new WAD failed\n");
    WadFree(wad);
    d2k_free(wad);
    return NULL;
  }

  return wad;
}

static void free_lump(gpointer data) {
  Lump *lump = (Lump *)data;

  LumpFree(lump);
}

size_t WadInit(Wad *wad, WadSource source, GPtrArray *resource_file_paths) {
  size_t loaded_resources_count = 0;

  wad->source = source;

  for (size_t i = 0; i < resource_file_paths->len; i++) {
    const char *resource_file_path = g_ptr_array_index(resource_file_paths, i);

    if (!load_resource(wad, source, resource_file_path)) {
      d2k_warn("Loading resource %s failed\n", resource_file_path);
    }
    else {
      loaded_resources_count++;
    }
  }

  coalesce_marked_lump("S_START", "S_END", LUMP_NAMESPACE_SPRITES);
  coalesce_marked_lump("F_START", "F_END", LUMP_NAMESPACE_FLATS);
  coalesce_marked_lump("C_START", "C_END", LUMP_NAMESPACE_COLORMAPS);
  coalesce_marked_lump("B_START", "B_END", LUMP_NAMESPACE_PRBOOM);
  coalesce_marked_lump("HI_START", "HI_END", LUMP_NAMESPACE_HIRES);

  for (size_t i = 0; i < LUMP_NAMESPACE_MAX + 1; i++) {
    GHashTable *ht = g_hash_table_new(g_direct_hash, g_direct_equal);

    for (size_t j = 0; j < wad->lumps->len; j++) {
      Lump *lump = &g_array_index(wad->lumps, Lump, j);

      if ((i != LUMP_NAMESPACE_MAX) && (lump->ns != i)) {
        continue;
      }

      g_hash_table_insert(ht, lump->name, lump);
    }

    wad->lump_lookup[i] = ht;
  }

  return loaded_resources_count;
}

void WadClear(Wad *wad) {
  for (size_t i = 0; i < LUMP_NAMESPACE_MAX + 1; i++) {
    g_hash_table_destroy(wad->lump_lookup[i]);
    wad->lump_lookup[i] = NULL;
  }

  g_array_free(wad->lumps, true);
  wad->lumps = NULL;
}

Lump* WadLookupLump(Wad *wad, const char *lump_name) {
  return g_hash_table_lookup(wad->lump_lookup[LUMP_NAMESPACE_MAX], lump_name);
}

Lump* WadLookupLumpNS(Wad *wad, const char *lump_name, LumpNamespace ns) {
  return g_hash_table_lookup(wad->lump_lookup[ns], lump_name);
}

/* vi: set et ts=2 sw=2: */
