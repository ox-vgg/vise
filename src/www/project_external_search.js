/**
 *
 * @desc code to build HTML user interface for /{PNAME}/external_search endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 02 Dec. 2020
 *
 * todo: create a module based on functionality of project_showmatch.js and reuse it
 *       in project_external_search.js and project_search.js. The current code is not
 *       most easy to understand and debug. (3 Dec. 2020)
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

// for file upload
var upload_container = document.createElement('div');
upload_container.setAttribute('class', 'upload_container');
var upload_panel = document.createElement('div');
upload_panel.setAttribute('class', 'upload_panel');
var progress_panel = document.createElement('div');
progress_panel.setAttribute('class', 'progress_panel');
var progress = document.createElement('progress');
progress.setAttribute('max', '3');
progress.setAttribute('value', '0');
var progress_message = document.createElement('p');
progress_panel.appendChild(progress);
progress_panel.appendChild(progress_message);
upload_container.appendChild(upload_panel);
//upload_container.appendChild(progress_panel); // added after user uploads an image
content.appendChild(upload_container);

// for external search results
var query_panel = document.createElement('div');
query_panel.setAttribute('class', 'query_panel hide');
content.appendChild(query_panel);
var results_panel = document.createElement('div');
results_panel.setAttribute('class', 'results_panel');
content.appendChild(results_panel);
var _vise_external_search = {};


// for match details
var querymatch_panel = document.createElement('div');
querymatch_panel.setAttribute('class', 'querymatch_panel hide');
content.appendChild(querymatch_panel);
var toggle_panel = document.createElement('div');
toggle_panel.setAttribute('class', 'toggle_panel');
content.appendChild(toggle_panel);
var match_panel = document.createElement('div');
match_panel.setAttribute('class', 'match_panel');
content.appendChild(match_panel);
var query_img;
var match_img;
var toggle_canvas;
var match_img_tx_canvas;
var toggle_now_showing = '';
var register_response = {}; // received from /{PNAME}/register endpoint
var feat_match_canvas;

var rand_pts_count = 3;
var show_region = true;
var show_correspondence = true;
var rand_selected_match_pts_index = [];
var stroke_width = 3;

var is_manual_toggle_mode = false;
var is_toggle_active = true;
var mouse_move_count = 0;
var toggle_canvas_timer;
var feature_type = 'matches'; // {matches, putative}

document.body.appendChild(toolbar);
document.body.appendChild(content);

var showing_result_from = 0;
var showing_result_to = 0;
const SHOW_SIZE = 5;
var current_score_threshold = 0;
var current_norm_score_threshold = 0.07;
var next_norm_score_threshold = 0.05;

var selected_file = null;
var selected_file_object_url = null;
var selected_file_features = null;
var selected_image_dim = [-1, -1];

var _vise_match_details_file_id = null;

var results = document.createElement('div');
results.setAttribute('class', 'resultgrid');
//results.setAttribute('class', 'resultlist');

// update the search result when browser is resized
// this is required as the image size changes and hence the bounding box needs update
window.addEventListener('resize', _vise_init_external_search_result_ui);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'filelist');
  pname_link.setAttribute('title', 'Home page of ' + _vise_data.PNAME);
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_init_external_search_result_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_init_external_search_result_ui() {
  _vise_set_project_pagetools(pagetools);
  _vise_show_search_result_toolbar();
  _vise_show_local_file_uploader();
}

function _vise_show_search_results() {
  pageinfo.innerHTML = '';
  upload_container.innerHTML = '';
  query_panel.innerHTML = '';
  results_panel.innerHTML = '';
  querymatch_panel.classList.add('hide');

  _vise_show_search_query_content();
  _vise_show_search_result_content();
}

function _vise_show_search_result_toolbar() {
  pageinfo.innerHTML = '';
}

function _vise_show_local_file_uploader() {
  upload_panel.innerHTML = '';

  var title = document.createElement('h3');
  title.innerHTML = 'Search using you image'
  var file_selector = document.createElement('input');
  file_selector.setAttribute('type', 'file');
  file_selector.setAttribute('accept', 'image/*');
  file_selector.addEventListener('change', function(e) {
    selected_file = e.target.files[0];
    selected_file_object_url = URL.createObjectURL(selected_file);
    var selected_image = document.createElement('img');
    selected_image.addEventListener('load', function(e) {
      selected_image_dim = [this.naturalWidth, this.naturalHeight];
    });
    selected_image.src = selected_file_object_url;
    selected_image.setAttribute('title', selected_file.name);
    upload_panel.innerHTML = '';
    upload_panel.appendChild(selected_image);

    upload_container.appendChild(progress_panel);
    progress.setAttribute('value', '1');
    var file_size = (selected_file.size/1024).toFixed(1);
    progress_message.innerHTML = 'Uploading image ' + selected_file.name + ' (' + file_size + ' KB) ...';
    _vise_extract_features();
  });
  upload_panel.appendChild(title);
  upload_panel.appendChild(file_selector);
}

function _vise_extract_features() {
  var xhr = new XMLHttpRequest();
  xhr.responseType = "blob";
  xhr.addEventListener('load', function(e) {
    switch(xhr.statusText) {
    case 'OK':
      progress.setAttribute('value', '2');
      progress_message.innerHTML = 'Image features extracted ...';
      selected_file_features = this.response;
      _vise_search_features();
      break;
    default:
      progress_message.innerHTML = 'Error: malformed response from VISE server. (' + this.response + ')';
    }
  });
  xhr.addEventListener('timeout', function(e) {
    progress_message.innerHTML = 'Timeout waiting for response from server';
  });
  xhr.addEventListener('error', function(e) {
    progress_message.innerHTML = 'Error waiting for response from server';
  });
  xhr.open('POST', '_extract_image_features');
  xhr.send(selected_file);
}

function _vise_search_features() {
  var xhr = new XMLHttpRequest();
  xhr.responseType = "application/json";
  xhr.addEventListener('load', function(e) {
    switch(xhr.statusText) {
    case 'OK':
      progress.setAttribute('value', '3');
      progress_message.innerHTML = 'Finished searching using image features.';
      _vise_external_search = JSON.parse(this.response);
      _vise_external_search.QUERY = {'file_id':'Uploaded',
                          'filename':selected_file.name,
                          'x':0, 'y':0, 'width':selected_image_dim[0], 'height':selected_image_dim[1]};
      upload_container.innerHTML = '';
      _vise_show_search_results();
      break;
    default:
      progress_message.innerHTML = 'Error: malformed response from VISE server. (' + this.response + ')';
    }
  });
  xhr.addEventListener('timeout', function(e) {
    progress_message.innerHTML = 'Timeout waiting for response from server';
  });
  xhr.addEventListener('error', function(e) {
    progress_message.innerHTML = 'Error waiting for response from server';
  });
  xhr.open('POST', '_search_using_features');
  xhr.send(selected_file_features);
}

function _vise_show_search_query_content() {
  var query = document.createElement('div');
  query.setAttribute('class', 'query');
  var qimgcontainer = document.createElement('div');
  qimgcontainer.setAttribute('class', 'img_with_region');
  var qimg = document.createElement('img');
  qimg.setAttribute('src', selected_file_object_url);
  qimg.addEventListener('load', _vise_on_img_load_show_query_rshape);
  var qlabel = document.createElement('p');
  var qhref = selected_file.name + ' (uploaded)';
  qlabel.innerHTML = 'Query: ' + qhref;
  qimgcontainer.appendChild(qimg);
  query.appendChild(qimgcontainer);
  query.appendChild(qlabel);

  query_panel.innerHTML = '';
  query_panel.appendChild(query);
}

function _vise_on_img_load_show_query_rshape(e) {
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

  var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(_vise_external_search.QUERY['x'] * scale));
  rshape.setAttribute('y', Math.floor(_vise_external_search.QUERY['y'] * scale));
  rshape.setAttribute('width', Math.floor(_vise_external_search.QUERY['width'] * scale));
  rshape.setAttribute('height', Math.floor(_vise_external_search.QUERY['height'] * scale));
  svg.appendChild(rshape);

  e.target.parentNode.appendChild(svg);
}

function _vise_show_search_result_nomatch_found() {
  var nomatches = document.createElement('div');
  nomatches.setAttribute('id', 'nomatches');
  nomatches.innerHTML = 'No matches found';
  results.appendChild(nomatches);

  showing_result_from = -1;
  showing_result_to   = -1;
}

function _vise_show_search_result_content() {
  results_panel.innerHTML = '';
  var result_title = document.createElement('h3');
  result_title.innerHTML = 'Search Results';
  results_panel.appendChild(result_title);

  results.innerHTML = '';
  if(_vise_external_search.RESULT.length === 0) {
    // indicates only match to itself or no match at all
    _vise_show_search_result_nomatch_found();
    return;
  }

  if(_vise_external_search.RESULT.length < 5) {
    // show all results
    for( var i=0; i<_vise_external_search.RESULT.length; ++i) {
      var a = _vise_search_result_html_element(i);
      results.appendChild(a);
    }
  } else {
    showing_result_from = 0; // discard first result which corresponds to the query image
    showing_result_to = Math.min(showing_result_from + SHOW_SIZE, _vise_external_search.RESULT.length);
    for( var i=showing_result_from; i<showing_result_to; ++i) {
      var a = _vise_search_result_html_element(i);
      results.appendChild(a);
    }
    if(showing_result_to < (_vise_external_search.RESULT.length - 1)) {
      // more results remaining to show
      var showmore = document.createElement('div');
      showmore.setAttribute('id', 'showmore');
      var info = document.createElement('p');
      info.innerHTML = 'Showing results from 1 to ' + showing_result_to + '.';
      var more = document.createElement('p');
      more.innerHTML = '<span class="text_button" onclick="_vise_show_more_search_results()">Show ' + SHOW_SIZE + ' more</span>';
      showmore.appendChild(info);
      showmore.appendChild(more);
      results.appendChild(showmore);
    }
  }

  results_panel.appendChild(results);
  _vise_search_set_view_style();
}

function _vise_search_result_html_element(result_index) {
  var a = document.createElement('a');
  a.setAttribute('data-rindex', result_index)
  a.setAttribute('title', _vise_external_search.RESULT[result_index]['filename'] + ', score=' + _vise_external_search.RESULT[result_index]['score'].toFixed(1));
  a.setAttribute('onclick', '_vise_get_feature_match_details(' + _vise_external_search.RESULT[result_index]['file_id'] + ')');

  var img = document.createElement('img');
  img.setAttribute('src', 'image/' + _vise_external_search.RESULT[result_index]['filename']);
  img.setAttribute('data-rindex', result_index);
  img.addEventListener('load', _vise_on_img_load_show_result_rshape);
  a.appendChild(img);

  if(results.classList.contains('resultgrid')) {
    return a;
  }
  if(results.classList.contains('resultlist')) {
    var div = document.createElement('div');
    div.setAttribute('class', 'item');
    a.setAttribute('title', 'Click to show details of match');
    div.appendChild(a);

    var table = document.createElement('table');
    var tr0 = document.createElement('tr');
    var td01 = document.createElement('td');
    td01.innerHTML = 'Rank';
    var td02 = document.createElement('td');
    td02.innerHTML = result_index + 1;
    tr0.appendChild(td01);
    tr0.appendChild(td02);
    table.appendChild(tr0);

    var tr1 = document.createElement('tr');
    var td11 = document.createElement('td');
    td11.innerHTML = 'Filename';
    var td12 = document.createElement('td');
    td12.innerHTML = '<a href="file?file_id=' + _vise_external_search.RESULT[result_index]['file_id'] + '">' + _vise_external_search.RESULT[result_index]['filename'] + '</a>';
    tr1.appendChild(td11);
    tr1.appendChild(td12);
    table.appendChild(tr1);

    var tr2 = document.createElement('tr');
    var td21 = document.createElement('td');
    td21.innerHTML = 'Score';
    var td22 = document.createElement('td');
    td22.innerHTML = _vise_external_search.RESULT[result_index]['score'];
    tr2.appendChild(td21);
    tr2.appendChild(td22);
    table.appendChild(tr2);
    div.appendChild(table);

    return div;
  }
}

function _vise_show_more_search_results() {
  var showmore = document.getElementById('showmore');
  results.removeChild(showmore);

  showing_result_from = showing_result_to;
  showing_result_to = Math.min(showing_result_from + SHOW_SIZE, _vise_external_search.RESULT.length);
  for( var i=showing_result_from; i<showing_result_to; ++i) {
    var a = _vise_search_result_html_element(i);
    results.appendChild(a);
  }
  if(showing_result_to < (_vise_external_search.RESULT.length - 1)) {
    // more results remaining to show
    var showmore = document.createElement('div');
    showmore.setAttribute('id', 'showmore');
    var info = document.createElement('p');
    info.innerHTML = 'Showing results from 1 to ' + showing_result_to + ' (of ' + _vise_external_search.RESULT.length + ').';
    var more = document.createElement('p');
    more.innerHTML = '<span class="text_button" onclick="_vise_show_more_search_results()">Show ' + SHOW_SIZE + ' more</span>';
    showmore.appendChild(info);
    showmore.appendChild(more);
    results.appendChild(showmore);
  }
}

function _vise_get_feature_match_details(match_file_id) {
  _vise_match_details_file_id = match_file_id;
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function(e) {
    switch(xhr.statusText) {
    case 'OK':
      _vise_data = JSON.parse(this.response);
      _vise_data['QUERY'] = _vise_external_search.QUERY;
      _vise_show_match();
      break;
    default:
      progress_message.innerHTML('Error: malformed response from VISE server. (' + this.response + ')');
    }
  });
  xhr.addEventListener('timeout', function(e) {
    progress_message.innerHTML = 'Timeout waiting for response from server';
  });
  xhr.addEventListener('error', function(e) {
    progress_message.innerHTML = 'Error waiting for response from server';
  });
  xhr.open('POST', '_get_feature_match_details?match_file_id=' + _vise_match_details_file_id);
  xhr.send(selected_file_features);
}

function _vise_on_img_load_show_result_rshape(e) {
  var rindex = parseInt(e.target.dataset.rindex);
  var svg = document.createElementNS(_VISE_SVG_NS, 'svg');
  svg.setAttribute('style', 'height:' + e.target.height + 'px;width:' + e.target.width + 'px;');

  var qx1 = _vise_external_search.QUERY['x'];
  var qy1 = _vise_external_search.QUERY['y'];
  var qx2 = qx1 + _vise_external_search.QUERY['width'];
  var qy2 = qy1 + _vise_external_search.QUERY['height'];
  var H = _vise_external_search.RESULT[rindex]['H'];

  var scale = e.target.height / e.target.naturalHeight;

  var rx1 = (H[0] * qx1 + H[1] * qy1 + H[2]) * scale;
  var ry1 = (H[3] * qx1 + H[4] * qy1 + H[5]) * scale;
  var rx2 = (H[0] * qx2 + H[1] * qy2 + H[2]) * scale;
  var ry2 = (H[3] * qx2 + H[4] * qy2 + H[5]) * scale;
  var sane_rect = _vise_sanitize_rect(rx1, ry1, rx2, ry2);
  rx1 = sane_rect[0];
  ry1 = sane_rect[1];
  rx2 = sane_rect[2];
  ry2 = sane_rect[3];

  var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(rx1));
  rshape.setAttribute('y', Math.floor(ry1));
  rshape.setAttribute('width', Math.floor(rx2 - rx1));
  rshape.setAttribute('height', Math.floor(ry2 - ry1));

  svg.appendChild(rshape);
  e.target.parentNode.appendChild(svg);

  // set image label
  if(results.classList.contains('resultgrid')) {
    var label = document.createElement('div');
    label.setAttribute('class', 'result_index');
    label.innerHTML = rindex + 1;
    e.target.parentNode.appendChild(label);
  }
}


function _vise_search_result_toggle_layout() {
  if(results.classList.contains('resultgrid')) {
	  results.setAttribute('class', 'resultlist');
  } else {
	  results.setAttribute('class', 'resultgrid');
  }
  _vise_show_search_result_content();
}

function _vise_search_result_toggle_layout() {
  if(results.classList.contains('resultgrid')) {
    results.classList.remove('resultgrid');
    results.classList.add('resultlist');
  } else {
    results.classList.remove('resultlist');
    results.classList.add('resultgrid');
  }
  _vise_show_search_result_content();
}

function _vise_search_set_view_style() {
  var result_layout_selector = document.createElement('div');
  result_layout_selector.setAttribute('class', 'text_button');
  result_layout_selector.setAttribute('onclick', '_vise_search_result_toggle_layout()');
  if(results.classList.contains('resultgrid')) {
	  result_layout_selector.innerHTML = 'List View';
  } else {
	  result_layout_selector.innerHTML = 'Grid View';
  }
  pageinfo.innerHTML = '';
  pageinfo.appendChild(result_layout_selector);
}

//
// code borrowed from project_showmatch.js
//
function _vise_show_match() {
  pageinfo.innerHTML = '';
  upload_container.innerHTML = '';
  query_panel.innerHTML = '';
  querymatch_panel.classList.remove('hide');
  results_panel.innerHTML = '<p><span onclick="_vise_show_search_results()" class="text_button">Back to list of search results for uploaded image</span></p>';

  if(toggle_canvas_timer) {
    clearTimeout(toggle_canvas_timer);
  }
  register_response = {};
  mouse_move_count = 0;

  _vise_querymatch_panel_show();
  var query_img_uri = selected_file_object_url;
  var match_img_uri = 'image/' + _vise_data.MATCH['filename'];
  var load_promise_list = [];
  load_promise_list.push( _vise_load_remote_img(query_img_uri) );
  load_promise_list.push( _vise_load_remote_img(match_img_uri) );
  Promise.all(load_promise_list).then( function(loaded_img_list) {
    query_img = loaded_img_list[0];
    match_img = loaded_img_list[1];
    _vise_togglepanel_show();
    _vise_matchpanel_show();
  }, function(err_list) {
    console.log('failed to load images');
    console.log(err_list);
  });
}

function _vise_load_remote_img(src) {
  return new Promise( function(ok_callback, err_callback) {
    var img = new Image();
    img.src = src;
    img.addEventListener('load', function(e) {
      ok_callback(this);
    });
    img.addEventListener('error', function(e) {
      err_callback();
    });
  });
}

function _vise_querymatch_panel_show() {
  querymatch_panel.innerHTML = '';

  var query = document.createElement('div');
  query.setAttribute('class', 'query');
  var qimgcontainer = document.createElement('div');
  qimgcontainer.setAttribute('class', 'img_with_region');
  var qimg = document.createElement('img');
  qimg.setAttribute('src', selected_file_object_url);
  qimg.addEventListener('load', _vise_on_img_load_show_query_rshape);
  var qlabel = document.createElement('span');
  qlabel.innerHTML = 'Query: ' + selected_file.name + ' (uploaded)';
  qimgcontainer.appendChild(qimg);
  query.appendChild(qimgcontainer);
  query.appendChild(qlabel);

  var match = document.createElement('div');
  match.setAttribute('class', 'match');
  var mimgcontainer = document.createElement('div');
  mimgcontainer.setAttribute('class', 'img_with_region');
  var mimg = document.createElement('img');
  mimg.setAttribute('src', 'image/' + _vise_data.MATCH['filename']);
  mimg.addEventListener('load', _vise_on_img_load_show_match_rshape);
  var mlabel = document.createElement('a');
  mlabel.innerHTML = 'Match: ' + _vise_data.MATCH['filename'];
  mlabel.setAttribute('href', 'file?file_id=' + _vise_data.MATCH['file_id']);
  mimgcontainer.appendChild(mimg);
  match.appendChild(mimgcontainer);
  match.appendChild(mlabel);

  querymatch_panel.appendChild(query);
  querymatch_panel.appendChild(match);
}

function _vise_on_img_load_show_query_rshape(e) {
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

  var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(_vise_external_search.QUERY['x'] * scale));
  rshape.setAttribute('y', Math.floor(_vise_external_search.QUERY['y'] * scale));
  rshape.setAttribute('width', Math.floor(_vise_external_search.QUERY['width'] * scale));
  rshape.setAttribute('height', Math.floor(_vise_external_search.QUERY['height'] * scale));
  svg.appendChild(rshape);
  e.target.parentNode.appendChild(svg);
}

function _vise_on_img_load_show_match_rshape(e) {
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

  var qx1 = _vise_data.QUERY['x'];
  var qy1 = _vise_data.QUERY['y'];
  var qx2 = qx1 + _vise_data.QUERY['width'];
  var qy2 = qy1 + _vise_data.QUERY['height'];
  var H = _vise_data.MATCH_DETAILS['H'];

  var scale = e.target.height / e.target.naturalHeight;
  var rx1 = (H[0] * qx1 + H[1] * qy1 + H[2]) * scale;
  var ry1 = (H[3] * qx1 + H[4] * qy1 + H[5]) * scale;
  var rx2 = (H[0] * qx2 + H[1] * qy2 + H[2]) * scale;
  var ry2 = (H[3] * qx2 + H[4] * qy2 + H[5]) * scale;
  var sane_rect = _vise_sanitize_rect(rx1, ry1, rx2, ry2);
  rx1 = sane_rect[0];
  ry1 = sane_rect[1];
  rx2 = sane_rect[2];
  ry2 = sane_rect[3];

  var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(rx1));
  rshape.setAttribute('y', Math.floor(ry1));
  rshape.setAttribute('width', Math.floor(rx2 - rx1));
  rshape.setAttribute('height', Math.floor(ry2 - ry1));

  svg.appendChild(rshape);
  e.target.parentNode.appendChild(svg);
}

function _vise_togglepanel_show() {
  toggle_panel.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = 'Difference between Query and Match';
  toggle_panel.appendChild(title);

  //var target_height = toggle_panel.clientHeight;
  var target_height = _vise_data.QUERY['height'];

  var qx1 = _vise_data.QUERY['x'];
  var qy1 = _vise_data.QUERY['y'];
  var qw = _vise_data.QUERY['width'];
  var qh = _vise_data.QUERY['height'];
  var qx2 = qx1 + qw;
  var qy2 = qy1 + qh;
  var qcanvas = _vise_imregion_to_canvas(query_img, qx1, qy1, qw, qh, target_height);
  var qregion = document.createElement('div');
  var qcaption = document.createElement('p');
  qcaption.innerHTML = 'Query Region';
  qregion.appendChild(qcanvas);
  qregion.appendChild(qcaption);

  var H = _vise_data.MATCH_DETAILS['H'];
  var rx1 = (H[0] * qx1 + H[1] * qy1 + H[2]);
  var ry1 = (H[3] * qx1 + H[4] * qy1 + H[5]);
  var rx2 = (H[0] * qx2 + H[1] * qy2 + H[2]);
  var ry2 = (H[3] * qx2 + H[4] * qy2 + H[5]);
  var sane_rect = _vise_sanitize_rect(rx1, ry1, rx2, ry2);
  rx1 = sane_rect[0];
  ry1 = sane_rect[1];
  rx2 = sane_rect[2];
  ry2 = sane_rect[3];

  var mcanvas = _vise_imregion_to_canvas(match_img,
                                         rx1,
                                         ry1,
                                         rx2 - rx1,
                                         ry2 - ry1,
                                         target_height
                                        );
  var mregion = document.createElement('div');
  var mcaption = document.createElement('p');
  mcaption.innerHTML = 'Match Region';
  mregion.appendChild(mcanvas);
  mregion.appendChild(mcaption);

  toggle_canvas = document.createElement('canvas');
  var toggle = document.createElement('div');
  var tcaption = document.createElement('p');
  var tspeed = document.createElement('input');
  tspeed.setAttribute('type', 'range');
  tspeed.setAttribute('id', 'toggle_speed');
  tspeed.setAttribute('min', '200');
  tspeed.setAttribute('max', '1400');
  tspeed.setAttribute('step', '200');
  tspeed.setAttribute('value', '600');
  var tspeedlabel = document.createElement('label');
  tspeedlabel.setAttribute('for', 'toggle_speed');
  tspeedlabel.setAttribute('id', 'toggle_speed_label');
  tspeedlabel.innerHTML = '&nbsp;Delay&nbsp;';

  var toggle_label = document.createElement('label');
  toggle_label.setAttribute('for', 'toggle_checkbox');
  toggle_label.innerHTML = 'Toggle';
  var toggle_checkbox = document.createElement('input');
  toggle_checkbox.setAttribute('type', 'checkbox');
  toggle_checkbox.setAttribute('name', 'toggle_checkbox');
  toggle_checkbox.checked = true;
  toggle_checkbox.addEventListener('change', function(e) {
	  if(this.checked) {
		//enable toggle
		is_toggle_active = true;
		is_manual_toggle_mode = false;
		mouse_move_count = 0;
        _vise_toggle_canvas_toggle();
	  } else {
		//disable toggle
		is_toggle_active = false;
		is_manual_toggle_mode = true;
	  }
  });
  tcaption.appendChild(toggle_label);
  tcaption.appendChild(toggle_checkbox);
  tcaption.appendChild(tspeedlabel);
  tcaption.appendChild(tspeed);
  toggle.appendChild(toggle_canvas);
  toggle.appendChild(tcaption);

  toggle_panel.appendChild(qregion);
  toggle_panel.appendChild(toggle);
  toggle_panel.appendChild(mregion);

  var req = [];
  req.push('_external_register?file2_id=' + _vise_data.MATCH['file_id']);
  req.push('H0=' + JSON.stringify(_vise_data.MATCH_DETAILS['H']));
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function(e) {
    switch(xhr.statusText) {
    case 'OK':
      register_response = JSON.parse(xhr.responseText);
      if(register_response['STATUS'] !== 'ok') {
        _vise_toggle_canvas_show_error_msg(register_response['message']);
        return;
      }
      break;
    default:
      _vise_toggle_canvas_show_error_msg('Malformed response from VISE server.');
      return;
    }

    // transform the match_img based on obtained Homography
    var H = register_response.H;
    match_img_tx_canvas = document.createElement('canvas');
    match_img_tx_canvas.width = query_img.naturalWidth;
    match_img_tx_canvas.height = query_img.naturalHeight;
    var match_img_tx_ctx = match_img_tx_canvas.getContext('2d', { alpha: false });
    match_img_tx_ctx.setTransform(H[0], H[3], H[1], H[4], H[2], H[5]);
    match_img_tx_ctx.drawImage(match_img, 0, 0);

    toggle_canvas.height = _vise_data.QUERY['height'];
    toggle_canvas.width = _vise_data.QUERY['width'];

    _vise_toggle_canvas_show_query();
    toggle_canvas.addEventListener('mousedown', function(e) {
      _vise_toggle_canvas_show_match();
    });
    toggle_canvas.addEventListener('mouseup', function(e) {
      _vise_toggle_canvas_show_query();
    });
    toggle_canvas.addEventListener('mouseover', function(e) {
      is_manual_toggle_mode = true;
      mouse_move_count = 0;
    });
    toggle_canvas.addEventListener('mouseout', function(e) {
	    if(is_toggle_active) {
        is_manual_toggle_mode = false;
        mouse_move_count = 0;
        _vise_toggle_canvas_toggle();
	    }
    });
    toggle_canvas.addEventListener('mousemove', function(e) {
      if(is_manual_toggle_mode) {
        if(mouse_move_count > 10) {
          _vise_toggle_canvas_toggle();
          mouse_move_count = 0;
        } else {
          mouse_move_count = mouse_move_count + 1;
        }
      }
    });
	  if(toggle_canvas_timer) {
		  clearTimeout(toggle_canvas_timer);
	  }
    toggle_canvas_timer = setTimeout(_vise_toggle_canvas_toggle,
                                     parseInt(document.getElementById('toggle_speed').value));
  });
  xhr.addEventListener('timeout', function(e) {
    _vise_toggle_canvas_show_error_msg('Timeout waiting for response from server');
  });
  xhr.addEventListener('error', function(e) {
    _vise_toggle_canvas_show_error_msg('Error waiting for response from server');
  });
  xhr.open('POST', req.join('&'));
  xhr.send(selected_file);
}

function _vise_toggle_canvas_show_error_msg(msg) {
  toggle_canvas.height = _vise_data.QUERY['height'];
  toggle_canvas.width = _vise_data.QUERY['width'];
  var ctx = toggle_canvas.getContext('2d', { alpha: false });
  ctx.fillStyle='red';
  ctx.font = '16px Sans';
  var lineheight = 20;
  var words = msg.split(' ');
  var line = '';
  var y = 20;

  for(var i = 0; i < words.length; i++) {
    var testLine = line + words[i] + ' ';
    var metrics = ctx.measureText(testLine);
    var testWidth = metrics.width;
    if (testWidth > (toggle_canvas.width - 12)  && i > 0) {
      ctx.fillText(line, 10, y);
      line = words[i] + ' ';
      y += lineheight;
      if(y >= toggle_canvas.height) {
        break; // no more vertical space left to draw
      }
    } else {
      line = testLine;
    }
  }
  if(y < toggle_canvas.height) {
    ctx.fillText(line, 10, y);
  }
}

function _vise_toggle_canvas_get_expand_size() {
  var expand_size = Math.floor(_vise_data.QUERY['width'] * 0.1);
  var ex = Math.max(0, _vise_data.QUERY['x'] - expand_size);
  var ey = Math.max(0, _vise_data.QUERY['y'] - expand_size);
  var ewidth  = Math.min(_vise_data.QUERY['width'] + 2*expand_size, query_img.naturalWidth);
  var eheight = Math.min(_vise_data.QUERY['height'] + 2*expand_size, query_img.naturalHeight);
  return [ex, ey, ewidth, eheight];
}

function _vise_toggle_canvas_toggle() {
  if(toggle_now_showing === 'query') {
    _vise_toggle_canvas_show_match();
  } else {
    _vise_toggle_canvas_show_query();
  }

  if(!is_manual_toggle_mode) {
    setTimeout(_vise_toggle_canvas_toggle,
               parseInt(document.getElementById('toggle_speed').value));
  }
}

function _vise_toggle_canvas_show_query() {
  var expand_dim = _vise_toggle_canvas_get_expand_size();
  var ctx = toggle_canvas.getContext('2d', { alpha: false });
  ctx.drawImage(query_img, expand_dim[0], expand_dim[1], expand_dim[2], expand_dim[3],
                0, 0, toggle_canvas.width, toggle_canvas.height);
  toggle_now_showing = 'query';
}

function _vise_toggle_canvas_show_match() {
  var expand_dim = _vise_toggle_canvas_get_expand_size();
  var ctx = toggle_canvas.getContext('2d', { alpha: false });
  ctx.drawImage(match_img_tx_canvas, expand_dim[0], expand_dim[1], expand_dim[2], expand_dim[3],
                0, 0, toggle_canvas.width, toggle_canvas.height);
  toggle_now_showing = 'match';
}

function _vise_imregion_to_canvas(img, x, y, width, height, target_height) {
  var c = document.createElement('canvas');
  c.height = target_height;
  var ar = c.height/height;
  c.width = ar * width;

  var ctx = c.getContext('2d', { alpha: false });
  ctx.drawImage(img, x, y, width, height, 0, 0, c.width, c.height);

  return c;
}

function _vise_matchpanel_show() {
  var title = document.createElement('h3');
  title.innerHTML = 'Visual Features and their Correspondences';

  feat_match_canvas = document.createElement('canvas');
  _vise_matchpanel_update(); // initializes feat_match_canvas

  var toolbar = document.createElement('div');
  _vise_matchpanel_init_toolbar(toolbar);

  match_panel.innerHTML = '';
  match_panel.appendChild(title);
  match_panel.appendChild(feat_match_canvas);
  match_panel.appendChild(toolbar);
}

function _vise_matchpanel_update_region_count(e) {
  switch(e.target.value) {
  case 'drawall':
    rand_pts_count = _vise_data.MATCH_DETAILS[feature_type].length;
    break;
  case 'draw_random_regions':
    rand_pts_count = parseInt(document.getElementById('rand_pts_count_input').value);
    break;
  }
  _vise_matchpanel_update();
}

function _vise_matchpanel_update_draw_property(e) {
  switch(e.target.name) {
  case 'draw_region':
    if(e.target.checked) {
      show_region = true;
    } else {
      show_region = false;
    }
    break;
  case 'draw_correspondence':
    if(e.target.checked) {
      show_correspondence = true;
    } else {
      show_correspondence = false;
    }
    break;
  }
  _vise_matchpanel_redraw();
}

function _vise_matchpanel_update() {
  rand_selected_match_pts_index = [];
  if(rand_pts_count === _vise_data.MATCH_DETAILS[feature_type].length) {
    for(var i=0; i<_vise_data.MATCH_DETAILS[feature_type].length; ++i) {
      rand_selected_match_pts_index.push(i);
    }
  } else {
    rand_selected_match_pts_index = _vise_get_rand_int_list(0,
                                                            _vise_data.MATCH_DETAILS[feature_type].length,
                                                            rand_pts_count);
  }
  _vise_matchpanel_draw_correspondence(rand_selected_match_pts_index);
}

function _vise_matchpanel_redraw() {
  _vise_matchpanel_draw_correspondence(rand_selected_match_pts_index);
}

function _vise_matchpanel_draw_correspondence(selected_match_pts_index) {
  var match_pts_index = selected_match_pts_index;
  if(typeof(selected_match_pts_index) === 'undefined' ||
     selected_match_pts_index.length === 0 ) {
    match_pts_index = [];
    for(var i=0; i<_vise_data.MATCH_DETAILS[feature_type].length; ++i) {
      match_pts_index.push(i);
    }
  }

  var vw_max = match_panel.offsetWidth;
  var vh_max = Math.floor((3/4) * document.documentElement.clientHeight);
  var pad = 15;

  var qw = query_img.naturalWidth;
  var qh = query_img.naturalHeight;
  var mw = match_img.naturalWidth;
  var mh = match_img.naturalHeight;

  var imv_maxw = (vw_max/2) - 2*pad;
  var imv_maxh = vh_max - 2*pad;

  var qsdim = _vise_matchpanel_img_scale_dim(imv_maxw, imv_maxh, qw, qh);
  var msdim = _vise_matchpanel_img_scale_dim(imv_maxw, imv_maxh, mw, mh);

  var vw = qsdim[0] + msdim[0] + 4*pad;
  var vh = Math.max(qsdim[1], msdim[1]) + 4*pad;
  var qoffsetx = pad;
  var qoffsety = 2*pad;
  var moffsetx = qsdim[0] + 3*pad;
  var moffsety = 2*pad;

  feat_match_canvas.height = vh;
  feat_match_canvas.width = vw;

  var ctx = feat_match_canvas.getContext('2d', { alpha: false });
  ctx.fillStyle = 'black';
  ctx.fillRect(0, 0, vw, vh);
  ctx.drawImage(query_img,
                0, 0, qw, qh,
                qoffsetx, qoffsety, qsdim[0], qsdim[1]);
  ctx.drawImage(match_img,
                0, 0, mw, mh,
                moffsetx, moffsety, msdim[0], msdim[1]);

  // draw query and match regions
  ctx.strokeStyle = 'yellow';
  ctx.lineWidth = stroke_width;
  var qx1 = _vise_data.QUERY['x'];
  var qy1 = _vise_data.QUERY['y'];
  var qx2 = qx1 + _vise_data.QUERY['width'];
  var qy2 = qy1 + _vise_data.QUERY['height'];
  ctx.strokeRect(qoffsetx + qx1*qsdim[2], qoffsety + qy1*qsdim[2],
                 (qx2-qx1)*qsdim[2], (qy2-qy1)*qsdim[2]);

  var H = _vise_data.MATCH_DETAILS['H'];
  var mx1 = (H[0] * qx1 + H[1] * qy1 + H[2]);
  var my1 = (H[3] * qx1 + H[4] * qy1 + H[5]);
  var mx2 = (H[0] * qx2 + H[1] * qy2 + H[2]);
  var my2 = (H[3] * qx2 + H[4] * qy2 + H[5]);
  ctx.strokeRect(moffsetx + mx1*msdim[2], moffsety + my1*msdim[2],
                 (mx2-mx1)*msdim[2], (my2-my1)*msdim[2]);

  if(show_region) {
    // draw query Ellipses
    ctx.strokeStyle = 'blue';
    ctx.lineWidth = stroke_width;
    for(var i=0; i<match_pts_index.length; ++i) {
      var rand_index = match_pts_index[i];
      var d = _vise_data.MATCH_DETAILS[feature_type][rand_index];
      var ellipse1_param = _vise_matchpanel_get_ellipse_param(d[2], d[3], d[4]);
      ctx.beginPath();
      ctx.ellipse(qoffsetx + d[0]*qsdim[2], qoffsety + d[1]*qsdim[2],
                  ellipse1_param[0], ellipse1_param[1], ellipse1_param[2],
                  0, 2*Math.PI);
      ctx.stroke();
    }

    // draw match Ellipses
    ctx.strokeStyle = 'blue';
    ctx.lineWidth = stroke_width;
    for(var i=0; i<match_pts_index.length; ++i) {
      var rand_index = match_pts_index[i];
      var d = _vise_data.MATCH_DETAILS[feature_type][rand_index];
      var ellipse2_param = _vise_matchpanel_get_ellipse_param(d[7], d[8], d[9]);
      ctx.beginPath();
      ctx.ellipse(moffsetx + d[5]*msdim[2], moffsety + d[6]*msdim[2],
                  ellipse2_param[0], ellipse2_param[1], ellipse2_param[2],
                  0, 2*Math.PI);
      ctx.stroke();
    }
  }

  if(show_correspondence) {
    // draw correspondences
    ctx.strokeStyle = 'green';
    ctx.lineWidth = stroke_width;
    ctx.beginPath();
    //ctx.setLineDash([10, 5]);
    for(var i=0; i<match_pts_index.length; ++i) {
      var rand_index = match_pts_index[i];
      var d = _vise_data.MATCH_DETAILS[feature_type][rand_index];
      ctx.moveTo(qoffsetx + d[0]*qsdim[2], qoffsety + d[1]*qsdim[2]);
      ctx.lineTo(moffsetx + d[5]*msdim[2], moffsety + d[6]*msdim[2]);
    }
    ctx.stroke();
  }

  // draw label
  ctx.fillStyle = 'yellow';
  ctx.font = '14px Sans';
  var qregion = 'Query Region: (x,y)=(' + _vise_data.QUERY['x'] + ',' + _vise_data.QUERY['y'] + ')  (width,height)=(' + _vise_data.QUERY['width'] + ',' + _vise_data.QUERY['height'] + ') | Putative matches = ' + _vise_data.MATCH_DETAILS['putative'].length + ' | Spatially verified matches = ' + _vise_data.MATCH_DETAILS['matches'].length;
  ctx.fillText(qregion, qoffsetx, pad + 4);

  var charwidth = ctx.measureText('M').width;
  var qlabel = 'Query: ' + selected_file.name;
  var qmeasure = ctx.measureText(qlabel);
  if(qmeasure.width > qsdim[0]) {
    var maxchar = qsdim[0] / charwidth;
    var qlabel = '...' + selected_file.name.substring(selected_file.name.length - maxchar);
  }
  ctx.fillText(qlabel, qoffsetx, vh - pad + 5);

  var mlabel = 'Match: ' + _vise_data.MATCH['filename'];
  var mmeasure = ctx.measureText(mlabel);
  if(mmeasure.width > msdim[0]) {
    var maxchar = msdim[0] / charwidth;
    var mlabel = '...' + _vise_data.MATCH['filename'].substring(_vise_data.MATCH['filename'].length - maxchar);
  }
  ctx.fillText(mlabel, moffsetx, vh - pad + 5);
}

function _vise_matchpanel_init_toolbar(toolbar) {
  // controls
  var region_sel = document.createElement('p');

  var drawall = document.createElement('input');
  drawall.setAttribute('type', 'radio');
  drawall.setAttribute('value', 'drawall');
  drawall.setAttribute('name', 'region_selector');
  drawall.addEventListener('click', _vise_matchpanel_update_region_count);
  var drawall_label = document.createElement('label');
  drawall_label.setAttribute('for', 'drawall');
  drawall_label.innerHTML = 'Draw all ' + _vise_data.MATCH_DETAILS[feature_type].length + ' regions';
  region_sel.appendChild(drawall);
  region_sel.appendChild(drawall_label);

  var rand = document.createElement('input');
  rand.setAttribute('type', 'radio');
  rand.setAttribute('value', 'draw_random_regions');
  rand.setAttribute('checked', '');
  rand.setAttribute('name', 'region_selector');
  rand.addEventListener('click', _vise_matchpanel_update_region_count);
  var rand_label = document.createElement('label');
  rand_label.setAttribute('for', 'rand');
  var rand_label_text1 = document.createElement('span');
  rand_label_text1.innerHTML = 'Draw random';
  var rand_label_text2 = document.createElement('span');
  rand_label_text2.innerHTML = 'regions';
  var rand_count = document.createElement('input');
  rand_count.setAttribute('id', 'rand_pts_count_input');
  rand_count.setAttribute('type', 'text');
  rand_count.setAttribute('pattern', '[0-9]{1,10}');
  rand_count.setAttribute('value', rand_pts_count);
  rand_count.setAttribute('title', 'Only numbers are allowed');

  rand_count.addEventListener('change', function(e) {
    if(this.validity.valid) {
      rand_pts_count = parseInt(this.value);
      _vise_matchpanel_update();
    }
  });

  var redraw = document.createElement('button');
  redraw.innerHTML = 'Redraw';
  redraw.addEventListener('click', function(e) {
    _vise_matchpanel_update();
  });
  rand_label.appendChild(rand_label_text1);
  rand_label.appendChild(rand_count);
  rand_label.appendChild(rand_label_text2);
  region_sel.appendChild(rand);
  region_sel.appendChild(rand_label);
  region_sel.appendChild(redraw);

  // Select what to draw (correspondences, features)
  var drawsel = document.createElement('p');
  var label = document.createElement('span');
  label.innerHTML = 'Draw:&nbsp;';
  drawsel.appendChild(label);

  var region = document.createElement('input');
  region.setAttribute('type', 'checkbox');
  region.setAttribute('name', 'draw_region');
  region.setAttribute('id', 'draw_region_input');
  region.setAttribute('checked', '');
  region.addEventListener('change', _vise_matchpanel_update_draw_property);
  var regionlabel = document.createElement('label');
  regionlabel.setAttribute('for', 'draw_region_input');
  regionlabel.innerHTML = 'Features';
  drawsel.appendChild(region);
  drawsel.appendChild(regionlabel)

  var line = document.createElement('input');
  line.setAttribute('type', 'checkbox');
  line.setAttribute('name', 'draw_correspondence');
  line.setAttribute('id', 'draw_search_match_region_input');
  line.setAttribute('checked', '');
  line.addEventListener('change', _vise_matchpanel_update_draw_property);
  var linelabel = document.createElement('label');
  linelabel.setAttribute('for', 'draw_search_match_region_input');
  linelabel.innerHTML = 'Correspondence';
  drawsel.appendChild(line);
  drawsel.appendChild(linelabel)

  var widthlabel = document.createElement('span');
  widthlabel.innerHTML = 'Line width';
  var width = document.createElement('input');
  width.setAttribute('type', 'text');
  width.setAttribute('value', stroke_width);
  width.addEventListener('change', function(e) {
    var value = parseInt(e.target.value);
    if(value > 0 ) {
      stroke_width = value;
      _vise_matchpanel_redraw();
    }
  });
  var sep = document.createElement('span');
  sep.innerHTML = '&nbsp;|&nbsp;';
  drawsel.appendChild(sep);
  drawsel.appendChild(widthlabel);
  drawsel.appendChild(width);

  // Feature type selection
  var featsel = document.createElement('p');
  var featlabel = document.createElement('span');
  featlabel.innerHTML = 'Show:&nbsp;';
  featsel.appendChild(featlabel);

  var featlist = document.createElement('select');
  var matches = document.createElement('option');
  matches.setAttribute('value', 'matches');
  matches.innerHTML = 'Show only correct matches (i.e. spatially verified matches)';
  var putative = document.createElement('option');
  putative.setAttribute('value', 'putative');
  putative.innerHTML = 'Show all possible matches (i.e. putative matches)';
  switch(feature_type) {
  case 'matches':
    matches.setAttribute('selected', '');
    break;
  case 'putative':
    putative.setAttribute('selected', '');
    break;
  }
  featlist.appendChild(matches);
  featlist.appendChild(putative);
  featsel.appendChild(featlist);
  featlist.addEventListener('change', function(e) {
    feature_type = this.options[this.selectedIndex].value;
    _vise_matchpanel_update();
  });

  var download_icon = _vise_common_get_svg_button('micon_download', 'Download visualisation as a JPG file');
  var download_link = document.createElement('a');
  download_link.addEventListener('click', function() {
    var a = document.createElement('a');
    a.setAttribute('href', feat_match_canvas.toDataURL('image/png'));
    var download_filename = 'vise-' + selected_file.name + '-' + _vise_data.MATCH['filename'] + '-match.png';
    a.setAttribute('download', download_filename);
    a.click();
  });
  download_link.appendChild(download_icon);
  featsel.appendChild(download_link);

  // add everything to HTML DOM
  toolbar.appendChild(region_sel);
  toolbar.appendChild(drawsel);
  toolbar.appendChild(featsel);
}

function _vise_get_rand_int_list(min, max, count) {
  var rand_list = [];
  for(var i=0; i<count; ++i) {
    rand_list.push(_vise_get_rand_int(min, max));
  }
  return rand_list;
}

function _vise_get_rand_int(min, max) {
  return Math.floor(Math.random() * (max - min)) + min;
}

// convert ellipse parameterization (x,y,a,b,c) to
// ellipse with center(x,y) and axes(rx,ry) and rotation theta
// source: jp_draw/jp_draw/_jp_draw.cpp::ellipse_params()
function _vise_matchpanel_get_ellipse_param(a, b, c) {
  var b2 = 2.0*b;
  var det = (b2*b2)/4.0 - a * c;
  var D2 = -det;
  if(det < 0) {
    det = -det;
  }
  var trace = a + c;
  var disc = Math.sqrt(trace*trace - 4.0*D2);
  var factor_dr = D2 / (2.0*det);
  var cmaj = (trace+disc) * factor_dr;
  var cmin = (trace-disc) * factor_dr;
  var major_axis_len = 1.0 / Math.sqrt(cmin);
  var minor_axis_len = 1.0 / Math.sqrt(cmaj);
  var den = c - a;
  var theta = -0.5 * Math.atan2(b, den);

  return [minor_axis_len, major_axis_len, theta];
}

function _vise_matchpanel_img_scale_dim(maxw, maxh, imgw, imgh) {
  var imgsw = 0;
  var imgsh = 0;

  if( imgw > maxw ) {
    imgsw = maxw;
    imgsh = imgsw * (imgh/imgw);
    if(imgsh > maxh) {
      imgsh = maxh;
      imgsw = imgsh * (imgw/imgh);
    }
  } else {
    if(imgh > maxh) {
      imgsh = maxh;
      imgsw = imgsh * (imgw/imgh);
      if(imgsw > maxw) {
        imgsw = maxw;
        imgsh = imgsw * (imgh/imgw);
      }
    } else {
      imgsw = imgw;
      imgsh = imgh;
    }
  }
  return [imgsw, imgsh, imgsw/imgw];
}
