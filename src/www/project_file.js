/**
 *
 * @desc code to build HTML user interface for /{PNAME}/file endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 11 Feb. 2020
 *
 */
'use strict'

var toolbar = document.createElement('div');
toolbar.setAttribute('id', 'toolbar');
var pname = document.createElement('div');
pname.setAttribute('class', 'pname');
toolbar.appendChild(pname);

var pagetools = document.createElement('div');
pagetools.setAttribute('class', 'pagetools');
toolbar.appendChild(pagetools);

var pageinfo = document.createElement('div');
pageinfo.setAttribute('class', 'pageinfo');
toolbar.appendChild(pageinfo);

var content = document.createElement('div');
content.setAttribute('id', 'content');
var file_panel = document.createElement('div');
file_panel.setAttribute('class', 'file_panel');
var file_metadata = document.createElement('div');
file_metadata.setAttribute('class', 'file_metadata');
var file_content = document.createElement('div');
file_content.setAttribute('class', 'file_content');
var region_metadata = document.createElement('div');
region_metadata.setAttribute('class', 'region_metadata');

file_panel.appendChild(file_metadata);
file_panel.appendChild(file_content);
file_panel.appendChild(region_metadata);
content.appendChild(file_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

var query_img;
var via_img_annotator;
var query_img_container = document.createElement('div');
query_img_container.setAttribute('id', 'query_img_container');

var query_img_container_height = 65;

// update the search result when browser is resized
// this is required as the image size changes and hence the bounding box needs update
window.addEventListener('resize', _vise_init_file_panel);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'filelist');
  pname_link.setAttribute('title', 'Home of ' + _vise_data.PNAME + ' project');
  pname_link.innerHTML = _vise_data.PNAME;

  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 240 80');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'style', 'height:0.8em; padding-right:1em;');

  var home_link = document.createElement('a');
  home_link.setAttribute('href', '../home');
  home_link.appendChild(vise_logo);

  pname.innerHTML = '';
  pname.appendChild(home_link);
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_init_file_panel();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_init_file_panel() {
  _vise_set_project_pagetools(pagetools);
  _vise_init_query_toolbar();
  _vise_init_query_file_metadata();
  _vise_init_query_file_content();
  _vise_init_query_region_metadata();
}

function _vise_init_query_toolbar() {
  var sep = document.createElement('div');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var fileinfo = document.createElement('div');
  fileinfo.innerHTML = 'Showing file ' + _vise_data.FILE_ID + ' of ' + _vise_data.FLIST_SIZE + ' | <a target="_blank" href="image/' + _vise_data.FILENAME + '">' + _vise_data.FILENAME + '</a>';

  var list = document.createElement('a');
  list.setAttribute('href', 'filelist?start=' + _vise_data.FILE_ID);
  list.innerHTML = 'List';

  var prev;
  if(_vise_data.FILE_ID === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    prev.setAttribute('href', 'file?file_id=' + (_vise_data.FILE_ID-1));
  }
  prev.innerHTML = 'Prev';

  var next;
  if(_vise_data.FILE_ID === (_vise_data.FLIST_SIZE-1)) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    next.setAttribute('href', 'file?file_id=' + (_vise_data.FILE_ID+1));
  }
  next.innerHTML = 'Next';

  var nav = document.createElement('div');
  nav.appendChild(list);
  nav.appendChild(sep.cloneNode(true));
  nav.appendChild(prev);
  nav.appendChild(sep.cloneNode(true));
  nav.appendChild(next);

  pageinfo.innerHTML = '';
  pageinfo.appendChild(nav);
  pageinfo.appendChild(sep.cloneNode(true));
  pageinfo.appendChild(fileinfo);
}

//
// file content
//
function _vise_init_query_file_metadata() {
  file_metadata.innerHTML = '';
  var table = document.createElement('table');
  var metadata_row_count = 0;
  if(_vise_data.hasOwnProperty('METADATA_CONF')) {
    for(var aindex in _vise_data['METADATA_CONF']['file_attributes_id_list']) {
      var aid = _vise_data['METADATA_CONF']['file_attributes_id_list'][aindex];
      var tr = document.createElement('tr');
      var key = document.createElement('td');
      key.innerHTML = _vise_data['METADATA_CONF']['file_attributes'][aid]['aname'];
      var value_col = document.createElement('td');
      var avalue = _vise_data['FILE_METADATA'][aid];
      if(_vise_data['METADATA_CONF']['file_attributes'][aid].hasOwnProperty('href')) {
        var href_tokens = _vise_data['METADATA_CONF']['file_attributes'][aid]['href'].split('$');
        var tx_href = [];
        for(var i=0; i<href_tokens.length; ++i) {
          if(_vise_data['FILE_METADATA'].hasOwnProperty(href_tokens[i])) {
            tx_href.push(_vise_data['FILE_METADATA'][ href_tokens[i] ]);
          } else {
            tx_href.push(href_tokens[i]);
          }
        }
        if(_vise_data['FILE_METADATA'][aid] !== '') {
          avalue = '<a target="_blank" href="' + tx_href.join('') + '">' + _vise_data['FILE_METADATA'][aid] + '</a>';
        }
      }
      if(avalue === '') {
        continue; // do not show empty attributes
      }
      value_col.innerHTML = avalue;
      tr.appendChild(key);
      tr.appendChild(value_col);
      table.appendChild(tr);
      metadata_row_count = metadata_row_count + 1;
    }
  } else {
    for(var aid in _vise_data['FILE_METADATA']) {
      var tr = document.createElement('tr');
      var key = document.createElement('td');
      key.innerHTML = aid;
      var value = document.createElement('td');
      value.innerHTML = _vise_data['FILE_METADATA'][aid];
      tr.appendChild(key);
      tr.appendChild(value);
      table.appendChild(tr);
      metadata_row_count = metadata_row_count + 1;
    }
  }
  if(metadata_row_count) {
    file_metadata.appendChild(table);
  } else {
    // we do not need to allocate column for file metadata
    file_panel.removeChild(file_metadata);
    file_panel.setAttribute('style', 'grid-template-columns:1fr;');
  }
}

function _vise_init_query_file_content() {
  file_content.innerHTML = '';
  var toolbar = document.createElement('div');
  toolbar.setAttribute('id', 'img_toolbar');
  var zoomin_icon = _vise_common_get_svg_button('micon_zoomin', 'Zoom In');
  var zoomout_icon = _vise_common_get_svg_button('micon_zoomout', 'Zoom Out');

  var zoomin_link = document.createElement('a');
  zoomin_link.appendChild(zoomin_icon);
  zoomin_link.addEventListener('click', function() {
    query_img_container_height = query_img_container_height + 10;
    query_img_container.innerHTML = '';
    query_img_container.setAttribute('style', 'height:' + query_img_container_height + 'vh');
    via_img_annotator = new _via0(query_img_container, 'image/' + _vise_data.FILENAME);
    via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);
  });
  var zoomout_link = document.createElement('a');
  zoomout_link.appendChild(zoomout_icon);
  zoomout_link.addEventListener('click', function() {
    query_img_container_height = query_img_container_height - 10;
    if(query_img_container_height > 0) {
      query_img_container.innerHTML = '';
      query_img_container.setAttribute('style', 'height:' + query_img_container_height + 'vh');
      via_img_annotator = new _via0(query_img_container, 'image/' + _vise_data.FILENAME);
      via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);
    }
  });
  var instruction = document.createElement('span');
  instruction.innerHTML = 'Draw a region on the image to define the search query. <br/>To zoom the image, use '
  toolbar.appendChild(instruction);
  toolbar.appendChild(zoomin_link);
  toolbar.appendChild(zoomout_link);

  query_img_container.innerHTML = '';
  var formdata = {'file_id':_vise_data.FILE_ID};
  query_img_container.style.height = query_img_container_height + 'vh';
  via_img_annotator = new _via0(query_img_container, 'image/' + _vise_data.FILENAME);
  via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);

  var search_query_form = _vise_query_init_form(formdata);

  file_content.appendChild(query_img_container);
  file_content.appendChild(search_query_form);
  file_content.appendChild(toolbar);
}

function _vise_query_init_form(formdata) {
  var c = document.createElement('div');
  c.setAttribute('id', 'search_query_form');

  var form = document.createElement('form');
  form.setAttribute('id', 'search_query_form');
  form.setAttribute('method', 'GET');
  form.setAttribute('action', 'search');

  for(var name in formdata) {
    var field = document.createElement('input');
    field.setAttribute('type', 'hidden');
    field.setAttribute('name', name);
    field.setAttribute('value', Math.floor(formdata[name]));
    form.appendChild(field);
  }

  var search = document.createElement('input');
  search.setAttribute('type', 'submit');
  search.setAttribute('value', 'Search');
  form.appendChild(search);

  c.appendChild(form);
  return c;
}

function _vise_query_on_region_add(region) {
  var old_search_query_form = document.getElementById('search_query_form');
  var search_query_form = _vise_query_init_form(region);
  file_content.replaceChild(search_query_form, old_search_query_form)
}

function _vise_init_query_region_metadata() {
  region_metadata.innerHTML = '';
  if(_vise_data['REGION_METADATA'].length === 0) {
    //region_metadata.innerHTML = '<p>No image regions defined for this image.</p>';
    return;
  }
  var title = document.createElement('h3');
  title.innerHTML = 'Image Regions';
  region_metadata.appendChild(title);

  for(var rindex in _vise_data['REGION_METADATA']) {
    var region_metadata_container = document.createElement('div');
    region_metadata_container.setAttribute('class', 'region_metadata_container');
    var table = document.createElement('table');

    var region_attribute_id_list = _vise_data['METADATA_CONF']['region_attributes_id_list'];
    for(var aindex in region_attribute_id_list) {
      var aid = region_attribute_id_list[aindex];
      if(_vise_data['REGION_METADATA'][rindex].hasOwnProperty(aid)) {
        var tr = document.createElement('tr');
        var key = document.createElement('td');
        if(_vise_data['METADATA_CONF']['region_attributes'].hasOwnProperty(aid)) {
          key.innerHTML = _vise_data['METADATA_CONF']['region_attributes'][aid]['aname'];
        } else {
          key.innerHTML = aid;
        }
        var value = document.createElement('td');
        value.innerHTML = _vise_data['REGION_METADATA'][rindex][aid];
        tr.appendChild(key);
        tr.appendChild(value);
        table.appendChild(tr);
      }
    }

    var region_content_container = document.createElement('div');
    var imgcontainer = document.createElement('div');
    imgcontainer.setAttribute('class', 'img_with_region');
    var img = document.createElement('img');
    img.setAttribute('src', 'image/' + _vise_data.FILENAME);
    img.setAttribute('data-rindex', rindex);
    img.addEventListener('load', _vise_on_region_content_load);
    imgcontainer.appendChild(img);
    region_content_container.appendChild(imgcontainer);

    region_metadata_container.appendChild(table);
    region_metadata_container.appendChild(region_content_container);
    region_metadata.appendChild(region_metadata_container);
  }
}

function _vise_on_region_content_load(e) {
  var rindex = e.target.dataset.rindex;
  var scale = e.target.height / e.target.naturalHeight;
  var svg = document.createElementNS(_VISE_SVG_NS, 'svg');
  var svg_css = [];
  svg_css.push('position:absolute');
  svg_css.push('-webkit-user-select:none;-moz-user-select:none;user-select:none');
  svg_css.push('top:0');
  svg_css.push('left:0');
  svg_css.push('height:' + e.target.height + 'px');
  svg_css.push('width:' + e.target.width + 'px');
  svg_css.push('fill:none');
  svg_css.push('stroke:yellow');
  svg_css.push('stroke-width:2');
  svg.setAttribute('style', svg_css.join(';'));

  var region_shape = _vise_data.REGION_METADATA[rindex]['region_shape'];
  var region_points = _vise_data.REGION_METADATA[rindex]['region_points'];
  switch(region_shape) {
  case 'rect':
    var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
    rshape.setAttribute('x', Math.floor(region_points[0] * scale));
    rshape.setAttribute('y', Math.floor(region_points[1] * scale));
    rshape.setAttribute('width', Math.floor(region_points[2] * scale));
    rshape.setAttribute('height', Math.floor(region_points[3] * scale));
    svg.appendChild(rshape);
    break;
  case 'polygon':
    var rshape = document.createElementNS(_VISE_SVG_NS, 'polygon');
    var pts_array = [];
    for(var i=0; i<region_points.length; i=i+2) {
      var x = Math.floor(region_points[i]*scale);
      var y = Math.floor(region_points[i+1]*scale);
      pts_array.push( x + ',' + y );
    }
    rshape.setAttribute('points', pts_array.join(' '));
    svg.appendChild(rshape);
    break;
  default:
    console.log('unknown region shape: ' + region_shape);
  }
  e.target.parentNode.appendChild(svg);
}
