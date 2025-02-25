#!/usr/bin/env bash -x

FILES="workrave-suspended.svg workrave-quiet.svg workrave-normal.svg"

for f in $FILES
do
    SVG_FILENAME=$f
    ICONNAME=`basename ${SVG_FILENAME} .svg`

    ICON_PNGS=()
    mkdir -p tmppng
    for size in 256 128 64 48 32 24 16
    do
        PNG_FILENAME="tmppng/${ICONNAME}_${size}.png"
        #svg2png ${SVG_FILENAME} ${PNG_FILENAME} -w $size
        #rsvg-convert ${SVG_FILENAME} -o ${PNG_FILENAME} -w $size -h $size
        batik-rasterizer ${SVG_FILENAME} -d ${PNG_FILENAME} -w $size -h $size
        ICON_PNGS+=("${PNG_FILENAME}")
    done
    echo ${ICON_PNGS[@]}
    magick convert ${ICON_PNGS[@]} windows/${ICONNAME}.ico
    rm -rf tmppng
done
