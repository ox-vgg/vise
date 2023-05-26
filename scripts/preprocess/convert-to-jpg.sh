#!/bin/sh

# A script to sanitise images before indexing them using VISE
#
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# Date  : 2023-05-26

SRC="/data/original/"
TGT="/data/jpg/"
FILENAME_LIST="/data/image-filelist.txt"

LOGDIR="${TGT}log/"
if [ ! -d $LOGDIR ]; then
    mkdir $LOGDIR
fi

today=$( date +%Y%m%d )
ERR_LOG="${LOGDIR}error-log-${today}.txt"
ERR_FILELIST="${LOGDIR}error-filelist-${today}.txt"
OK_LOG="${LOGDIR}success-log-${today}.txt"

cd $SRC
line_number=0
cat $FILENAME_LIST | while read source_file; do
    line_number=$((line_number+1))
    subdir=`dirname "${source_file}" 2>&1`
    targetdir="${TGT}${subdir}/"
    dir_create_status=`mkdir -p "${targetdir}" 2>&1`
    if [ $? -ne 0 ]; then
	      echo "[${line_number}] ${source_file},DIR-CREATE-FAILED" >> $ERR_LOG 2>&1
        echo "${line_number},${source_file}" >> $ERR_FILELIST 2>&1
        continue
    fi
    srcfile="${SRC}${source_file}"
    tgtfile="${TGT}${source_file%.*}.jpg"
    if [ -f "${tgtfile}" ]; then
        continue
    fi

    echo "[${line_number}] ${srcfile}"
    img_conv_status=$(/ssd/adutta/dep/vise2/bin/convert "${srcfile}[0]" -quiet -colorspace sRGB -type truecolor -quality 80 -resize 1024x1024\> "${tgtfile}" 2>&1)
    retval=$?
    if [ $retval -ne 0 ]; then
	      echo "==== [${line_number}] ${source_file},${retval} ====" >> $ERR_LOG 2>&1
        echo "${img_conv_status}" >> $ERR_LOG 2>&1
        echo "${line_number},${source_file}" >> $ERR_FILELIST 2>&1
    else
	      echo "${line_number},${source_file}" >> $OK_LOG 2>&1
    fi
done
