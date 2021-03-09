/**
 *
 * @desc code to build HTML user interface for /{PNAME}/search endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 13 Feb. 2020
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
var query_panel = document.createElement('div');
query_panel.setAttribute('class', 'query_panel');
content.appendChild(query_panel);
var results_panel = document.createElement('div');
results_panel.setAttribute('class', 'results_panel');
content.appendChild(results_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

var showing_result_from = 0;
var showing_result_to = 0;
const SHOW_SIZE = 5;
var current_score_threshold = 0;
var current_norm_score_threshold = 0.07;
var next_norm_score_threshold = 0.05;

var results = document.createElement('div');
results.setAttribute('class', 'resultgrid');
//results.setAttribute('class', 'resultlist');

// update the search result when browser is resized
// this is required as the image size changes and hence the bounding box needs update
window.addEventListener('resize', _vise_init_search_result_ui);

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
  _vise_init_search_result_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_init_search_result_ui() {
  _vise_set_project_pagetools(pagetools);
  _vise_show_search_result_toolbar();
  _vise_show_search_query_content();
  _vise_show_search_result_content();
}

function _vise_show_search_result_toolbar() {
  pageinfo.innerHTML = '';
}

function _vise_show_search_query_content() {
  var query = document.createElement('div');
  query.setAttribute('class', 'query');
  var qimgcontainer = document.createElement('div');
  qimgcontainer.setAttribute('class', 'img_with_region');
  var query_image = document.createElement('img');
  query_image.setAttribute('src', 'image/' + _vise_data.QUERY['filename']);
  query_image.addEventListener('load', _vise_on_img_load_show_query_rshape);
  var qlabel = document.createElement('p');
  var qhref = '<a href="file?file_id=' + _vise_data.QUERY['file_id'] + '">' + _vise_data.QUERY['filename'] + '</a>'
  qlabel.innerHTML = 'Query: ' + qhref;
  qimgcontainer.appendChild(query_image);
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

  if(_vise_data.QUERY.hasOwnProperty('x') &&
     _vise_data.QUERY.hasOwnProperty('y') &&
     _vise_data.QUERY.hasOwnProperty('width') &&
     _vise_data.QUERY.hasOwnProperty('height')) {
    var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
    rshape.setAttribute('x', Math.floor(_vise_data.QUERY['x'] * scale));
    rshape.setAttribute('y', Math.floor(_vise_data.QUERY['y'] * scale));
    rshape.setAttribute('width', Math.floor(_vise_data.QUERY['width'] * scale));
    rshape.setAttribute('height', Math.floor(_vise_data.QUERY['height'] * scale));
    svg.appendChild(rshape);
  }

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
  if(_vise_data.RESULT.length < 2) {
    // indicates only match to itself or no match at all
    _vise_show_search_result_nomatch_found();
    return;
  }

  var best_norm_score = _vise_data.RESULT[1]['score'] / _vise_data.RESULT[0]['score'];
  // allow users to reveal the low scoring results
  current_norm_score_threshold = 0.07;
  next_norm_score_threshold = 0.05;
  if(best_norm_score >= 0.5) {
    current_norm_score_threshold = 0.45;
    next_norm_score_threshold = 0.25;
  } else if(best_norm_score >= 0.25 && best_norm_score < 0.5) {
    current_norm_score_threshold = 0.20;
    next_norm_score_threshold = 0.10;
  } else if(best_norm_score >= 0.1 && best_norm_score < 0.25) {
    current_norm_score_threshold = 0.07;
    next_norm_score_threshold = 0.03;
  } else {
    current_norm_score_threshold = 0.03;
    next_norm_score_threshold = 0.01;
  }

  //console.log('best_norm_score=' + best_norm_score + ', current_norm_score_threshold=' + current_norm_score_threshold + ', next_norm_score_threshold=' + next_norm_score_threshold);

  showing_result_from = 1; // discard first result which corresponds to the query image
  for( var i=showing_result_from; i<_vise_data.RESULT.length; ++i) {
    var norm_score = _vise_data.RESULT[i]['score'] / _vise_data.RESULT[0]['score'];
    if(norm_score < current_norm_score_threshold) {
      break;
    }
    var a = _vise_search_result_html_element(i);
    results.appendChild(a);
    showing_result_to = i;
  }
  if(showing_result_to < _vise_data.RESULT.length) {
    var showmore = document.createElement('div');
    showmore.setAttribute('id', 'showmore');
    var info = document.createElement('p');
    info.innerHTML = 'Low scoring matches not shown.';

    var more = document.createElement('p');
    more.innerHTML = '<span class="text_button" onclick="_vise_show_more_search_results()">Show more</span>';
    showmore.appendChild(info);
    showmore.appendChild(more);
    results.appendChild(showmore);
  }

  results_panel.appendChild(results);
  _vise_search_set_view_style();
}

function _vise_search_result_html_element(result_index) {
  var matchview_uri = [];
  matchview_uri.push('showmatch?file_id=' + _vise_data.QUERY['file_id']);
  matchview_uri.push('match_file_id=' + _vise_data.RESULT[result_index]['file_id']);
  if(_vise_data.QUERY.hasOwnProperty('x') &&
     _vise_data.QUERY.hasOwnProperty('y') &&
     _vise_data.QUERY.hasOwnProperty('width') &&
     _vise_data.QUERY.hasOwnProperty('height')) {
    matchview_uri.push('x=' + _vise_data.QUERY['x']);
    matchview_uri.push('y=' + _vise_data.QUERY['y']);
    matchview_uri.push('width=' + _vise_data.QUERY['width']);
    matchview_uri.push('height=' + _vise_data.QUERY['height']);
  }

  var a = document.createElement('a');
  a.setAttribute('data-rindex', result_index)
  a.setAttribute('title', _vise_data.RESULT[result_index]['filename'] + ', score=' + _vise_data.RESULT[result_index]['score'].toFixed(1));
  a.setAttribute('href', matchview_uri.join('&'));

  var img = document.createElement('img');
  img.setAttribute('src', 'image/' + _vise_data.RESULT[result_index]['filename']);
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
    td02.innerHTML = result_index;
    tr0.appendChild(td01);
    tr0.appendChild(td02);
    table.appendChild(tr0);

    var tr1 = document.createElement('tr');
    var td11 = document.createElement('td');
    td11.innerHTML = 'Filename';
    var td12 = document.createElement('td');
    td12.innerHTML = '<a href="file?file_id=' + _vise_data.RESULT[result_index]['file_id'] + '">' + _vise_data.RESULT[result_index]['filename'] + '</a>';
    tr1.appendChild(td11);
    tr1.appendChild(td12);
    table.appendChild(tr1);

    var tr2 = document.createElement('tr');
    var td21 = document.createElement('td');
    td21.innerHTML = 'Score';
    var td22 = document.createElement('td');
    td22.innerHTML = _vise_data.RESULT[result_index]['score'];
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

  showing_result_from = showing_result_to + 1;
  for( var i=showing_result_from; i<_vise_data.RESULT.length; ++i) {
    var norm_score = _vise_data.RESULT[i]['score'] / _vise_data.RESULT[0]['score'];
    if(norm_score > next_norm_score_threshold) {
      var a = _vise_search_result_html_element(i);
      results.appendChild(a);
      showing_result_to = i;
    } else {
      break;
    }
  }

  if(showing_result_to < (_vise_data.RESULT.length - 1)) {
    var showmore = document.createElement('div');
    showmore.setAttribute('id', 'showmore');
    var info = document.createElement('p');
    info.innerHTML = 'Showing results from 1 to ' + (showing_result_to) + ' (of ' + _vise_data.RESULT.length + ').';

    var more = document.createElement('p');
    more.innerHTML = '<span class="text_button" onclick="_vise_show_all_remaining_search_results()">Show remaining results</span>';
    showmore.appendChild(info);
    showmore.appendChild(more);
    results.appendChild(showmore);
  }
}

function _vise_show_all_remaining_search_results() {
  var showmore = document.getElementById('showmore');
  results.removeChild(showmore);

  showing_result_from = showing_result_to + 1;
  for( var i=showing_result_from; i<_vise_data.RESULT.length; ++i) {
      var a = _vise_search_result_html_element(i);
      results.appendChild(a);
      showing_result_to = i;
  }
}

function _vise_on_img_load_show_result_rshape(e) {
  var rindex = parseInt(e.target.dataset.rindex);
  var svg = document.createElementNS(_VISE_SVG_NS, 'svg');
  svg.setAttribute('style', 'height:' + e.target.height + 'px;width:' + e.target.width + 'px;');

  var qx1 = _vise_data.QUERY['x'];
  var qy1 = _vise_data.QUERY['y'];
  var qx2 = qx1 + _vise_data.QUERY['width'];
  var qy2 = qy1 + _vise_data.QUERY['height'];
  var H = _vise_data.RESULT[rindex]['H'];

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
    label.innerHTML = rindex;
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
