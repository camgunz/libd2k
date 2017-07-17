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

#ifndef D2K_MAP_PROBLEM_H__
#define D2K_MAP_PROBLEM_H__

typedef struct D2KMapProblemTruncatedBlockmapHeaderStruct {
} D2KMapProblemTruncatedBlockmapHeader;

typedef struct D2KMapProblemNegativeBlockmapHeightStruct{
  int16_t blockmap_height;
} D2KMapProblemNegativeBlockmapHeight;

typedef struct D2KMapProblemNegativeBlockmapWidthStruct{
  int16_t blockmap_width;
} D2KMapProblemNegativeBlockmapWidth;

typedef struct D2KMapProblemStruct{
} D2KMapProblem;

typedef struct D2KMapProblemStruct {
  int type;
  union {
    D2KMapProblemTruncatedBlockmapHeader truncated_blockmap_header;
  } as;
} D2KMapProblem;


#endif

/* vi: set et ts=2 sw=2: */
