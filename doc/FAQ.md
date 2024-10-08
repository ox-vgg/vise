Here are some of the Frequently Asked Questions (FAQ) by our users. If you would like to add a new entry to this page or update an existing entry, you can either post an issue or write to [adutta-REMOVE@robots.ox.ac.uk](mailto:adutta-REMOVE@robots.ox.ac.uk) 

[[_TOC_]]

## What computing resources are required to index 1 million images?
We created a visual search engine based on 1 million images. All the images in this dataset had a maximum image resolution of `1024x1024`. 

The project configuration file is shown below:
```
$ cat conf.txt
bow_descriptor_count=100000000
bow_cluster_count=100000
resize_dimension=-1
sift_scale_3=true
use_root_sift=true
nthread-indexing=64
hamm_embedding_bits=64
cluster_num_iteration=30
preset_conf_id=preset_conf_manual
search_engine=relja_retrival
```

Here are the specifications of the server that was used for creating the index:
```
$ lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                64
Thread(s) per core:    2
Core(s) per socket:    16
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 85
Model name:            Intel(R) Xeon(R) Gold 5218 CPU @ 2.30GHz
CPU MHz:               2799.615
CPU max MHz:           3900.0000
CPU min MHz:           1000.0000
BogoMIPS:              4600.00
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              1024K
L3 cache:              22528K

$ lsmem
Memory block size:         1G
Total online memory:     383G
```

VISE builds a search engine index in the following 5 stages: traindesc, cluster, trainassign, hamm, index. The table below shows computation time for each stage.
```
|----------------+--------------------------|
| Indexing Stage | Computation Time (HH:MM) |
|----------------+--------------------------|
| traindesc      |                    00:37 |
| cluster        |                    06:23 |
| trainassign    |                    00:30 |
| hamm           |                    00:02 |
| index          |                    35:04 |
|----------------+--------------------------|
```

The size of binary files produced by this indexing process are as follows:
```
|-----------------------+--------|
| VISE Project Filename | Size   |
|-----------------------+--------|
| index_dset.bin        | 17  Mb |
| index_fidx.bin        | 2.5 Gb |
| index_iidx.bin        | 42  Gb |
| metadata_db.sqlite    | 98  Mb |
| trainassign.bin       | 382 Mb |
| traindesc.bin         | 12  Gb |
| trainhamm.bin         | 25  Mb |
| weight.bin            | 2.4 Mb |
|-----------------------+--------|
```

Note: If you want to create a search engine for a large image dataset but do not have the computing resources to do so, please [contact us](mailto:adutta@robots.ox.ac.uk). We may be able to help you by generating the index in our servers and sharing the resulting index. To allow your users to visually search through this image dataset, you will need to host this visual search engine in your server which has sufficiently large memory (e.g. 64Gb). The server does not need too much of computational resources.


## Should I use the generic visual vocabulary to index my images?
You should try to index a small subset (e.g. 10%) of your images using the provided [generic visual vocabulary](doc/Visual-Vocabulary.md) and manually judge the visual search performance. If the visual search results are accurate and acceptable for your purpose, you should certainly use the generic visual vocabulary on the full dataset as this avoids the heavy computation associated with creating a visual vocabulary (i.e. traindesc, cluster, trainassign and trainhamm stages). Here is how you can do this experiment.

```
# Assuming that your project folder configuration file is 
# stored in $MYPROJ/data/conf.txt

# 1. Download generic visual vocabulary
cd $MYPROJ/data
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm64.zip
unzip imcount53629-imsize400x400-voc10k-hamm64.zip
mv imcount53629-imsize400x400-voc10k-hamm64/bowcluster.bin $MYPROJ/data
mv imcount53629-imsize400x400-voc10k-hamm64/trainhamm.bin $MYPROJ/data
## delete imcount53629-imsize400x400-voc10k-hamm64/ folder
## rm -fr imcount53629-imsize400x400-voc10k-hamm64

# 2. Ensure that all your images (i.e. the full dataset) is 
# stored in $MYPROJ/image/ folder. We create a $MYPROJ/data/filelist.txt
# file that contains filenames of only 10% of your files (instead of the 
# the usual list of all files). We randomly select 10% of the files as follows:

cd $MYPROJ/image/
find . -type f -printf '%f\n' > $MYPROJ/data/filelist-all.txt
# assuming that you have 1,000,000 images, we try on 10,000 images
shuf -n 10000 $MYPROJ/data/filelist-all.txt > $MYPROJ/data/filelist.txt

# 3. Update project state
touch $MYPROJ/data/traindesc.bin # to avoid running clustering stage
echo "start,filelist,traindesc,cluster,assign,hamm" > $MYPROJ/data/index_status.txt

# 4. Update $MYPROJ/data/conf.txt to use 64 bits for hamming embedding
# because the generic visual vocabulary uses 64 bits.
# i.e. set hamm=64 in $MYPROJ/data/conf.txt

# 5. Start the indexing process
# The indexing process will only consider 10,000 images as 
# $MYPROJ/data/filelist.txt only contains filenames of 10000 images
$VISE_CODE/vise/cmake_build/vise/vise-cli --cmd=create-project \
  myproject:$MYPROJ/data/conf.txt
```

## How can I speed up the process of creating a VISE project?
 * **Use [generic visual vocabulary](doc/Visual-Vocabulary.md)** :
```
# Assuming that your project folder configuration file is 
# stored in $MYPROJ/data/conf.txt

# 1. Download generic visual vocabulary
cd $MYPROJ/data
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm64.zip
unzip imcount53629-imsize400x400-voc10k-hamm64.zip
mv imcount53629-imsize400x400-voc10k-hamm64/bowcluster.bin $MYPROJ/data
mv imcount53629-imsize400x400-voc10k-hamm64/trainhamm.bin $MYPROJ/data
## delete imcount53629-imsize400x400-voc10k-hamm64/ folder
## rm -fr imcount53629-imsize400x400-voc10k-hamm64

# 2. Update project state
touch $MYPROJ/data/traindesc.bin # to avoid running clustering stage
echo "start,filelist,traindesc,cluster,assign,hamm" > $MYPROJ/data/index_status.txt

# 3. Update conf.txt to use 64 bits for hamming embedding
# because the generic visual vocabulary uses 64 bits.
# update conf.txt such that hamm=64
```

 * **Resize the images to `1024x1024`** : Image resolution of `1024x1024` is often sufficient for visual search and therefore higher resolution images (e.g `1600x1600`) can be resized to speed up the region detection, feature extraction and indexing process.
```
# Assuming that your project folder is organized as follows:
# $MYPROJ/data/ : contains conf.txt and other project files
# $MYPROJ/image_src/ : original images (e.g. 1600x1600 pixels) 
# $MYPROJ/image/     : images that will be used by VISE for indexing

rsync -av $MYPROJ/image_src/ $MYPROJ/image/
cd $MYPROJ/image/
find . -exec mogrify -resize 1024x1024\> {} \;
```


### Speed up the web interface of a project
The home page of a VISE project shows a list of images. This home page shows thumbnail sized version of images present in a project in a grid layout. The thumbnail sized version of images is generated by the web browser by scaling original images. This can be avoided by creating thumbnail sized images (e.g. `250x250`) of the original images and using them in the home page view. Therefore, we create a new folder `image_small` and store all the thumbnails in this folder.
 
```
# Assuming that your project folder is organized as follows:
# $MYPROJ/data/ : contains conf.txt and other project files
# $MYPROJ/image_src/ : original images (e.g. 1600x1600 pixels) 
# $MYPROJ/image/     : images that are used by VISE (e.g. 1024x1024 pixels)

mkdir $MYPROJ/image_small/
rsync -av $MYPROJ/image/ $MYPROJ/image_small/
cd $MYPROJ/image_small/
find . -exec mogrify -resize 250x250\> {} \;
```

### Change the web interface to suit my requirements
The VISE web based user interface is generated by HTML, CSS and Javascript files stored in `src/www/` folder of the VISE source code. You can create a copy of this folder and update the files in this new folder according to your requirements. You can use `--http-www-dir=NEW-FOLDER` option in `vise-cli` command to let VISE generate web based user interface from the new folder.


## Can I control what metadata is available to VISE REST API or gets shown in the user interface?
Yes. Using the `data/metadata_conf.json` file, you can define the file and region attributes that are visible to the users. Here is an sample `data/metadata_conf.json` file taken from the [NLS Chapbooks project](https://gitlab.com/vgg/nls-chapbooks-illustrations).

```
{
  "data_format_version": 1,
  "file_attributes_id_list": [   ## these are the column-ids from file_metadata
                                 ## table of the metadata_db.sqlite database that
                                 ## are visible to the users
    "file_id",                  
    "filename",
    "chapbook_id",
    "publisher"
  ],
  "region_attributes_id_list": [ ## these are the column-ids from region_metadata
                                 ## table of the metadata_db.sqlite database that
                                 ## are visible to the users
    "region_index"
  ],
  "groupby_aid_list": [          ## Users can group all the images in the database
                                 ## using only these file attributes
    "chapbook_id",
    "publisher"
  ],
  "file_attributes": {           ## this section controls the display of file attributes
    "file_id": {
      "aname": "File Id",        ## show label "File Id" instead of "file_id"
      "href": "file?file_id=$file_id$"  ## show as hyperlink to the original file
    },
    "filename": {
      "aname": "Filename",
      "href": "image/$filename$"
    },
    "chapbook_id": {             ## show chapbook-id as a hyperlink to the 
                                 ## external chapbook metadata hosted by NLS 
      "aname": "NLS Chapbook Id",
      "href": "https://digital.nls.uk/$chapbook_id$"
    },
    "publisher": {
      "aname": "Publisher"       ## show as a plain text metadata (without any link)
    }
  },
  "region_attributes": {        ## this section controls the display of region attributes
    "region_index": {
      "aname": "Region Index"
    }
  }
}
```

## Can VISE be installed on a web server? Can this server be running Windows Server 2016 or 2019?
Yes. By default, the VISE HTTP web server is bound to `localhost` (or `127.0.0.1` interface) for security reasons. This is for security reason and prevents access from external network. In Windows Server (or any other server), you will need to open the port (e.g. 9669) on which the VISE HTTP web server is listening. 

We suggest to use another HTTP web server (e.g. nginx, apache, IIS, etc.) to redirect external requests to the VISE HTTP server. This allows the other web server to handle security aspects (e.g. SSL certificates) of the HTTP web server.

## Can we change the graphical interface to work integrated with our website?
Yes. VISE comes with a basic and minimal web application (HTML, Javascript, CSS) that allows users to explore all the functionalities of the VISE application. Users can extend this basic web application in any way that they see suitable for their requirements. The code for basic web application is contained in the [`src/www/`](src/www/) folder of the source tree. See [doc/HTTP-REST-API.md](doc/HTTP-REST-API.md)

## Whenever we have new images in our database, do we have to do a re-index in VISE?
Yes. You can avoid some computations by reusing the visual vocabulary (i.e. `traindesc`, `cluster`, `trainhamm` stages). If you reuse an existing visual vocabulary, you will need to only run the final `index` stage. See `data/index_status.log` file to see a list of all stages (e.g. `start,filelist,traindesc,cluster,assign,hamm,index,end`) that gets executed when a new VISE project is created. So, if you only want to re-run the index stage, update the `data/index_status.log` file to contain `start,filelist,traindesc,cluster,assign,hamm` (i.e. remove the last `index` term) so that the final `index` stage gets recomputed.

## Would be possible to integrate VISE (and how?) with the site on IIS?
VISE runs its own HTTP server on a specific port (e.g. 9669). All HTTP requests to this port are handled by the VISE web server. Another web server (like Windows IIS) can redirect requests to this port.

## In Windows, is it possible to use VISE without installing it by just copying the VISE folder and running the VISE.exe to get the server running?
Yes. The VISE Windows installer simply copies all the required files ( VISE executable, dependencies DLL, sample project data, etc. ) to `C:\Program Files\VISE\` folder.

## My VISE project already has 10 million images and I want to add some more images (e.g. 1000 images) to this project. Can I add new images to existing VISE project?
At the moment, it is not possible to add new images. So, you will have to create a new VISE project with 10,001,000 images in it and redo all the stages of creating a VISE project. This is inefficient and we are working to find a more efficient way to add new images to an existing VISE project.

It is possible to avoid recomputing the visual vocabularly by re-using the visual vocabulary computed in the original 10 million images. The visual vocabulary computed on 10 million images is already sufficient for a small number (e.g. 10000) of new images. Here is an illustrative example showing the commands required to re-use existing visual vocabulary.

```
## See User-Guide.md
export VISE_DIR=$HOME/vise
export VISE_CODE=$HOME/vise/code
export VISE_DEP=$HOME/vise/dep

## We assume that the following.
## - $VISE_DIR/projects/p10m/    : a VISE project with 10 million images
## - $VISE_DIR/projects/p10m-1k/ : a new VISE project which contains 10 million
##                                 images from "p10m" project as well as an
##                                 additional 1000 images

mkdir -p $VISE_DIR/projects/p10m-1k/
rsync -av $VISE_DIR/projects/p10m/ $VISE_DIR/projects/p10m-1k/     ## create a copy of the original project
cd $VISE_DIR/projects/p10m-1k/data
rm index_dset.bin index_fidx.bin \              ## delete existing index files but retain visual vocabulary
  index_iidx.bin index.log trainassign.bin \    ## i.e. retain bowcluster.bin, conf.txt, filelist.txt,
  trainhamm.bin weight.bin                      ## index_status.txt, metadata_conf.json, metadata_db.sqlite

## add new images to $VISE_DIR/projects/p10m-1k/image/
cp SOME-FOLDER/new-file1.jpg $VISE_DIR/projects/p10m-1k/image/
...

## update index_status.txt such that visual vocabularly
## computation stage is skipped
cd $VISE_DIR/projects/p10m-1k/data
touch traindesc.bin
cat index_status.txt                            ## ensure that the index_status.txt file contents match as
start,filelist,traindesc,cluster                ## shown here

## append new image filenames to filelist.txt
echo "new-file1.jpg" >> $VISE_DIR/projects/p10m-1k/filelist.txt
...

## delete any temporary files
rm $VISE_DIR/projects/p10m-1k/_tmp/*.*

## start the indexing process which indexes all images contained filelist.txt
## Note: the first 3 stages (i.e. filelist, traindesc, cluster) stages
## are skipped and existing visual vocabulary (i.e. bowcluster.bin) is
## reused by the indexing process
$VISE_CODE/vise/cmake_build/vise/vise-cli --cmd=create-project \
  p10m-1k:$VISE_DIR/projects/p10m-1k/data/conf.txt
```

## VISE crashes with SEGMENTATION FAULT
Most often, the SEGMENTATION FAULT error in VISE is caused by the feature extractor crashing due to a malformed input image. For large datasets, it is important to cleanup the data before feeding it to VISE. Here are some tips on how to identify malformed images in a large dataset.

 * Discard all images which are less than 1KB as these often correspond to images without much content using the following command: `find . -size +1023c -type f -printf '%P\n'` (to delete the files, use `-delete` flag).
 * If the dataset contains files of various types (e.g. TIF, PNG, JPG), the [convert-to-jpg.sh](scripts/preprocess/convert-to-jpg.sh) script can be used to convert them to JPEG. This script also resizes images and converts them to sRGB colourspace.
 * Use imagemagick's `identify` command to check if an image is malformed.

## How can I reduce the memory requirements of VISE?
Resizing the images is one of the easiest way to reduce the memory footprint of a VISE project. VISE operates with high accuracy every for lower resolution images like `600x600` or `800x800` pixels. VISE does not benefit much from very high resolution images like `1600x1600` or more. The [convert-to-jpg.sh](scripts/preprocess/convert-to-jpg.sh) script converts images to JPG and resizes them such that largest image dimension is at most `1200` pixels.

## The image page does not show any metadata and VISE log states `failed to load metadata_conf`
The `data/metadata_conf.json` file must be present to show file metadata. This file should be updated based on specific requirements of a project. Here is a minimal example of the `data/metadata_conf.json` file.
```
{
  "data_format_version": 1,
  "file_attributes_id_list": [
    "file_id",
    "filename",
  ],
  "region_attributes_id_list": [
  ],
  "groupby_aid_list": [
  ],
  "file_attributes": {
    "file_id": {
      "aname": "File Id",
      "href": "file?file_id=$file_id$"
    },
    "filename": {
      "aname": "Filename",
      "href": "image/$filename$"
    },
  },
  "region_attributes": {
  }
}
```
***

Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.
