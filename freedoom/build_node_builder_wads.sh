#!/bin/bash

set -e

BASE_WAD_NAME=$(basename $1 .wad)

zdbsp ${BASE_WAD_NAME}.wad -t             -o ${BASE_WAD_NAME}-zdbsp-normal-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t       -X    -o ${BASE_WAD_NAME}-zdbsp-extended-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t          -z -o ${BASE_WAD_NAME}-zdbsp-normal-compressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t       -X -z -o ${BASE_WAD_NAME}-zdbsp-extended-compressed.wad

zdbsp ${BASE_WAD_NAME}.wad -t -g          -o ${BASE_WAD_NAME}-zdbsp-gl-normal-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g    -X    -o ${BASE_WAD_NAME}-zdbsp-gl-extended-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g       -z -o ${BASE_WAD_NAME}-zdbsp-gl-normal-compressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g    -X -z -o ${BASE_WAD_NAME}-zdbsp-gl-extended-compressed.wad

zdbsp ${BASE_WAD_NAME}.wad -t -g -G       -o ${BASE_WAD_NAME}-zdbsp-glmatching-normal-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -G -X    -o ${BASE_WAD_NAME}-zdbsp-glmatching-extended-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -G    -z -o ${BASE_WAD_NAME}-zdbsp-glmatching-normal-compressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -G -X -z -o ${BASE_WAD_NAME}-zdbsp-glmatching-extended-compressed.wad

zdbsp ${BASE_WAD_NAME}.wad -t -g -x       -o ${BASE_WAD_NAME}-zdbsp-glonly-normal-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -x -X    -o ${BASE_WAD_NAME}-zdbsp-glonly-extended-uncompressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -x    -z -o ${BASE_WAD_NAME}-zdbsp-glonly-normal-compressed.wad
zdbsp ${BASE_WAD_NAME}.wad -t -g -x -X -z -o ${BASE_WAD_NAME}-zdbsp-glonly-extended-compressed.wad

zdbsp ${BASE_WAD_NAME}.wad -t -g --gl-v5  -o ${BASE_WAD_NAME}-zdbsp-glv5-normal-uncompressed.wad

glbsp -q -v1 -xp ${BASE_WAD_NAME}.wad -o ${BASE_WAD_NAME}-glbsp-glv1.wad
glbsp -q -v2 -xp ${BASE_WAD_NAME}.wad -o ${BASE_WAD_NAME}-glbsp-glv2.wad
glbsp -q -v3 -xp ${BASE_WAD_NAME}.wad -o ${BASE_WAD_NAME}-glbsp-glv3.wad
glbsp -q -v5 -xp ${BASE_WAD_NAME}.wad -o ${BASE_WAD_NAME}-glbsp-glv5.wad
