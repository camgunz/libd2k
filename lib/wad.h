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

#ifndef WAD_H__
#define WAD_H__

typedef enum {
  LUMP_NAMESPACE_GLOBAL,
  LUMP_NAMESPACE_SPRITES,
  LUMP_NAMESPACE_FLATS,
  LUMP_NAMESPACE_COLORMAPS,
  LUMP_NAMESPACE_PRBOOM,
  LUMP_NAMESPACE_DEMOS,
  LUMP_NAMESPACE_HIRES,
  LUMP_NAMESPACE_MAX
} LumpNamespace;

typedef enum {
  WAD_SOURCE_IWAD,
  WAD_SOURCE_PWAD,
  WAD_SOURCE_LUMP,
} WadSource;

typedef struct WadStruct {
  WadSource  source;
  Array     *lumps;
  Table     *lump_lookup[LUMP_NAMESPACE_MAX + 1];
} Wad;

typedef struct LumpStruct {
  char           name[9];
  LumpNamespace  ns;
  Wad           *wad;
  size_t         size;
  const char    *data;
} Lump;

Wad*  WadNew(WadSource source, PArray *resource_file_paths);
void  WadInit(Wad *wad, WadSource source, PArray *resource_file_paths);
void  WadClear(Wad *wad);
Lump* WadLookupLump(Wad *wad, const char *lump_name);
Lump* WadLookupLumpNS(Wad *wad, const char *lump_name, LumpNamespace ns);

#endif

/* vi: set et ts=2 sw=2: */
