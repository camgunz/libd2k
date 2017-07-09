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

#ifndef D2K_ANGLE_H__
#define D2K_ANGLE_H__

struct D2KLumpDirectoryStruct;

#define FINEANGLES 8192
#define FINEMASK   (FINEANGLES - 1)

#define ANGLETOFINESHIFT 19 /* 0x100000000 to 0x2000 */

/* Binary Angle Measument, BAM. */
#define ANG45     0x20000000
#define ANG90     0x40000000
#define ANG180    0x80000000
#define ANG270    0xc0000000
#define ANG1      (ANG45 / 45)
#define ANGLE_MAX 0xFFFFFFFF

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SLOPERANGE 2048
#define SLOPEBITS    11
#define DBITS      (FRACBITS - SLOPEBITS)

#define SINE_COUNT             (5 * FINEANGLES / 4) /* 10240 */
#define COSINE_COUNT           FINEANGLES           /*  8192 */
#define TANGENT_COUNT          (FINEANGLES / 2)     /*  4096 */

/* The +1 size is to handle the case when x==y without additional checking. */
#define TANGENT_TO_ANGLE_COUNT (SLOPERANGE + 1)     /*  2049 */

enum {
  D2K_ANGLE_INVALID_SINE_TABLE = 1,
  D2K_ANGLE_INVALID_TANGENT_TABLE,
  D2K_ANGLE_INVALID_TANGENT_TO_ANGLE_TABLE,
};

typedef uint32_t D2KAngle;

/* Utility function, called by R_PointToAngle. */
typedef int (*slope_div_fn)(uint32_t num, uint32_t den);

int  d2k_slope_div(uint32_t num, uint32_t den);
int  d2k_slope_div_ex(unsigned int num, unsigned int den);
bool d2k_angle_load_trig_tables(struct D2KLumpDirectoryStruct *lump_directory,
                                D2KFixedPoint *finesine,
                                D2KFixedPoint *finecosine,
                                D2KFixedPoint *finetangent,
                                D2KAngle *tantoangle,
                                Status *status);

#endif

/* vi: set et ts=2 sw=2: */
