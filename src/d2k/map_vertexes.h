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

#ifndef D2K_MAP_VERTEXES_H__
#define D2K_MAP_VERTEXES_H__

#include "d2k/map_loader.h"

enum {
  D2K_MAP_VERTEXES_MALFORMED_LUMP = 1,
  D2K_MAP_VERTEXES_MALFORMED_GL_VERT_LUMP,
};

bool d2k_map_loader_load_vertexes(D2KMapLoader *map_loader, Status *status);

#endif

/* vi: set et ts=2 sw=2: */
