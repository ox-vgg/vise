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

## Search Query Using External Image

Searching using an external image is a two step process. The first step extracts
features from the user uploaded image. The second step performs matching of these
features to the set of features corresponding to all the images in the search engine.

External search is implemented using a two step process HTTP POST request to allow
the VISE server remain stateless. This stateless nature of VISE server allows
load balancing of user requests using multiple VISE servers hosting the same
search engine behind a proxy server. The proxy server (e.g. nginx) performs
load balancing of user requests by forwarding stateless user requests to one of
the servers which has the capacity to handle more requests.


For a sample implementation, see the
[src/www/project_external_search.js](src/www/project_external_search.js)
Javascript code which implements external search in the default VISE HTML user
interface that is shipped with the VISE application. The first step of extracting
features from an external image is handled as follows by the VISE server.
[project_manager.cc](https://gitlab.com/vgg/vise/-/blob/master/src/vise/project_manager.cc#L728) 
-> [project.cc](https://gitlab.com/vgg/vise/-/blob/master/src/vise/project.cc#L841)
-> [relja_retrival.cc](https://gitlab.com/vgg/vise/-/blob/master/src/search_engine/relja_retrival/relja_retrival.cc#L1408)
-> [spatial_verif_v2.h](https://gitlab.com/vgg/vise/-/blob/master/src/search_engine/relja_retrival/v2/retrieval/spatial_verif_v2.h#L65)
-> [retriever_v2.cpp](https://gitlab.com/vgg/vise/-/blob/master/src/search_engine/relja_retrival/v2/retrieval/retriever_v2.cpp#L123).
The second step of searching using features extracted from an image (e.g. external image)
is handled by the VISE server as follows.
[project_manager.cc](https://gitlab.com/vgg/vise/-/blob/master/src/vise/project_manager.cc#L756) 
-> [project.cc](https://gitlab.com/vgg/vise/-/blob/master/src/vise/project.cc#L847)
-> [relja_retrival.cc](https://gitlab.com/vgg/vise/-/blob/master/src/search_engine/relja_retrival/relja_retrival.cc#L1413)
-> [spatial_verif_v2.h](https://gitlab.com/vgg/vise/-/blob/master/src/search_engine/relja_retrival/v2/retrieval/spatial_verif_v2.h#L69).

* POST /{PNAME}/_extract_image_features
  - Description: extract visual features from an external image
  - Parameters
    - Request body : image data (binary)
  - Response:
    - a binary stream representing visual features of the image
    - Content-Type: application/octet-stream
  - Example:
    ```
    var xhr = new XMLHttpRequest();
    xhr.responseType = "blob";
    xhr.addEventListener('load', function(e) {
        if(xhr.status === 200) {
            user_selected_image_features = this.response; // bindary data
        }
    });
    xhr.open('POST', '_extract_image_features');
    xhr.send(user_selected_file); // file selected using <input type="file" ...>
    ```

* POST /{PNAME}/_search_using_features
  - Description: perform visual search of internal images using the provided visual features
  - Parameters: None
  - Response: search results in JSON format
  - Example:
    - Sample Javascript code
    ```
    var xhr = new XMLHttpRequest();
    xhr.responseType = "application/json";
    xhr.addEventListener('load', function(e) {
      if(xhr.statusText === 'OK') {
          _vise_external_search = JSON.parse(this.response);
          _vise_external_search.QUERY = {
              'file_id':'Uploaded',
              'filename':selected_filename,
              'x':0, 'y':0,
              'width':user_selected_image_dim[0],
              'height':user_selected_image_dim[1]
          };
      }
    });
    xhr.open('POST', '_search_using_features');
    xhr.send(user_selected_image_features);  // received from "POST _extract_image_features"
    ```
    - Sample JSON response
    ```
    {
      "PNAME": "oxford-buildings",
      "QUERY": "__USING_IMAGE_FEATURE__",
      "RESULT_SIZE": 145,
      "RESULT": [
        {
          "file_id": 2,
          "filename": "all_souls_000002.jpg",
          "score": 2238,
          "H": [1, 0, 0, 0, 1, 0, 0, 0, 1]
        },
        ...
      ]
    }
    ```
