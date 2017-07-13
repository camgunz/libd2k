#!/bin/bash

set -e

zdbsp freedoom2.wad -t             -o freedoom2-zdbsp-normal-uncompressed.wad
zdbsp freedoom2.wad -t       -X    -o freedoom2-zdbsp-extended-uncompressed.wad
zdbsp freedoom2.wad -t          -z -o freedoom2-zdbsp-normal-compressed.wad
zdbsp freedoom2.wad -t       -X -z -o freedoom2-zdbsp-extended-compressed.wad

zdbsp freedoom2.wad -t -g          -o freedoom2-zdbsp-gl-normal-uncompressed.wad
zdbsp freedoom2.wad -t -g    -X    -o freedoom2-zdbsp-gl-extended-uncompressed.wad
zdbsp freedoom2.wad -t -g       -z -o freedoom2-zdbsp-gl-normal-compressed.wad
zdbsp freedoom2.wad -t -g    -X -z -o freedoom2-zdbsp-gl-extended-compressed.wad

zdbsp freedoom2.wad -t -g -G       -o freedoom2-zdbsp-glmatching-normal-uncompressed.wad
zdbsp freedoom2.wad -t -g -G -X    -o freedoom2-zdbsp-glmatching-extended-uncompressed.wad
zdbsp freedoom2.wad -t -g -G    -z -o freedoom2-zdbsp-glmatching-normal-compressed.wad
zdbsp freedoom2.wad -t -g -G -X -z -o freedoom2-zdbsp-glmatching-extended-compressed.wad

zdbsp freedoom2.wad -t -g -x       -o freedoom2-zdbsp-glonly-normal-uncompressed.wad
zdbsp freedoom2.wad -t -g -x -X    -o freedoom2-zdbsp-glonly-extended-uncompressed.wad
zdbsp freedoom2.wad -t -g -x    -z -o freedoom2-zdbsp-glonly-normal-compressed.wad
zdbsp freedoom2.wad -t -g -x -X -z -o freedoom2-zdbsp-glonly-extended-compressed.wad

zdbsp freedoom2.wad -t -g --gl-v5  -o freedoom2-zdbsp-glv5-normal-uncompressed.wad

glbsp -q -v1 -xp freedoom2.wad -o freedoom2-glbsp-glv1.wad
glbsp -q -v2 -xp freedoom2.wad -o freedoom2-glbsp-glv2.wad
glbsp -q -v3 -xp freedoom2.wad -o freedoom2-glbsp-glv3.wad
glbsp -q -v5 -xp freedoom2.wad -o freedoom2-glbsp-glv5.wad
