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

#ifndef D2K_THINKER_H__
#define D2K_THINKER_H__

typedef void (*D2KActionFunction)();

/* I don't think these are necessary. */

#if 0
typedef void (*D2KActionFunctionV)();
typedef void (*D2KActionFunctionP1)(void *);
typedef void (*D2KActionFunctionP2)(void *, void *);
#endif

typedef struct D2KThinkerStruct {
  struct  D2KThinkerStruct *prev;
  struct  D2KThinkerStruct *next;
  D2KActionFunction         function;
  struct  D2KThinkerStruct *cnext;
  struct  D2KThinkerStruct *cprev;
  unsigned int references;
} D2KThinker;

#endif

/* vi: set et ts=2 sw=2: */
