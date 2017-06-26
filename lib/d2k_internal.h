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

#ifndef D2K_INTERNAL_H__
#define D2K_INTERNAL_H__

#ifdef __GNUC__

#ifdef __MINGW32__
#define PRINTF_DECL(x, y) __attribute__((format(gnu_printf, x, y)))
#else
#define PRINTF_DECL(x, y) __attribute__((format(printf, x, y)))
#endif
#else
#define __attribute__(x)
#define PRINTF_DECL(x, y)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define __STDC_FORMAT_MACROS

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup

static inline void* d2k_malloc(size_t count, size_t size) {
	if (count && size && ((SIZE_MAX / count) < size)) {
		errno = ENOMEM;
    return (NULL);
  }

  return g_malloc(count * size);
}

static inline void* d2k_calloc(size_t count, size_t size) {
	if (count && size && ((SIZE_MAX / count) < size)) {
		errno = ENOMEM;
    return (NULL);
  }

  return g_malloc0(count * size);
}

static inline void* d2k_realloc(void *ptr, size_t count, size_t size) {
	if (count && size && ((SIZE_MAX / count) < size)) {
		errno = ENOMEM;
    return (NULL);
  }

  return g_realloc0(ptr, count * size);
}

static inline void d2k_free(void *ptr) {
  g_free(ptr);
}

static inline void* d2k_memdup(const void *ptr, size_t count) {
  return g_memdup(ptr, count);
}

static inline char* d2k_strdup(const char *s) {
  return g_strdup(s);
}

static inline char* d2k_strndup(const char *s, size_t n) {
  return g_strndup(s, n);
}

#endif

/* vi: set et ts=2 sw=2: */
