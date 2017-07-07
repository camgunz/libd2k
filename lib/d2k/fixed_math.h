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

#ifndef D2K_FIXEDMATH_H__
#define D2K_FIXEDMATH_H__

/*
 * Fixed point, 32bit as 16.16.
 */

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

typedef int32_t D2KFixedPoint;

static inline D2KFixedPoint d2k_fixed_mul(D2KFixedPoint a, D2KFixedPoint b) {
  return (D2KFixedPoint)((int64_t) a * b >> FRACBITS);
}

static inline D2KFixedPoint d2k_fixed_div(D2KFixedPoint a, D2KFixedPoint b) {
  return (abs(a) >> 14) >= abs(b) ? ((a ^ b) >> 31) ^ INT_MAX :
    (D2KFixedPoint)(((int64_t) a << FRACBITS) / b
  );
}

static inline D2KFixedPoint d2k_fixed_mod(D2KFixedPoint a, D2KFixedPoint b) {
  if (b & (b - 1)) {
    D2KFixedPoint r = a % b;

    return ((r < 0) ? r + b : r);
  }

  return (a & (b - 1));
}

#endif

/* vi: set et ts=2 sw=2: */
