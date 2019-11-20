#!/bin/sh

PNAME="ox5k"
upload_all_files_in_folder()
{
    FOLDER=$1
    for jpg_file in $FOLDER/*.jpg; do
        fn=`basename $jpg_file`
        echo "Sending file ${jpg_file} ..."
        curl --silent -X PUT --data-binary @"${jpg_file}" "http://localhost:9669/${PNAME}/${fn}"
    done
}

#curl -X DELETE http://localhost:9669/ox5k
curl -X PUT http://localhost:9669/ox5k
#upload_all_files_in_folder /home/tlm/data/dataset/ox5k/test_200
curl -X POST http://localhost:9669/ox5k/_conf --data-binary @/home/tlm/code/vise2/src/conf/relja_retrival_conf.txt
