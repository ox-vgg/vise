/**
 *
 * @desc code to build HTML user interface for /{PNAME}/visual_group endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 26 Jan. 2021
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
group_search_panel.setAttribute('class', 'visual_group_search_panel');
var group_panel = document.createElement('div');
group_panel.setAttribute('class', 'visual_group_panel');
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

  var group_icon = _vise_common_get_svg_button('micon_group', 'Show Visual Group');
  var group_link = document.createElement('a');
  if( _vise_data.GROUP.hasOwnProperty('group_id') ) {
    group_link.setAttribute('href', 'visual_group?group_id=' + _vise_data.GROUP['group_id']);
  } else {
    group_link.setAttribute('href', 'visual_group');
  }
  group_link.appendChild(group_icon);
  pagetools.appendChild(group_link);

  if(_vise_data.GROUP.hasOwnProperty('STATUS')) {
    if(_vise_data.GROUP['STATUS'] === 'error') {
      group_panel.innerHTML = '<h3>Error</h3><p>' + _vise_data.GROUP['MESSAGE'] + '</p>';
      return;
    }
    if(_vise_data.GROUP['STATUS'] === 'group_index') {
      var html = '<h3>Image Groups</h3><p>' + _vise_data.GROUP['MESSAGE'] + '</p><ul>';
      for(var i=0; i<_vise_data.GROUP['group_id_list'].length; ++i) {
        var group_id = _vise_data.GROUP['group_id_list'][i];
        html += '<li><a href="visual_group?group_id=' + group_id + '">' + group_id + '</a></li>';
      }
      html += '</ul>'
      group_panel.innerHTML = html;
      return;
    }
    group_panel.innerHTML = '<h3>Unknown Status</h3><p>STATUS = ' + _vise_data.GROUP['STATUS'] + '</p>';
    return;
  }

  if(_vise_data.GROUP.hasOwnProperty('set_id') &&
     _vise_data.GROUP.hasOwnProperty('file_id_list') &&
     _vise_data.GROUP.hasOwnProperty('filename_list') ) {
    // show details of a set from the visual_group
    _vise_init_set_content();
  } else {
    if(_vise_data.GROUP.hasOwnProperty('file_id') &&
       _vise_data.GROUP.hasOwnProperty('set_id_list')) {
      // show a list of sets that contain a file-id
      _vise_init_set_file_id_content();
    } else {
      // show visual_group filtered by set_size
      _vise_init_group_toolbar();
      _vise_init_group_filter_toolbar();
      _vise_init_group_content();
    }
  }
}

function _vise_init_group_toolbar() {
  pageinfo.innerHTML = '';
  if(_vise_data.GROUP['set_index_list'].length === 0) {
    return;
  }
  var sep = document.createElement('span');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var label1 = document.createElement('span');
  label1.innerHTML = '&nbsp;Showing sets from ';
  var start_input = document.createElement('input');
  start_input.setAttribute('type', 'text');
  start_input.setAttribute('style', 'width:4em;');
  start_input.setAttribute('value', _vise_data.GROUP['set_index_list'][0]);
  start_input.setAttribute('pattern', '[0-9]{1,10}');
  start_input.setAttribute('title', 'Enter the set index to show that set and remaining some sets.');
  start_input.addEventListener('change', function(e) {
    var new_start = parseInt(this.value);
	  if(isNaN(new_start)) {
	    this.value = _vise_data.GROUP['set_index_from'];
	    return;
	  }
	  new_start = new_start;
	  if(new_start < _vise_data.GROUP['set_index_range'][0] || new_start >= _vise_data.GROUP['set_index_range'][1]) {
	    this.value = _vise_data.GROUP['set_index_from'];
	    return;
	  } else {
      var SET_PER_PAGE = _vise_data.GROUP['set_index_list'].length;
      var new_end = Math.min(_vise_data.GROUP['set_index_range'][1], new_start + SET_PER_PAGE);
      var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
      url.push('set_size=' + _vise_data.GROUP['set_size']);
      url.push('from=' + new_start);
      url.push('to=' + new_end);
      window.location.href = url.join('&');
    }
  });

  var label2 = document.createElement('span');
  var SET_PER_PAGE = _vise_data.GROUP['set_index_list'].length;

  var set_index_list_count = _vise_data.GROUP['set_index_list'].length;
  var last_set_id = _vise_data.GROUP['set_index_list'][set_index_list_count-1];
  label2.innerHTML = 'to ' + _vise_data.GROUP['set_index_to'] + ' of ' + _vise_data.GROUP['set_index_range'][1] + ' sets.&nbsp;';

  pageinfo.appendChild(label1);
  pageinfo.appendChild(start_input);
  pageinfo.appendChild(label2);

  pageinfo.appendChild(sep.cloneNode(true));

  var prev_start = Math.max(_vise_data.GROUP['set_index_range'][0], _vise_data.GROUP['set_index_from'] - SET_PER_PAGE);
  var prev_end = _vise_data.GROUP['set_index_from'];
  var prev;
  if(_vise_data.GROUP['set_index_from'] === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + _vise_data.GROUP['set_size']);
    url.push('from=' + prev_start);
    url.push('to=' + prev_end);
    prev.setAttribute('href', url.join('&'));
  }
  prev.innerHTML = 'Prev';
  pageinfo.appendChild(prev);
  pageinfo.appendChild(sep.cloneNode(true));

  var next_start = _vise_data.GROUP['set_index_to'];
  var next_end = Math.min(_vise_data.GROUP['set_index_range'][1], _vise_data.GROUP['set_index_to'] + SET_PER_PAGE);
  var next;
  if(_vise_data.GROUP['set_index_to'] === _vise_data.GROUP['set_index_range'][1]) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + _vise_data.GROUP['set_size']);
    url.push('from=' + next_start);
    url.push('to=' + next_end);
    next.setAttribute('href', url.join('&'));
  }
  next.innerHTML = 'Next';
  pageinfo.appendChild(next);
}

function _vise_init_group_filter_toolbar() {
  var jumpinfo_container = document.createElement('div');
  var jumpinfo1 = document.createElement('span');
  jumpinfo1.innerHTML = 'Show Set Id.&nbsp;';
  var set_id_input = document.createElement('input');
  set_id_input.setAttribute('type', 'text');
  set_id_input.setAttribute('style', 'width:6em; ');
  var sample_set_id = Math.floor((_vise_data.GROUP['set_id_range'][1] - _vise_data.GROUP['set_id_range'][0])/2)
  set_id_input.setAttribute('placeholder', 'e.g. ' + sample_set_id );
  set_id_input.setAttribute('pattern', '[0-9]{1,10}');
  set_id_input.setAttribute('title', 'Enter a set id between ' + _vise_data.GROUP['set_id_range'][0] + ' and ' + _vise_data.GROUP['set_id_range'][1] + ' to show the set');
  set_id_input.addEventListener('change', function(e) {
    var new_set_id = parseInt(this.value);
	  if(isNaN(new_set_id)) {
	    this.value = _vise_data.GROUP['set_id'];
	    return;
	  }
	  if(new_set_id < _vise_data.GROUP['set_id_range'][0] || new_set_id > _vise_data.GROUP['set_id_range'][1]) {
	    this.value = _vise_data.GROUP['set_id'];
	    return;
	  } else {
      var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
      url.push('set_id=' + new_set_id);
      window.location.href = url.join('&');
    }
  });
  jumpinfo_container.innerHTML = '';
  jumpinfo_container.appendChild(jumpinfo1);
  jumpinfo_container.appendChild(set_id_input);

  // show set containing a file
  var fileset_container = document.createElement('div');
  var fileset_info = document.createElement('span');
  fileset_info.innerHTML = 'or, show all the sets containing the file id.&nbsp;';
  var file_id_input = document.createElement('input');
  file_id_input.setAttribute('type', 'text');
  file_id_input.setAttribute('style', 'width:6em; ');
  var sample_file_id = Math.floor(_vise_data['FLIST_SIZE']/2);
  file_id_input.setAttribute('placeholder', 'e.g. ' + sample_file_id );
  file_id_input.setAttribute('pattern', '[0-9]{1,10}');
  file_id_input.setAttribute('title', 'Enter a file id between 0 and ' + _vise_data['FLIST_SIZE'] + ' to show the visual group sets that contains the file.');
  file_id_input.addEventListener('change', function(e) {
    var new_file_id = parseInt(this.value);
	  if(isNaN(new_file_id)) {
	    this.value = '';
	    return;
	  }
	  if(new_file_id < 0 || new_file_id >= _vise_data['FLIST_SIZE']) {
	    this.value = '';
	    return;
	  } else {
      var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
      url.push('file_id=' + new_file_id);
      window.location.href = url.join('&');
    }
  });
  fileset_container.innerHTML = '';
  fileset_container.appendChild(fileset_info);
  fileset_container.appendChild(file_id_input);

  // set size
  var set_size_container = document.createElement('div');
  var set_size_input = document.createElement('select');
  set_size_input.setAttribute('name', 'set_size');
  set_size_input.setAttribute('id', 'set_size');
  for(var set_size in _vise_data.GROUP['set_size_stat']) {
    var set_size_member_count = _vise_data.GROUP['set_size_stat'][set_size];
    var oi = document.createElement('option');
    oi.setAttribute('value', set_size);
    oi.innerHTML = 'Showing sets with ' + set_size + ' images';
    if(set_size_member_count === 1) {
      oi.innerHTML += ' (only 1 such set is available)';
    } else {
      oi.innerHTML += ' (total ' + set_size_member_count + ' such sets are available)';
    }
    if(parseInt(set_size) === _vise_data.GROUP['set_size']) {
      oi.setAttribute('selected', '');
    }
    set_size_input.appendChild(oi);
  }
  set_size_input.addEventListener('change', function(e) {
    var new_set_size = this.options[this.selectedIndex].value;
    var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_size=' + new_set_size);
    window.location.href = url.join('&');
  });
  set_size_container.appendChild(set_size_input);

  group_search_panel.innerHTML = '';
  group_search_panel.appendChild(jumpinfo_container);
  group_search_panel.appendChild(fileset_container);
  group_search_panel.appendChild(set_size_container);
}

function _vise_init_group_content() {
  if(!_vise_data.hasOwnProperty("GROUP")) {
    group_panel.innerHTML = 'Missing group data';
    return;
  }

  group_panel.innerHTML = '';
  if(_vise_data.GROUP['set_index_list'].length === 0) {
    group_panel.innerHTML = '<p>No results found.</p>';
    return;
  }
  for(var i in _vise_data.GROUP['set_index_list']) {
    var set_index = _vise_data.GROUP['set_index_list'][i];
    var setdata = _vise_data.GROUP.SET[set_index];

    var set_panel = document.createElement('div');
    set_panel.setAttribute('class', 'set_panel');

    // title
    var set_info_container = document.createElement('div');
    set_info_container.setAttribute('class', 'set_info');
    var set_index_container = document.createElement('span');
    set_index_container.setAttribute('class', 'set_index');
    set_index_container.innerHTML = set_index;
    var set_id_container = document.createElement('span');
    set_id_container.setAttribute('class', 'set_id');
    set_id_container.setAttribute('title', 'Permanent link to this set');
    var set_url = '<a href="visual_group?group_id='  + _vise_data.GROUP['group_id'] + '&set_id=' + setdata['set_id'] + '">Set Id. ' + setdata['set_id'] + '</a>';
    set_id_container.innerHTML = set_url;
    var set_spec_download_icon = _vise_common_get_svg_button('micon_download', 'Download specifications of this set (e.g. filenames, set id, etc.) as a text file.');
    var set_spec_download_link = document.createElement('span');
    set_spec_download_link.setAttribute('onclick', '_vise_download_set_specifications(' + set_index + ')');
    set_spec_download_link.appendChild(set_spec_download_icon);

    set_info_container.appendChild(set_index_container);
    set_info_container.appendChild(set_id_container);
    set_info_container.appendChild(set_spec_download_link);
    set_panel.appendChild(set_info_container);

    // match
    var set_matches = document.createElement('div');
    set_matches.setAttribute('class', 'set_matches');
    for (var i=0; i<setdata['filename_list'].length; ++i) {
      //var set_match_figure = document.createElement('figure');
      var set_match_img = document.createElement('img');
      set_match_img.setAttribute('src', 'image/' + setdata['filename_list'][i]);
      set_match_img.setAttribute('data-setindex', set_index);
      set_match_img.setAttribute('data-fileindex', i);
      set_match_img.addEventListener('load', _vise_on_img_load_highlight_set_img_region);
      var set_match_a = document.createElement('a');
      set_match_a.setAttribute('href', 'file?file_id=' + setdata['file_id_list'][i]);
      set_match_a.setAttribute('title', '[' + setdata['file_id_list'][i] + '] : ' + setdata['filename_list'][i]);
      set_match_a.appendChild(set_match_img);
      set_panel.appendChild(set_match_a);
    }

    group_panel.appendChild(set_panel);
  }
}

function _vise_init_set_content() {
  group_panel.innerHTML = '';
  if(_vise_data.GROUP['file_id_list'].length === 0) {
    group_panel.innerHTML = '<p>Empty set.</p>';
    return;
  }

  var setdata = _vise_data.GROUP;
  var set_panel = document.createElement('div');
  set_panel.setAttribute('class', 'set_panel');

  // pageinfo
  var pageinfo1 = document.createElement('span');
  pageinfo1.innerHTML = 'Showing set&nbsp;';
  var set_id_input = document.createElement('input');
  set_id_input.setAttribute('type', 'text');
  set_id_input.setAttribute('style', 'width:4em;');
  set_id_input.setAttribute('value', _vise_data.GROUP['set_id']);
  set_id_input.setAttribute('pattern', '[0-9]{1,10}');
  set_id_input.setAttribute('title', 'Enter the set id to show that set.');
  set_id_input.addEventListener('change', function(e) {
    var new_set_id = parseInt(this.value);
	  if(isNaN(new_set_id)) {
	    this.value = _vise_data.GROUP['set_id'];
	    return;
	  }
	  if(new_set_id < _vise_data.GROUP['set_id_range'][0] || new_set_id > _vise_data.GROUP['set_id_range'][1]) {
	    this.value = _vise_data.GROUP['set_id'];
	    return;
	  } else {
      var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
      url.push('set_id=' + new_set_id);
      window.location.href = url.join('&');
    }
  });
  var pageinfo2 = document.createElement('span');
  var set_id_length = _vise_data.GROUP['set_id_range'][1] - _vise_data.GROUP['set_id_range'][0];
  pageinfo2.innerHTML = ' of ' + set_id_length + ' sets in image group [' + setdata['group_id'] + '].';
  pageinfo.innerHTML = '';
  pageinfo.appendChild(pageinfo1);
  pageinfo.appendChild(set_id_input);
  pageinfo.appendChild(pageinfo2);

  // title
  var set_info_container = document.createElement('div');
  set_info_container.setAttribute('class', 'set_info');
  var set_id_container = document.createElement('span');
  set_id_container.setAttribute('class', 'set_id');
  set_id_container.setAttribute('title', 'Unique identifier for this set');
  set_id_container.innerHTML = 'Set Id. ' + setdata['set_id'];
  set_info_container.appendChild(set_id_container);
  set_panel.appendChild(set_info_container);

  // match
  for (var i=0; i<setdata['filename_list'].length; ++i) {
    var set_match_img = document.createElement('img');
    set_match_img.setAttribute('data-fileindex', i);
    set_match_img.setAttribute('src', 'image/' + setdata['filename_list'][i]);
    set_match_img.addEventListener('load', _vise_on_img_load_highlight_img_region);

    var set_match_a = document.createElement('a');
    set_match_a.setAttribute('href', 'file?file_id=' + setdata['file_id_list'][i]);
    set_match_a.setAttribute('title', '[' + setdata['file_id_list'][i] + '] ' + setdata['filename_list'][i]);
    set_match_a.appendChild(set_match_img);
    set_panel.appendChild(set_match_a);
  }

  group_panel.appendChild(set_panel);
}

function _vise_download_set_specifications(set_index) {
  var setdata = _vise_data.GROUP.SET[set_index];
  var set_id = setdata['set_id'];
  var csv = [];
  csv.push('project_id,visual_group_id,set_id,file_id,filename');
  for (var i=0; i<setdata['filename_list'].length; ++i) {
    var line = [];
    line.push(_vise_data['PNAME']);
    line.push(_vise_data['GROUP']['group_id']);
    line.push(set_id);
    line.push(setdata['file_id_list'][i]);
    line.push('"' + setdata['filename_list'][i] + '"');
    csv.push(line.join(','))
  }
  var csv_blob = new Blob( [csv.join('\n')], {type:'text/csv;charset=utf-8'} );
  var csv_filename = _vise_data['PNAME'] + '-' + _vise_data['GROUP']['group_id'] + '-set' + set_id + '.csv';
  _vise_save_data_to_local_file(csv_blob, csv_filename)
}

function _vise_init_set_file_id_content() {
  group_panel.innerHTML = '';
  if(_vise_data.GROUP['set_id_list'].length === 0) {
    group_panel.innerHTML = '<p>Empty set.</p>';
    return;
  }

  group_search_panel.innerHTML = '';
  var file_id = _vise_data.GROUP['file_id'];
  var file_id_link = '<a href="file?file_id=' + file_id + '">' + file_id + '</a>';
  var info = document.createElement('span');
  info.innerHTML = 'File id ' + file_id_link + ' is contained in following sets:';
  group_search_panel.appendChild(info);
  for(var i=0; i<_vise_data.GROUP['set_id_list'].length; ++i) {
    var setid = _vise_data.GROUP['set_id_list'][i];
    var span = document.createElement('span');
    var url = [ 'visual_group?group_id=' + _vise_data.GROUP['group_id'] ];
    url.push('set_id=' + setid);
    if(i !== 0) {
      span.innerHTML = ',&nbsp;';
    } else {
      span.innerHTML = '&nbsp;';
    }
    span.innerHTML += '<a href="' + url.join('&') + '">' + setid + '</a>';
    group_search_panel.appendChild(span);
  }
}

function _vise_on_img_load_highlight_img_region(e) {
  var file_index = parseInt(e.target.dataset.fileindex);
  if('region_points_list' in _vise_data.GROUP.SET[set_index]) {

    var svg = document.createElementNS(_VISE_SVG_NS, 'svg');
    svg.setAttribute('style', 'height:' + e.target.height + 'px;width:' + e.target.width + 'px;');

    var x = _vise_data.GROUP['region_points_list'][file_index][0];
    var y = _vise_data.GROUP['region_points_list'][file_index][1];
    var width = _vise_data.GROUP['region_points_list'][file_index][2];
    var height = _vise_data.GROUP['region_points_list'][file_index][3];
    var scale = e.target.height / e.target.naturalHeight;

    var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
    rshape.setAttribute('x', Math.floor(x*scale));
    rshape.setAttribute('y', Math.floor(y*scale));
    rshape.setAttribute('width', Math.floor(width*scale));
    rshape.setAttribute('height', Math.floor(height*scale));

    svg.appendChild(rshape);
    e.target.parentNode.appendChild(svg);
  }
}

function _vise_on_img_load_highlight_set_img_region(e) {
  var set_index = parseInt(e.target.dataset.setindex);
  var file_index = parseInt(e.target.dataset.fileindex);
  if('region_points_list' in _vise_data.GROUP.SET[set_index]) {
    var svg = document.createElementNS(_VISE_SVG_NS, 'svg');
    svg.setAttribute('style', 'height:' + e.target.height + 'px;width:' + e.target.width + 'px;');

    var x = _vise_data.GROUP.SET[set_index]['region_points_list'][file_index][0];
    var y = _vise_data.GROUP.SET[set_index]['region_points_list'][file_index][1];
    var width = _vise_data.GROUP.SET[set_index]['region_points_list'][file_index][2];
    var height = _vise_data.GROUP.SET[set_index]['region_points_list'][file_index][3];
    var scale = e.target.height / e.target.naturalHeight;

    var rshape = document.createElementNS(_VISE_SVG_NS, 'rect');
    rshape.setAttribute('x', Math.floor(x*scale));
    rshape.setAttribute('y', Math.floor(y*scale));
    rshape.setAttribute('width', Math.floor(width*scale));
    rshape.setAttribute('height', Math.floor(height*scale));

    svg.appendChild(rshape);
    e.target.parentNode.appendChild(svg);
  }
}
