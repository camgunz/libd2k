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

#ifndef D2K_MAP_OBJECT_H__
#define D2K_MAP_OBJECT_H__

#include "d2k/sprite.h"
#include "d2k/thinker.h"
#include "d2k/map_object_info.h"
#include "d2k/map_object_state.h"
#include "d2k/map_object_type.h"

struct D2KPlayer;

typedef struct D2KMapObjectStruct {
    D2KThinker                     thinker;
    D2KFixedPoint                  x;
    D2KFixedPoint                  y;
    D2KFixedPoint                  z;
    struct D2KMapObjectStruct     *snext;
    struct D2KMapObjectStruct    **sprev;
    D2KAngle                       angle;
    D2KSpriteNum                   sprite;
    int                            frame;
    struct D2KMapObjectStruct     *bnext;
    struct D2KMapObjectStruct    **bprev;
    struct D2KSubSectorStruct     *sub_sector;
    D2KFixedPoint                  floorz;
    D2KFixedPoint                  ceilingz;
    D2KFixedPoint                  dropoffz;
    D2KFixedPoint                  radius;
    D2KFixedPoint                  height;
    D2KFixedPoint                  momx;
    D2KFixedPoint                  momy;
    D2KFixedPoint                  momz;
    int                            valid_count;
    D2KMapObjectType               type;
    D2KMapObjectInfo              *info;
    int                            tics;
    D2KMapObjectState             *state;
    uint64_t                       flags;
    int                            intflags;
    int                            health;
    short                          movedir;
    short                          movecount;
    short                          strafecount;
    struct D2KMapObjectStruct     *target;
    short                          reactiontime;
    short                          threshold;
    short                          pursuecount;
    short                          gear;
    struct D2KPlayer              *player;
    short                          lastlook;
    D2KMapThing                    spawnpoint;
    struct D2KMapObjectStruct     *tracer;
    struct D2KMapObjectStruct     *lastenemy;
    int                            friction;
    int                            move_factor
    struct D2KMapSectorNodeStruct *touching_sectorlist;
    D2KFixedPoint                  prev_x;
    D2KFixedPoint                  prev_y;
    D2KFixedPoint                  prev_z;
    D2KAngle                       pitch;
    int                            index;
    short                          patch_width;
    int                            iden_nums;
    D2KFixedPoint                  pad;
} D2KMapObject;

#endif

/* vi: set et ts=2 sw=2: */
