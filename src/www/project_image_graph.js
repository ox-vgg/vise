/**
 *
 * @desc code to build HTML user interface for /{PNAME}/image_graph endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 18 Jan. 2021
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
var group_search_panel = document.createElement('div');
group_search_panel.setAttribute('class', 'group_search_panel');
var group_panel = document.createElement('div');
group_panel.setAttribute('class', 'group_panel');
content.appendChild(group_search_panel);
content.appendChild(group_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

const SET_PER_PAGE = 5;

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', '.');
  pname_link.setAttribute('title', 'Home page of ' + _vise_data.PNAME);
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_show_group_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_group_ui() {
  _vise_set_project_pagetools(pagetools);

  var group_icon = _vise_common_get_svg_button('micon_group', 'Show image graph');
  var group_link = document.createElement('a');
  group_link.setAttribute('href', 'group');
  group_link.appendChild(group_icon);
  pagetools.appendChild(group_link);

  if(_vise_data.GROUP.hasOwnProperty('STATUS')) {
    if(_vise_data.GROUP['STATUS'] === 'error') {
      group_panel.innerHTML = '<h3>Error</h3><p>' + _vise_data.GROUP['MESSAGE'] + '</p>';
      return;
    }
    if(_vise_data.GROUP['STATUS'] === 'group_index') {
      var html = '<h3>Image Graph Index</h3><p>' + _vise_data.GROUP['MESSAGE'] + '</p><ul>';
      for(var i=0; i<_vise_data.GROUP['group_id_list'].length; ++i) {
        var group_id = _vise_data.GROUP['group_id_list'][i];
        html += '<li><a href="image_graph?group_id=' + group_id + '">' + group_id + '</a></li>';
      }
      html += '</ul>'
      group_panel.innerHTML = html;
      return;
    }
    group_panel.innerHTML = '<h3>Unknown Status</h3><p>STATUS = ' + _vise_data.GROUP['STATUS'] + '</p>';
    return;
  }
  _vise_init_group_toolbar();
  _vise_init_group_filter_toolbar();
  _vise_init_group_content();
}

function _vise_init_group_toolbar() {
  pageinfo.innerHTML = '';
  if(_vise_data.GROUP['set_id_list'].length === 0) {
    return;
  }
  var sep = document.createElement('span');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var label1 = document.createElement('span');
  label1.innerHTML = '&nbsp;Showing set id';
  var start_input = document.createElement('input');
  start_input.setAttribute('type', 'text');
  start_input.setAttribute('style', 'width:4em;');
  start_input.setAttribute('value', _vise_data.GROUP['set_id_list'][0]);
  start_input.setAttribute('pattern', '[0-9]{1,10}');
  start_input.setAttribute('title', 'Enter the set index to show that set and remaining some sets.');
  start_input.addEventListener('change', function(e) {
    var new_start = parseInt(this.value);
	  if(isNaN(new_start)) {
	    this.value = _vise_data.GROUP['set_id_from'];
	    return;
	  }
	  new_start = new_start;
	  if(new_start < _vise_data.GROUP['set_id_range'][0] || new_start >= _vise_data.GROUP['set_id_range'][1]) {
	    this.value = _vise_data.GROUP['set_id_from'];
	    return;
	  } else {
      var SET_PER_PAGE = _vise_data.GROUP['set_id_list'].length;
      var new_end = Math.min(_vise_data.GROUP['set_id_range'][1], new_start + SET_PER_PAGE);
      var url = [ 'image_graph?group_id=' + _vise_data.GROUP['group_id'] ];
      url.push('set_size=' + _vise_data.GROUP['set_size']);
      url.push('score_threshold=' + _vise_data.GROUP['score_threshold']);
      url.push('from=' + new_start);
      url.push('to=' + new_end);
      window.location.href = url.join('&');
    }
  });

  var label2 = document.createElement('span');
  var SET_PER_PAGE = _vise_data.GROUP['set_id_list'].length;

  var set_id_list_count = _vise_data.GROUP['set_id_list'].length;
  var last_set_id = _vise_data.GROUP['set_id_list'][set_id_list_count-1];
  label2.innerHTML = 'to ' + _vise_data.GROUP['set_id_to'] + ' of ' + _vise_data.GROUP['set_id_range'][1] + ' sets.&nbsp;';
  pageinfo.appendChild(label1);
  pageinfo.appendChild(start_input);
  pageinfo.appendChild(label2);

  pageinfo.appendChild(sep.cloneNode(true));

  var prev_start = Math.max(_vise_data.GROUP['set_id_range'][0], _vise_data.GROUP['set_id_from'] - SET_PER_PAGE);
  var prev_end = _vise_data.GROUP['set_id_from'];
  var prev;
  if(_vise_data.GROUP['set_id_from'] === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    var url = [ 'image_graph?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + _vise_data.GROUP['set_size']);
    url.push('score_threshold=' + _vise_data.GROUP['score_threshold']);
    url.push('from=' + prev_start);
    url.push('to=' + prev_end);
    prev.setAttribute('href', url.join('&'));
  }
  prev.innerHTML = 'Prev';
  pageinfo.appendChild(prev);
  pageinfo.appendChild(sep.cloneNode(true));

  var next_start = _vise_data.GROUP['set_id_to'];
  var next_end = Math.min(_vise_data.GROUP['set_id_range'][1], _vise_data.GROUP['set_id_to'] + SET_PER_PAGE);
  var next;
  if(_vise_data.GROUP['set_id_to'] === _vise_data.GROUP['set_id_range'][1]) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    var url = [ 'image_graph?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + _vise_data.GROUP['set_size']);
    url.push('score_threshold=' + _vise_data.GROUP['score_threshold']);
    url.push('from=' + next_start);
    url.push('to=' + next_end);
    next.setAttribute('href', url.join('&'));
  }
  next.innerHTML = 'Next';
  pageinfo.appendChild(next);
}

function _vise_init_group_filter_toolbar() {
  // info
  var group_info_container = document.createElement('div');
  group_info_container.innerHTML = 'Showing image graph with following properties:&nbsp;';

  // set size
  var set_size_container = document.createElement('div');
  var set_size_label = document.createElement('label');
  set_size_label.setAttribute('for', 'set_size');
  set_size_label.innerHTML = 'Number of matches&nbsp;';
  var set_size_input = document.createElement('select');
  set_size_input.setAttribute('name', 'set_size');
  set_size_input.setAttribute('id', 'set_size');
  for(var set_size in _vise_data.GROUP['set_size_stat']) {
    var set_size_member_count = _vise_data.GROUP['set_size_stat'][set_size];
    var oi = document.createElement('option');
    oi.setAttribute('value', set_size);
    oi.innerHTML = set_size + ' (' + set_size_member_count + ')';
    if(parseInt(set_size) === _vise_data.GROUP['set_size']) {
      oi.setAttribute('selected', '');
    }
    set_size_input.appendChild(oi);
  }
  set_size_input.addEventListener('change', function(e) {
    var new_set_size = this.options[this.selectedIndex].value;
    var url = [ 'image_graph?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + new_set_size);
    url.push('score_threshold=' + _vise_data.GROUP['score_threshold']);
    window.location.href = url.join('&');
  });
  set_size_container.appendChild(set_size_label);
  set_size_container.appendChild(set_size_input);

  // score threshold
  var score_container = document.createElement('div');
  var score_label = document.createElement('label');
  score_label.setAttribute('for', 'score_threshold');
  score_label.innerHTML = 'Score threshold (higher value entails less number of matches)&nbsp;';
  var score_input = document.createElement('input');
  score_input.setAttribute('type', 'number');
  score_input.setAttribute('name', 'score_threshold');
  score_input.setAttribute('id', 'score_threshold');
  score_input.setAttribute('min', _vise_data.GROUP['match_score_range'][0]);
  score_input.setAttribute('max', _vise_data.GROUP['match_score_range'][1]);
  score_input.setAttribute('value', _vise_data.GROUP['score_threshold']);
  score_input.setAttribute('title', 'all matches below this score will be hidden');
  score_input.setAttribute('style', 'width:4em;');
  score_input.addEventListener('change', function(e) {
    var url = [ 'image_graph?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + _vise_data.GROUP['set_size']);
    url.push('score_threshold=' + this.value);
    console.log(url.join('&'))
    window.location.href = url.join('&');
  });
  score_container.appendChild(score_label);
  score_container.appendChild(score_input);

  group_search_panel.innerHTML = '';
  group_search_panel.appendChild(group_info_container);
  group_search_panel.appendChild(set_size_container);
  group_search_panel.appendChild(score_container);
}

function _vise_init_group_content() {
  if(!_vise_data.hasOwnProperty("GROUP")) {
    group_panel.innerHTML = 'Missing group data';
    return;
  }

  group_panel.innerHTML = '';
  if(_vise_data.GROUP['set_id_list'].length === 0) {
    group_panel.innerHTML = '<p>No results found.</p>';
    return;
  }
  for(var set_index in _vise_data.GROUP['set_id_list']) {
    var set_id = _vise_data.GROUP['set_id_list'][set_index];
    var setdata = _vise_data.GROUP.SET[set_id];

    var set_panel = document.createElement('div');
    set_panel.setAttribute('class', 'set_panel');

    // set-id
    var set_id_container = document.createElement('span');
    set_id_container.innerHTML = 'Set ' + set_id;
    set_panel.appendChild(set_id_container);

    // query
    var set_query_figure = document.createElement('figure');
    var set_query_img = document.createElement('img');
    set_query_img.setAttribute('src', 'image_small/' + setdata['query_filename']);
    var set_query_caption = document.createElement('figcaption');
    set_query_caption.innerHTML = 'Query Image : ' + setdata['query_filename'];
    set_query_figure.appendChild(set_query_img);
    set_query_figure.appendChild(set_query_caption);
    var set_query_a = document.createElement('a');
    set_query_a.setAttribute('href', 'file?file_id=' + setdata['query_file_id']);
    set_query_a.appendChild(set_query_figure);
    set_panel.appendChild(set_query_a);

    // match
    var set_matches = document.createElement('div');
    set_matches.setAttribute('class', 'set_matches');
    for (var i=0; i<setdata['match_filename_list'].length; ++i) {
      var set_match_figure = document.createElement('figure');
      var set_match_img = document.createElement('img');
      set_match_img.setAttribute('src', 'image_small/' + setdata['match_filename_list'][i]);
      var set_match_caption = document.createElement('figcaption');
      set_match_caption.innerHTML = '[' + (i+1) + '] : ' + setdata['match_filename_list'][i];
      set_match_figure.appendChild(set_match_img);
      set_match_figure.appendChild(set_match_caption);
      var set_match_a = document.createElement('a');
      set_match_a.setAttribute('href', 'file?file_id=' + setdata['match_file_id_list'][i]);
      set_match_a.setAttribute('title', 'Match score = ' + setdata['score_list'][i]);
      set_match_a.appendChild(set_match_figure);
      set_panel.appendChild(set_match_a);
    }
    group_panel.appendChild(set_panel);
  }
}
