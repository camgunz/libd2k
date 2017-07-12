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

#ifndef D2K_ALLOC_H__
#define D2K_ALLOC_H__

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef strndup

static inline bool _d2k_malloc(void **ptr, size_t count, size_t size,
                                                         Status *status) {
  *ptr = cbmalloc(count, size);

  if (!(*ptr)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_malloc
#define d2k_malloc _d2k_malloc
#endif

static inline bool _d2k_calloc(void **ptr, size_t count, size_t size,
                                                         Status *status) {
  *ptr = cbcalloc(count, size);

  if (!(*ptr)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_calloc
#define d2k_calloc _d2k_calloc
#endif

static inline bool _d2k_realloc(void **ptr, size_t count, size_t size,
                                                          Status *status) {
  *ptr = cbrealloc(*ptr, count, size);

  if (!(*ptr)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_realloc
#define d2k_realloc _d2k_realloc
#endif

static inline void _d2k_free(void *ptr) {
  cbfree(ptr);
}

#ifndef d2k_free
#define d2k_free _d2k_free
#endif

static inline bool _d2k_memdup(void **newptr, const void *ptr,
                                              size_t count,
                                              Status *status) {
  *newptr = cbmemdup(ptr, count);

  if (!(*newptr)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_memdup
#define d2k_memdup _d2k_memdup
#endif

static inline bool _d2k_strdup(char **news, const char *s, Status *status) {
  *news = cbstrdup(s);

  if (!(*news)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_strdup
#define d2k_strdup _d2k_strdup
#endif

static inline bool _d2k_strndup(char **news, const char *s, size_t count,
                                                            Status *status) {
  *news = cbstrndup(s, count);

  if (!(*news)) {
    return alloc_failure(status);
  }

  return true;
}

#ifndef d2k_strndup
#define d2k_strndup _d2k_strndup
#endif

#endif

/* vi: set et ts=2 sw=2: */
