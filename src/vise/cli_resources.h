/** @file cli_resources.h
 *  @brief resources for vise-cli command
 *  @author Abhishek Dutta <adutta _at_ robots.ox.ac.uk>
 *  @date 16 Mar. 2021
 */
#ifndef CLI_RESOURCES_H
#define CLI_RESOURCES_H

namespace vise {
  const char *VISE_CLI_HELP_STR = R"TEXT(## NAME
vise-cli - provides a command line interface to the VGG Image Search Engine (VISE).


## SYNOPSIS
vise-cli --cmd=[create-project | serve-project | web-ui | create-visual-group]
  ...
  PROJECT1_NAME:CONF-FILENAME PROJECT2_NAME:CONF_FILENAME ...

vise-cli --version | --license | --help

## COMMON OPTIONS
VISE runs an HTTP web server to power the HTML based graphical user interface of
the VISE software. The following options control the behaviour of this web server:
All the options below can be saved in a file as OPTION=VALUE in each line and
can be parsed using --vise-conf-filename=vise-conf.txt
--vise-home-dir      : defines $VISE_HOME directory which stores projects, assets, etc.
                       (default is $HOME/.vise/ for Unix and $LOCALAPPDATA/vise/ for Windows)
--vise-project-dir   : default is $VISE_HOME/project/
--vise-asset-dir     : default is $VISE_HOME/asset/

--http-www-dir       : default is $VISE_HOME/www/ and stores static web application files
--http-address       : localhost, 127.0.0.1 or 0.0.0.0
--http-port          : http web server listens to this port
--http-worker        : number of concurrent handler of HTTP requests
--http-namespace     : web interface is http://localhost:9669/HTTP_NAMESPACE/

--nthread-index      : number of concurrent threads to create during indexing
--nthread-search     : number of concurrent threads to use during visual search

--help     : show detailed help message (i.e. this text)
--version  : show current version

## OPTIONS SPECIFIC to create-visual-group
--vgroup-id        : unique identifier for a visual group
--max-matches      : the maximum number of matches that should be considered for each query
--min-match-score  : discard all matches below this score
--filename-like    : pattern in SQL LIKE format, "AB%" selects filenames starting with "AB"
--query-type       : query using file (i.e. full image) or region (i.e. user defined regions)
--vgroup-min-score : score threshold used while creating visual groups
--match-iou-threshold : IoU threshold to check if match regions are same (default = 0.9)


## EXAMPLES
a) access all features of VISE using the web address http://localhost:10011/my_vise/
  vise-cli --cmd=web-ui --http-port=10011 --http-worker=8 \
    --http-namespace=/my_vise/

b) create a project using configuration file /dataset/Oxford-Buildings/data/conf.txt
  vise-cli --cmd=create-project --nthread-index=16 \
    Oxford-Buildings:/dataset/Oxford-Buildings/data/conf.txt

c) allow users to search two projects (e.g. P1, P2) at http://localhost:80/demo/
  vise-cli --cmd=serve-project --http-port=80 \
    --http-namespace=/demo/ \
    P1:/dataset/p1/data/conf.txt \
    P2:/dataset/p2/data/conf.txt

d) to create a visual group
  vise-cli --cmd=create-visual-group \
    --vgroup-id=negative-groups --max-matches=50 --min-match-score=30 \
    --filename-like="negatives/%" --query-type=file --vgroup-min-score=50 \
    --match-iou-threshold=0.9 \
    MY-PROJECT:/dataset/myproject/data/conf.txt


## PROJECT FOLDER STRUCTURE
VISE projects are stored in a folder using the following structure. The folder
structure and names should not be changed as VISE expects these folder names
when loading a project.

Oxford-Buildings/
├── data                # VISE application specific data, log and configurations
│   └── conf.txt        # project configuration
│   └── filelist.txt    # image filename list (relative path)
│   └── index.log       # indexing process log file
│   └── ...             # other application data
├── image               # images used for computing features, matching and display
├── image_src           # original images which are resized/converted and moved to image/
├── image_small         # thumnail sized version of images/ (e.g. 250x250) to speedup UI


## AUTHOR
Abhishek Dutta, Relja Arandjelović, Andrew Zisserman


## HISTORY
VISE builds on the C++ codebase developed by Dr. Relja Arandjelović during his
DPhil / Postdoc at the Visual Geometry Group in 2014. VISE is being further developed
and maintained by Dr. Abhishek Dutta under the guidance of Prof. Andrew Zisserman.

Development and maintenance of VISE software has been supported by the following two grants:
* 2016 to 2020 : Seebibyte: Visual Search for the Era of Big Data (EPSRC Grant EP/M013774/1)
* 2021 to 2025 : Visual AI: An Open World Interpretable Visual Transformer (UKRI Grant EP/T028572/1)

For more details, see https://www.robots.ox.ac.uk/~vgg/software/vise/
)TEXT";

}
#endif
