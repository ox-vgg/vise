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
var pageinfo = document.createElement('div');
pageinfo.setAttribute('class', 'pageinfo');
toolbar.appendChild(pageinfo);

var content = document.createElement('div');
content.setAttribute('id', 'content');
var fileview_panel = document.createElement('div');
fileview_panel.setAttribute('class', 'fileview_panel');
content.appendChild(fileview_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

var query_img;
var via_img_annotator;
var query_img_container = document.createElement('div');
query_img_container.setAttribute('id', 'query_img_container');

var query_img_container_height = 65;

// update the search result when browser is resized
// this is required as the image size changes and hence the bounding box needs update
window.addEventListener('resize', _vise_show_query_ui);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var home_icon = _vise_common_get_svg_button('micon_home', 'VISE Home');
  var home_link = document.createElement('a');
  home_link.setAttribute('href', '../index.html');
  home_link.appendChild(home_icon);

  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'filelist');
  pname_link.setAttribute('title', 'Home of ' + _vise_data.PNAME + ' project');
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(home_link);
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_show_query_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_query_ui() {
  _vise_init_query_toolbar();
  _vise_init_query_content();
}

function _vise_init_query_toolbar() {
  var sep = document.createElement('div');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var fileinfo = document.createElement('div');
  fileinfo.innerHTML = 'Showing file ' + _vise_data.FILE_ID + ' of ' + _vise_data.FLIST_SIZE + ' | <a target="_blank" href="' + _vise_data.FILENAME + '">' + _vise_data.FILENAME + '</a>';

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
  pageinfo.appendChild(fileinfo);
  pageinfo.appendChild(sep.cloneNode(true));
  pageinfo.appendChild(nav);
}

function _vise_init_query_content() {
  fileview_panel.innerHTML = '';
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
    via_img_annotator = new _via0(query_img_container, _vise_data.FILENAME);
    via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);
  });
  var zoomout_link = document.createElement('a');
  zoomout_link.appendChild(zoomout_icon);
  zoomout_link.addEventListener('click', function() {
    query_img_container_height = query_img_container_height - 10;
    if(query_img_container_height > 0) {
      query_img_container.innerHTML = '';
      query_img_container.setAttribute('style', 'height:' + query_img_container_height + 'vh');
      via_img_annotator = new _via0(query_img_container, _vise_data.FILENAME);
      via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);
    }
  });
  toolbar.appendChild(zoomin_link);
  toolbar.appendChild(zoomout_link);

  var formdata = {'file_id':_vise_data.FILE_ID};
  query_img_container.style.height = query_img_container_height + 'vh';
  via_img_annotator = new _via0(query_img_container, _vise_data.FILENAME);
  via_img_annotator.add_hook('region_add', _vise_query_on_region_add, formdata);

  var search_query_form = _vise_query_init_form(formdata);

  var info = document.createElement('p');
  info.innerHTML = 'Draw a region to define the search query.'

  fileview_panel.appendChild(toolbar);
  fileview_panel.appendChild(query_img_container);
  fileview_panel.appendChild(search_query_form);
  fileview_panel.appendChild(info);
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
  fileview_panel.replaceChild(search_query_form, old_search_query_form)
}
