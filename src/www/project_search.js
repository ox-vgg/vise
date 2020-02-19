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

document.body.innerHTML = '';
document.body.appendChild(toolbar);
document.body.appendChild(content);

var SVG_NS = "http://www.w3.org/2000/svg";

var NORM_SCORE_THRESHOLD_LIST = [0.8, 0.6, 0.4, 0.2];
var current_norm_score_threshold_index = 0;

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  pname.innerHTML = '<a href="/' + _vise_data.PNAME + '/">' + _vise_data.PNAME + '</a>';
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
  var svg = document.createElementNS(SVG_NS, 'svg');
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

  var rshape = document.createElementNS(SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(_vise_data.QUERY['x'] * scale));
  rshape.setAttribute('y', Math.floor(_vise_data.QUERY['y'] * scale));
  rshape.setAttribute('width', Math.floor(_vise_data.QUERY['width'] * scale));
  rshape.setAttribute('height', Math.floor(_vise_data.QUERY['height'] * scale));
  svg.appendChild(rshape);

  e.target.parentNode.appendChild(svg);
}

function _vise_show_search_result_content() {
  var resultlist = document.createElement('div');
  resultlist.setAttribute('class', 'resultlist');

  // @todo: only show some of the results and hide the low score results
  // allow users to reveal the low scoring results
  for( var i=1; i<_vise_data.RESULT.length; ++i) {
    var a = document.createElement('a');
    a.setAttribute('data-rindex', i)
    a.setAttribute('title', 'Score=' + _vise_data.RESULT[i]['score'] + ', Filename=' + _vise_data.RESULT[i]['filename'] + ' : Click to view details of match.');
    var matchview_uri = [];
    matchview_uri.push('/' + _vise_data.PNAME + '/showmatch?file_id=' + _vise_data.QUERY['file_id']);
    matchview_uri.push('match_file_id=' + _vise_data.RESULT[i]['file_id']);
    matchview_uri.push('x=' + _vise_data.QUERY['x']);
    matchview_uri.push('y=' + _vise_data.QUERY['y']);
    matchview_uri.push('width=' + _vise_data.QUERY['width']);
    matchview_uri.push('height=' + _vise_data.QUERY['height']);
    a.setAttribute('href', matchview_uri.join('&'));

    var img = document.createElement('img');
    img.setAttribute('src', '/' + _vise_data.PNAME + '/' + _vise_data.RESULT[i]['filename']);
    img.setAttribute('data-rindex', i);
    img.addEventListener('load', _vise_on_img_load_show_result_rshape);

    var label = document.createElement('div');
    label.innerHTML = (i+1);

    a.appendChild(img);
    a.appendChild(label);
    resultlist.appendChild(a);
  }
  resultlist_panel.innerHTML = '';
  resultlist_panel.appendChild(resultlist);
}

function _vise_show_more_search_results() {
}

function _vise_on_img_load_show_result_rshape(e) {
  var rindex = parseInt(e.target.dataset.rindex);
  var svg = document.createElementNS(SVG_NS, 'svg');
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
  var H = _vise_data.RESULT[rindex]['H'];

  var scale = e.target.height / e.target.naturalHeight;
  var rx1 = (H[0] * qx1 + H[1] * qy1 + H[2]) * scale;
  var ry1 = (H[3] * qx1 + H[4] * qy1 + H[5]) * scale;
  var rx2 = (H[0] * qx2 + H[1] * qy2 + H[2]) * scale;
  var ry2 = (H[3] * qx2 + H[4] * qy2 + H[5]) * scale;

  var rshape = document.createElementNS(SVG_NS, 'rect');
  rshape.setAttribute('x', Math.floor(rx1));
  rshape.setAttribute('y', Math.floor(ry1));
  rshape.setAttribute('width', Math.floor(rx2 - rx1));
  rshape.setAttribute('height', Math.floor(ry2 - ry1));
  svg.appendChild(rshape);
  e.target.parentNode.appendChild(svg);
}
