#include <setjmp.h>

#include "d2k.h"
#include "d2k_test.h"

#include <cmocka.h>

#define FREEDOOM2_WAD_PATH "/home/charlie/wads/freedoom2.wad"

void test_map(void **state) {
  Status status;
  Path path;
  D2KWad freedoom2;
  PArray wads;
  D2KLumpDirectory lump_directory;
  D2KMap map;
  char map_name[6];

  status_init(&status);
  parray_init(&wads);

  assert_true(path_init_non_local_from_cstr(
    &path,
    FREEDOOM2_WAD_PATH,
    &status
  ));

  assert_true(d2k_wad_init_from_path(
    &freedoom2,
    D2K_WAD_SOURCE_IWAD,
    &path,
    &status,
  ));

  assert_true(parray_append(&wads, (void *)&freedoom2, &status));

  assert_true(d2k_lump_directory_init(&lump_directory, &wads, &status));

  d2k_map_init(&map);

  for (size_t i = 0; i < 32; i++) {
    D2KMapLoader map_loader;
    char map_name[6];

    sprintf(map_name, "MAP%1zu", i);

    assert_true(d2k_map_loader_load_map(
      &map_loader,
      &map,
      &lump_directory,
      map_name,
      status
    ));

    d2k_map_clear(&map);
  }
}

/* vi: set et ts=2 sw=2: */
