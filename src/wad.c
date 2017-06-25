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

typedef struct {
  char identification[4]; // Should be "IWAD" or "PWAD".
  int  numlumps;
  int  infotableofs;
} wad_header_t;

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
                                            LumpNamespace ns) {
  size_t marked_count = 0;
  bool   in_marked = false;
  Lump   start_marker;
  Lump   end_marker;

  for (size_t i = 0; i < wad->lumps->len; i++) {
    Lump *lump = &g_array_index(wad->lumps, Lump, i);

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
      g_array_remove_index(wad->lumps, i);
      g_array_insert_val(wad->lumps, marked_count, lump);
      marked_count++;
    }
  }

  if (!marked_count) {
    return
  }

  strncpy(start_marker.name, start_marker_name, 8);
  start_marker.ns = LUMP_NAMESPACE_GLOBAL;
  start_marker.wad = wad;
  start_marker.size = 0;
  start_marker.data = NULL;

  strncpy(end_marker.name, end_marker_name, 8);
  end_marker.ns = LUMP_NAMESPACE_GLOBAL;
  end_marker.wad = wad;
  end_marker.size = 0;
  end_marker.data = NULL;

  g_array_prepend_val(wad->lumps, start_marker);
  g_array_insert_val(wad->lumps, marked_count + 1, end_marker);
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

static bool load_resource(WadSource source, const char *resource_file_path) {
  wad_header_t      header;
  wad_lump_entry_t *lump_p;
  unsigned          i;
  int               length;
  int               startlump;
  wad_lump_entry_t *fileinfo;
  wad_lump_entry_t *fileinfo2free = NULL; //killough
  wad_lump_entry_t  singleinfo;
  int               flags = 0;
  int               fh;

  fh = g_open(resource_file_path, O_RDONLY, S_IRUSR);

  if (fh == -1) {
    d2k_error("Error opening resource file %s\n", resource_file_path);
    return false;
  }

  startlump = numlumps;

  switch (source) {
    case WAD_SOURCE_IWAD:
    case WAD_SOURCE_PWAD:
      if (read(fh, &header, sizeof(header))) {
        d2k_error("Error reading WAD header from %s\n", resource_file_path);
        close(fh);
        return false;
      }
      if (strncmp(header.identification, "IWAD", 4) &&
          strncmp(header.identification, "PWAD", 4)) {
        d2k_error("WAD %s doesn't have IWAD or PWAD ID\n", resource_file_path);
        close(fh);
        return false;
      }
      header.numlumps = GINT_FROM_LE(header.numlumps);
      header.infotableofs = GINT_FROM_LE(header.infotableofs);
      lump_directory = d2k_calloc(header.numlumps, sizeof(wad_lump_entry_t));
      if (lseek(fh, header.infotableofs, SEEK_SET) == -1) {
        d2k_error("Error seeking to lump directory in WAD %s\n",
          resource_file_path
        );
        close(fh);
        d2k_free(lump_directory);
        return false;
      }
      if (read(fh, lump_directory,
                   header.numlumps * sizeof(wad_lump_entry_t))) {
        d2k_error("Error reading lump directory from WAD %s\n",
          resource_file_path
        );
        close(fh);
        d2k_free(lump_directory);
        return false;
      }
      lump_count = header.numlumps;
    case WAD_SOURCE_LUMP:
      lump_directory = d2k_calloc(1, sizeof(wad_lump_entry_t));
      lump_directory->filepos = 0;
      lump_directory->size = lseek(fh, 0, SEEK_END);
      if (lump_directory->size == -1) {
        d2k_error("Error seeking to end of lump %s\n", resource_file_path);
        close(fh);
        d2k_free(lump_directory);
        return false;
      }
      basename = g_path_get_basename(resource_file_path);
      for (lump_name_length = 0; i < sizeof(lump_directory->name); i++) {
        char c = basename[lump_name_length];

        if ((c == '.') || (c == '\0')) {
          break;
        }
      }
      memcpy(lump_directory->name, basename, lump_name_length);
      lump_count = 1;
    default:
      d2k_error("Invalid WAD source %d\n", source);
      g_close(fh);
      return false;
  }

  previous_lump_count = wad->lumps->len;
  g_array_set_size(wad->lumps, wad->lumps->len + lump_count);

  for (size_t i = 0; i < lump_count; i++) {
    Lump   *lump = g_array_index(wad->lumps, previous_lump_count - 1 + i);
    wad_lump_entry_t *wad_lump_entry = lump_directory + i;

    strncpy(lump->name, wad_lump_entry->name, sizeof(wad_lump_entry->name));

    if (wad->source == WAD_SOURCE_LUMP) {
      lump->ns = LUMP_NAMESPACE_DEMOS;
    }
    else {
      lump->ns = LUMP_NAMESPACE_GLOBAL;
    }
    lump->wad = wad;
    lump->size = GUINT_FROM_LE(wad_lump_entry->size);

    lump->data = d2k_malloc(lump->size);
    if (lseek(fh, wad_lump_entry->filepos, SEEK_SET)) {
      d2k_error("Error seeking to %s in resource %s\n",
        lump->name,
        resource_file_path
      );
      g_close(fh);
      free(lump_directory);
      g_array_set_size(wad->lumps, previous_lump_count);
      return false;
    }

    if (read(fh, lump->data, sizeof(lump->size))) {
      d2k_error("Error lump data from lump %s in resource %s\n",
        lump->name,
        resource_file_path
      );
      g_close(fh);
      free(lump_directory);
      g_array_set_size(wad->lumps, previous_lump_count);
      return false;
    }

    // IWAD file used as recource PWAD must not override TEXTURE1 or PNAMES
    if ((source != WAD_SOURCE_IWAD) &&
        ((source == WAD_SOURCE_LUMP) ||
         (!strncmp(header.identification, "IWAD" 4))) &&
        ((!strnicmp(wad_lump_entry->name, "TEXTURE1", 8)) ||
         (!strnicmp(wad_lump_entry->name, "PNAMES", 6)))) {
      strncpy(lump->name, "-IGNORE-", 8);
    }
  }

  free(lump_directory);
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
