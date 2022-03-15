# VISE HTTP REST API
The VGG Image Search Engine (VISE) listens for HTTP requests on port `9669` (default). All the functionality needed by a web based user interface are available at various GET and POST endpoints shown below.
 
Some Notes:
 - {PNAME} is a place holder for the name of the project (e.g. Oxford-Buildings)
 - A basic web application based solely on HTML/Javascript/CSS for VISE is available in [src/www](src/www) folder of the [source tree](https://gitlab.com/vgg/vise/-/tree/master)
 - HTTP REST API in VISE is implemented by [src/vise/project_manager.cc](src/vise/project_manager.cc)
 
## Search Engine Query

* GET /{PNAME}/filelist?mode=all
  - Description : returns a list of files with index in the range [start,end) for the project `PNAME`
  - Parameters
    - response_format : (optional) can be {html, json}
    - start : start file index (starts fr
    - end : end file index
  - Response : response includes basic information about the project (e.g. number of files, project name, etc.) and details (e.g. filename, file_id) of files in the requested range.
  - Example
  ```
  # show files from index 100 to 150 for the oxford-buildings project
  curl -X GET https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/filelist?mode=all&start=100&end=150&response_format=json
  ```
  
* GET /{PNAME}/filelist?mode=fts
  - Description : returns file list based on full text search of the metadata (e.g. filename, etc.) associated with each file. 
  - Parameters
    - response_format : (optional) can be {html, json}
    - query : (required) search keyword
    - groupby : group results by an file or region attribute
    - group : list only this group
    - start : start file index in the search result list
    - end : end file index in the search result list
  - Response
  - Example: 
    - HTML response: [https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/filelist?mode=fts&query=radcliffe*](https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/filelist?mode=fts&query=radcliffe*)
    - JSON response
    ```
    # shows all files whose filename starts with the keyword radcliffe
    curl -X GET https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/filelist?mode=fts&query=radcliffe*&response_format=json
    ```

* GET /{PNAME}/file
  - Description : shows an HTML page containing a file which allow users to define search query region and perform visual search using this query region.
  - Parameters
    - response_format : (optional) can be {html}
    - file_id : (required) file_id
  - Response
  - Example: 
    - HTML response: [https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/file?file_id=4496](https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/file?file_id=4496)
    
* GET /{PNAME}/search
  - Description : perform visual search based on user defined search query image region
  - Parameters
    - response_format : (optional) can be {html, json}
    - file_id : (required) query file_id
    - x : x coordinate of top left corner of query region
    - y : y coordinate of top left corner of query region
    - width : width of query region
    - height : height of query region
  - Response
  - Example: 
    - HTML response: [https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/search?x=203&y=161&width=507&height=446&file_id=4496](https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/search?x=203&y=161&width=507&height=446&file_id=4496)
    - JSON response
    ```
    # shows all files whose filename starts with the keyword radcliffe
    curl -X GET "https://zeus.robots.ox.ac.uk/vise/demo/oxford-buildings/search?x=203&y=161&width=507&height=446&file_id=4496&response_format=json"
    ```
