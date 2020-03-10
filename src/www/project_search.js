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
var pageinfo = document.createElement('div');
pageinfo.setAttribute('class', 'pageinfo');
toolbar.appendChild(pageinfo);

var content = document.createElement('div');
content.setAttribute('id', 'content');
var query_panel = document.createElement('div');
query_panel.setAttribute('class', 'query_panel');
content.appendChild(query_panel);
var resultlist_panel = document.createElement('div');
resultlist_panel.setAttribute('class', 'filelist_panel');
content.appendChild(resultlist_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

var showing_result_from = 0;
var showing_result_to = 0;
var current_score_threshold = 0;
var current_norm_score_threshold = 0.07;
var next_norm_score_threshold = 0.05;

var resultlist = document.createElement('div');
resultlist.setAttribute('class', 'resultlist');

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var home_icon = _vise_common_get_svg_button('micon_home', 'VISE Home');
  var home_link = document.createElement('a');
  home_link.setAttribute('href', '/index.html');
  home_link.appendChild(home_icon);

  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', '/' + _vise_data.PNAME + '/');
  pname_link.setAttribute('title', 'Home page of ' + _vise_data.PNAME);
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(home_link);
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
  var qimg = document.createElement('img');
  qimg.setAttribute('src', '/' + _vise_data.PNAME + '/' + _vise_data.QUERY['filename']);
  qimg.addEventListener('load', _vise_on_img_load_show_query_rshape);
  var qlabel = document.createElement('p');
  var qhref = '<a href="/' + _vise_data.PNAME + '/file?file_id=' + _vise_data.QUERY['file_id'] + '">' + _vise_data.QUERY['filename'] + '</a>'
  qlabel.innerHTML = 'Query: ' + qhref;
  qimgcontainer.appendChild(qimg);
  query.appendChild(qimgcontainer);
  query.appendChild(qlabel);
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
  rshape.setAttribute('x', Math.floor(_vise_data.QUERY['x'] * scale));
  rshape.setAttribute('y', Math.floor(_vise_data.QUERY['y'] * scale));
  rshape.setAttribute('width', Math.floor(_vise_data.QUERY['width'] * scale));
  rshape.setAttribute('height', Math.floor(_vise_data.QUERY['height'] * scale));
  svg.appendChild(rshape);

  e.target.parentNode.appendChild(svg);
}

function _vise_show_search_result_content() {
  resultlist.innerHTML = '';

  // @todo: only show some of the results and hide the low score results
  // allow users to reveal the low scoring results
  current_norm_score_threshold = 0.07;
  next_norm_score_threshold = 0.05;
  var best_norm_score = _vise_data.RESULT[1]['score'] / _vise_data.RESULT[0]['score'];
  if(best_norm_score >= 0.5) {
    current_norm_score_threshold = 0.45;
    next_norm_score_threshold = 0.25;
  }
  if(best_norm_score >= 0.25 && best_norm_score < 0.5) {
    current_norm_score_threshold = 0.20;
    next_norm_score_threshold = 0.10;
  }
  if(best_norm_score >= 0.1 && best_norm_score < 0.25) {
    current_norm_score_threshold = 0.07;
    next_norm_score_threshold = 0.03;
  }

  showing_result_from = 1;
  for( var i=showing_result_from; i<_vise_data.RESULT.length; ++i) {
    var norm_score = _vise_data.RESULT[i]['score'] / _vise_data.RESULT[0]['score'];
    if(norm_score < current_norm_score_threshold) {
      showing_result_to = i;
      break;
    }
    var a = _vise_search_result_html_element(i);
    resultlist.appendChild(a);
  }

  var next_norm_score = _vise_data.RESULT[showing_result_to]['score'] / _vise_data.RESULT[0]['score'];
  if(next_norm_score >= next_norm_score_threshold) {
    var showmore = document.createElement('div');
    showmore.setAttribute('id', 'showmore');
    var info = document.createElement('p');
    info.innerHTML = 'Low scoring matches not shown.';

    var more = document.createElement('p');
    more.innerHTML = '<span class="text_button" onclick="_vise_show_more_search_results()">Show more</span>';
    showmore.appendChild(info);
    showmore.appendChild(more);
    resultlist.appendChild(showmore);
  } else {
    if(showing_result_to === showing_result_from) {
      var nomatches = document.createElement('div');
      nomatches.setAttribute('id', 'nomatches');
      nomatches.innerHTML = 'No matches found';
      resultlist.appendChild(nomatches);
    }
  }

  resultlist_panel.innerHTML = '';
  resultlist_panel.appendChild(resultlist);
}

function _vise_search_result_html_element(result_index) {
  var a = document.createElement('a');
  a.setAttribute('data-rindex', result_index)
  a.setAttribute('title', _vise_data.RESULT[result_index]['filename'] + ', score=' + _vise_data.RESULT[result_index]['score'].toFixed(1));
  var matchview_uri = [];
  matchview_uri.push('/' + _vise_data.PNAME + '/showmatch?file_id=' + _vise_data.QUERY['file_id']);
  matchview_uri.push('match_file_id=' + _vise_data.RESULT[result_index]['file_id']);
  matchview_uri.push('x=' + _vise_data.QUERY['x']);
  matchview_uri.push('y=' + _vise_data.QUERY['y']);
  matchview_uri.push('width=' + _vise_data.QUERY['width']);
  matchview_uri.push('height=' + _vise_data.QUERY['height']);
  a.setAttribute('href', matchview_uri.join('&'));

  var img = document.createElement('img');
  img.setAttribute('src', '/' + _vise_data.PNAME + '/' + _vise_data.RESULT[result_index]['filename']);
  img.setAttribute('data-rindex', result_index);
  img.addEventListener('load', _vise_on_img_load_show_result_rshape);
  a.appendChild(img);
  return a;
}

function _vise_show_more_search_results() {
  var showmore = document.getElementById('showmore');
  resultlist.removeChild(showmore);

  showing_result_from = showing_result_to;
  for( var i=showing_result_to; i<_vise_data.RESULT.length; ++i) {
    var norm_score = _vise_data.RESULT[i]['score'] / _vise_data.RESULT[0]['score'];
    if(norm_score > next_norm_score_threshold) {
      var a = _vise_search_result_html_element(i);
      resultlist.appendChild(a);
    } else {
      break;
    }
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

  var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(rx1));
  rshape.setAttribute('y', Math.floor(ry1));
  rshape.setAttribute('width', Math.floor(rx2 - rx1));
  rshape.setAttribute('height', Math.floor(ry2 - ry1));

  svg.appendChild(rshape);
  e.target.parentNode.appendChild(svg);

  // set image label
  var label = document.createElement('div');
  label.innerHTML = rindex;
  e.target.parentNode.appendChild(label);
}
