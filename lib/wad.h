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

#ifndef D2K_WAD_H__
#define D2K_WAD_H__

#define NSKEY_SIZE (sizeof(D2KLumpNamespace) + 8)

enum {
  D2K_WAD_ERROR_INVALID_LUMP_DIRECTORY_LOCATION_IN_WAD_HEADER,
  D2K_WAD_ERROR_INVALID_WAD_SOURCE,
};

typedef enum {
  D2K_LUMP_NAMESPACE_GLOBAL,
  D2K_LUMP_NAMESPACE_SPRITES,
  D2K_LUMP_NAMESPACE_FLATS,
  D2K_LUMP_NAMESPACE_COLORMAPS,
  D2K_LUMP_NAMESPACE_PRBOOM,
  D2K_LUMP_NAMESPACE_DEMOS,
  D2K_LUMP_NAMESPACE_HIRES,
  D2K_LUMP_NAMESPACE_MAX
} D2KLumpNamespace;

typedef enum {
  WAD_SOURCE_IWAD,
  WAD_SOURCE_PWAD,
  WAD_SOURCE_LUMP,
} D2KWadSource;

typedef struct D2KWadStruct {
  D2KWadSource source;
  Buffer       data;
  Array        lumps;
} D2KWad;

typedef struct D2KLumpStruct {
  D2KLumpNamespace  ns;
  D2KWad           *wad;
  Slice             data;
  char              name[9];
  char              nskey[NSKEY_SIZE];
} D2KLump;

typedef struct D2KLumpDirectoryStruct {
  PArray *lumps;
  Table   lookups[2];
} D2KLumpDirectory;

bool d2k_wad_init_from_path(D2KWad *wad, D2KWadSource source, Path *path,
                                                              Status *status);
bool d2k_wad_init_from_data(D2KWad *wad, D2KWadSource source, Buffer *buffer,
                                                              Status *status);
bool d2k_wad_init_from_lump_file_data(D2KWad *wad, SSlice *lump_name,
                                                   Buffer *lump_data,
                                                   Status *status);
bool d2k_wad_init_from_lump_file(D2KWad *wad, Path *lump_file_path,
                                              Status *status);

bool d2k_lump_directory_init(D2KLumpDirectory *lump_directory, PArray *wads,
                                                               Status *status);
void d2k_lump_directory_free(D2KLumpDirectory *lump_directory, Status *status);
bool d2k_lump_directory_lookup(D2KLumpDirectory *lump_directory,
                               const char *lump_name,
                               Lump **lump,
                               Status *status);
bool d2k_lump_directory_lookup_ns(D2KLumpDirectory *lump_directory,
                                  const char *lump_name,
                                  D2KLumpNamespace ns,
                                  Status *status);

#endif

/* vi: set et ts=2 sw=2: */
