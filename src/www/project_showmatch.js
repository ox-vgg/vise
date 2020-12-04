/**
 *
 * @desc code to build HTML user interface for /{PNAME}/showmatch endpoint
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
var querymatch_panel = document.createElement('div');
querymatch_panel.setAttribute('class', 'querymatch_panel');
content.appendChild(querymatch_panel);

var toggle_panel = document.createElement('div');
toggle_panel.setAttribute('class', 'toggle_panel');
content.appendChild(toggle_panel);

var match_panel = document.createElement('div');
match_panel.setAttribute('class', 'match_panel');
content.appendChild(match_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

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

// update the search result when browser is resized
// this is required as the image size changes and hence the bounding box needs update
window.addEventListener('resize', _vise_init_show_match_ui);

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
  _vise_init_show_match_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data.PNAME) === 'string' &&
      typeof(_vise_data.QUERY) === 'object' &&
      typeof(_vise_data.MATCH) === 'object'
    ) {
    return true;
  }
  return false;
}

function _vise_init_show_match_ui() {
  _vise_set_project_pagetools(pagetools);

  _vise_querymatch_panel_show();

  var query_img_uri = 'image/' + _vise_data.QUERY['filename'];
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

function _vise_querymatch_panel_show() {
  querymatch_panel.innerHTML = '';

  var query = document.createElement('div');
  query.setAttribute('class', 'query');
  var qimgcontainer = document.createElement('div');
  qimgcontainer.setAttribute('class', 'img_with_region');
  var qimg = document.createElement('img');
  qimg.setAttribute('src', 'image/' + _vise_data.QUERY['filename']);
  qimg.addEventListener('load', _vise_on_img_load_show_query_rshape);
  var qlabel = document.createElement('a');
  qlabel.innerHTML = 'Query: ' + _vise_data.QUERY['filename'];
  qlabel.setAttribute('href', 'file?file_id=' + _vise_data.QUERY['file_id']);
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
  rshape.setAttribute('x', Math.floor(_vise_data.QUERY['x'] * scale));
  rshape.setAttribute('y', Math.floor(_vise_data.QUERY['y'] * scale));
  rshape.setAttribute('width', Math.floor(_vise_data.QUERY['width'] * scale));
  rshape.setAttribute('height', Math.floor(_vise_data.QUERY['height'] * scale));
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
  req.push('register?file1_id=' + _vise_data.QUERY['file_id']);
  req.push('file2_id=' + _vise_data.MATCH['file_id']);
  req.push('x=' + _vise_data.QUERY['x']);
  req.push('y=' + _vise_data.QUERY['y']);
  req.push('width=' + _vise_data.QUERY['width']);
  req.push('height=' + _vise_data.QUERY['height']);
  req.push('H0=' + JSON.stringify(_vise_data.MATCH_DETAILS['H']));
  var xhr = new XMLHttpRequest();
  xhr.open('GET', req.join('&'));
  xhr.send();
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
  var qlabel = 'Query: ' + _vise_data.QUERY['filename'];
  var qmeasure = ctx.measureText(qlabel);
  if(qmeasure.width > qsdim[0]) {
    var maxchar = qsdim[0] / charwidth;
    var qlabel = '...' + _vise_data.QUERY['filename'].substring(_vise_data.QUERY['filename'].length - maxchar);
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
    var download_filename = 'vise-' + _vise_data.QUERY['filename'] + '-' + _vise_data.MATCH['filename'] + '-match.png';
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
