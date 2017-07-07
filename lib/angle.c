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

#include "d2k/internal.h"

#include "d2k/angle.h"

int d2k_slope_div(uint32_t num, uint32_t den) {
  uint32_t ans;

  if (den < 512) {
    return SLOPERANGE;
  }

  ans = (num << 3) / (den >> 8);

  return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

// [crispy] catch SlopeDiv overflows, only used in rendering
int d2k_slope_div_ex(unsigned int num, unsigned int den) {
  uint64_t ans;

  if (den < 512) {
    return SLOPERANGE;
  }

  ans = ((uint64_t)num << 3) / (den >> 8);

  return ans <= SLOPERANGE ? (int)ans : SLOPERANGE;
}

void d2k_angle_load_trig_tables(D2KLumpDirectory *lump_directory,
                                D2KFixedPoint *finesine,
                                D2KFixedPoint *finecosine,
                                D2KFixedPoint *finetangent,
                                D2KAngle *tantoangle,
                                Status *status) {
  D2KLump *lump = NULL;
  size_t n;

  if (!d2k_lump_directory_lookup_ns(lump_directory, "SINETABL",
                                                    D2K_LUMP_NAMESPACE_PRBOOM,
                                                    &lump,
                                                    status)) {
    return false;
  }

  if (lump->data.len != (sizeof(D2KFixedPoint) * SINE_COUNT)) {
    return invalid_sine_table(status);
  }

  if (!slice_read(&lump->data, 0, lump->data.len, (void *)finesine, status)) {
    return false;
  }

  cbmemmove(finecosine, finesine + 2048, sizeof(D2KFixedPoint) * COSINE_COUNT);

  if (!d2k_lump_directory_lookup_ns(lump_directory, "TANGTABL",
                                                    D2K_LUMP_NAMESPACE_PRBOOM,
                                                    &lump,
                                                    status)) {
    return false;
  }

  if (lump->data.len != (sizeof(D2KFixedPoint) * TANGENT_COUNT)) {
    return invalid_tangent_table(status);
  }

  if (!slice_read(&lump->data, 0, lump->data.len, (void *)finetangent,
                                                  status)) {
    return false;
  }

  if (!d2k_lump_directory_lookup_ns(lump_directory, "TANTOANG",
                                                    D2K_LUMP_NAMESPACE_PRBOOM,
                                                    &lump,
                                                    status)) {
    return false;
  }

  if (lump->data.len != (sizeof(D2KAngle) * TANGENT_TO_ANGLE_COUNT)) {
    return invalid_tangent_to_angletable(status);
  }

  if (!slice_read(&lump->data, 0, lump->data.len, (void *)tantoangle,
                                                  status)) {
    return false;
  }

#ifdef WORDS_BIGENDIAN
  for (size_t i = 0; i < SINE_COUNT; i++) {
    finesine[i] = cble32(finesine[i]);
  }

  for (size_t i = 0; i < TANGENT_COUNT; i++) {
    finetangent[i] = cble32(finetangent[i]);
  }

  for (size_t i = 0; i < TANGENT_TO_ANGLE_COUNT; i++) {
    tantoangle[i] = cble32(tantoangle[i]);
  }
#endif
}

/* vi: set et ts=2 sw=2: */
