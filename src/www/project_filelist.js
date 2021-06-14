/**
 *
 * @desc code to build HTML user interface for /{PNAME}/filelist endpoint
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

var filelist_uri_args = [];
var content = document.createElement('div');
content.setAttribute('id', 'content');
var filelist_search_panel = document.createElement('div');
filelist_search_panel.setAttribute('class', 'filelist_search_panel');
var filelist_panel = document.createElement('div');
filelist_panel.setAttribute('class', 'filelist_panel');
content.appendChild(filelist_search_panel);
content.appendChild(filelist_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

const FILE_PER_PAGE = 25;

var search_panel_mode = document.createElement('input')
search_panel_mode.setAttribute('type', 'hidden');
search_panel_mode.setAttribute('name', 'mode');
search_panel_mode.setAttribute('value', _vise_data['FLIST_MODE']);

var help_panel = document.createElement('div');
help_panel.setAttribute('id', 'help_panel');
help_panel.setAttribute('class', 'hide');
help_panel.innerHTML = '<table><tr><th>Search Keyword</th><th>Result</th></tr><tr><td>Aesop</td><td>find metadata containing the exact word "Aesop" (misses Aesopus)</td></tr><tr><td>Aesop*</td><td>Finds metadata containing words starting with Aesop (e.g. Aesop, Aesopus, ...)</td></tr><tr><td>Aesopus <strong>AND</strong> Venice</td><td>finds all metadata containing keywords "Aesopus" and "Venice"</td></tr><tr><td>Aesopus <strong>OR</strong> Venice</td><td>finds all metadata either containing keyword "Aesopus" or keyword "Venice"</td></tr><tr><td>Aesopus <strong>NOT</strong> Venice</td><td>finds all metadata containing keyword "Aesopus" but not the keyword "Venice"</td></tr></table>';
/*
if(_vise_data.hasOwnProperty('METADATA_CONF')) {
  if(_vise_data['METADATA_CONF'].hasOwnProperty('file_attributes_id_list')) {
    help_panel.innerHTML += '<p>File attributes: ' + _vise_data['METADATA_CONF']['file_attributes_id_list'].join(', ') + '</p>';
  }
  if(_vise_data['METADATA_CONF'].hasOwnProperty('region_attributes_id_list')) {
    help_panel.innerHTML += '<p>Region attributes: ' + _vise_data['METADATA_CONF']['region_attributes_id_list'].join(', ') + '</p>';
  }
}
*/

var reset_form = document.createElement('form');
reset_form.setAttribute('class', 'hide');
reset_form.setAttribute('method', 'GET');
reset_form.setAttribute('action', 'filelist');
document.body.appendChild(reset_form);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'filelist');
  pname_link.setAttribute('title', 'Home page of ' + _vise_data.PNAME);
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
  // construct URI for HTTP POST
  if(_vise_data.hasOwnProperty('FLIST_MODE')) {
    filelist_uri_args.push('mode=' + _vise_data['FLIST_MODE']);
  }
  if(_vise_data.hasOwnProperty('FLIST_QUERY')) {
    filelist_uri_args.push('query=' + _vise_data['FLIST_QUERY']);
  }
  if(_vise_data.hasOwnProperty('FLIST_GROUP')) {
    filelist_uri_args.push('group=' + _vise_data['FLIST_GROUP']);
  }
  if(_vise_data.hasOwnProperty('FLIST_GROUPBY')) {
    filelist_uri_args.push('groupby=' + _vise_data['FLIST_GROUPBY']);
  }

  _vise_show_filelist_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_filelist_ui() {
  _vise_set_project_pagetools(pagetools);
  _vise_init_filelist_toolbar();
  _vise_init_filelist_search_toolbar();
  _vise_init_filelist_content();
}

function _vise_init_filelist_toolbar() {
  pageinfo.innerHTML = '';

  var sep = document.createElement('span');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var label1 = document.createElement('span');
  label1.innerHTML = '&nbsp;Showing';
  var start_input = document.createElement('input');
  start_input.setAttribute('type', 'text');
  start_input.setAttribute('style', 'width:2em;');
  start_input.setAttribute('value', _vise_data.FLIST_START);
  start_input.setAttribute('pattern', '[0-9]{1,10}');
  start_input.setAttribute('title', 'Enter the image index to show that image and remaining images.');
  start_input.addEventListener('change', function(e) {
    var new_start = parseInt(this.value);
	  if(isNaN(new_start)) {
	    this.value = _vise_data['FLIST_START'];
	    return;
	  }
	  new_start = new_start;
	  if(new_start < 0 || new_start >= _vise_data['FLIST_SIZE']) {
	    this.value = _vise_data.FLIST_START;
	    return;
	  } else {
      var new_end = Math.min(_vise_data['FLIST_SIZE'], new_start + FILE_PER_PAGE);
      window.location.href = 'filelist?start=' + new_start + '&end=' + new_end;
    }
  });

  var label2 = document.createElement('span');
  label2.innerHTML = 'to ' + (_vise_data['FLIST_END'] - 1) + ' of ' + _vise_data['FLIST_SIZE'] + ' files';
  if(_vise_data.hasOwnProperty('FLIST_GROUPBY')) {
    label2.innerHTML += ' in group&nbsp;';
  } else {
    label2.innerHTML += '.&nbsp;';
  }
  pageinfo.appendChild(label1);
  pageinfo.appendChild(start_input);
  pageinfo.appendChild(label2);

  if(_vise_data.hasOwnProperty('FLIST_GROUPBY')) {
    // group selector
    var group_form = document.createElement('form');
    group_form.setAttribute('method', 'GET');
    group_form.setAttribute('action', 'filelist');
    group_form.addEventListener('change', function(e) {
      e.target.parentNode.submit();
    });
    var group_select = document.createElement('select');
    group_select.setAttribute('name', 'group');
    var groupby_display_name = '';
    var groupby = _vise_data['FLIST_GROUPBY'];
    if(groupby in _vise_data['METADATA_CONF']['file_attributes']) {
      groupby_display_name = _vise_data['METADATA_CONF']['file_attributes'][groupby]['aname'];
    } else {
      if(groupby in _vise_data['METADATA_CONF']['region_attributes']) {
        groupby_display_name = _vise_data['METADATA_CONF']['region_attributes'][groupby]['aname'];
      }
    }

    for(var gindex in _vise_data['FLIST_GROUP_STAT']) {
      var group = _vise_data['FLIST_GROUP_STAT'][gindex]['group'];
      var group_size = _vise_data['FLIST_GROUP_STAT'][gindex]['size'];
      var oi = document.createElement('option');
      oi.setAttribute('value', group);
      oi.innerHTML = groupby_display_name + ' = ' + group + ' [' + group_size + ']';
      if(_vise_data['FLIST_GROUP'] === group) {
        oi.setAttribute('selected', '');
      }
      group_select.appendChild(oi);
    }
    var mode = document.createElement('input')
    mode.setAttribute('type', 'hidden');
    mode.setAttribute('name', 'mode');
    mode.setAttribute('value', search_panel_mode.getAttribute('value'));
    var query = document.createElement('input')
    query.setAttribute('type', 'hidden');
    query.setAttribute('name', 'query');
    if(_vise_data.hasOwnProperty('FLIST_QUERY')) {
      query.setAttribute('value', _vise_data['FLIST_QUERY']);
    } else {
      query.setAttribute('value', '');
    }
    var groupby = document.createElement('input')
    groupby.setAttribute('type', 'hidden');
    groupby.setAttribute('name', 'groupby');
    groupby.setAttribute('value', _vise_data['FLIST_GROUPBY']);

    group_form.appendChild(mode);
    group_form.appendChild(query);
    group_form.appendChild(groupby);
    group_form.appendChild(group_select);
    pageinfo.appendChild(group_form);
  }

  pageinfo.appendChild(sep.cloneNode(true));

  var prev_start = Math.max(0, _vise_data.FLIST_START - FILE_PER_PAGE);
  var prev_end = _vise_data['FLIST_START'];
  var prev;
  if(_vise_data['FLIST_START'] === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    prev.setAttribute('href', 'filelist?' + filelist_uri_args.join('&') + '&start=' + prev_start + '&end=' + prev_end);
  }
  prev.innerHTML = 'Prev';
  pageinfo.appendChild(prev);
  pageinfo.appendChild(sep.cloneNode(true));

  var next_start = _vise_data['FLIST_END'];
  var next_end = Math.min(_vise_data['FLIST_SIZE'], _vise_data['FLIST_END'] + FILE_PER_PAGE);
  var next;
  if(_vise_data['FLIST_END'] === _vise_data['FLIST_SIZE']) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    next.setAttribute('href', 'filelist?' + filelist_uri_args.join('&') + '&start=' + next_start + '&end=' + next_end);
  }
  next.innerHTML = 'Next';
  pageinfo.appendChild(next);
}

function _vise_init_filelist_search_toolbar() {
  var search_form = document.createElement('form');
  search_form.setAttribute('method', 'GET');
  search_form.setAttribute('action', 'filelist');
  search_form.setAttribute('id', 'search_form');
  var search_input = document.createElement('input');
  search_input.setAttribute('type', 'text');
  search_input.setAttribute('name', 'query');
  search_input.setAttribute('required', '');

  search_input.setAttribute('minlength', '2');
  if(_vise_data.hasOwnProperty('FLIST_QUERY')) {
    search_input.setAttribute('value', _vise_data['FLIST_QUERY']);
  }
  search_input.setAttribute('placeholder', 'Filter based on image filename (e.g. image01*.jpg)');
  search_input.addEventListener('change', function(e) {
    if(this.value === '') {
      search_panel_mode.setAttribute('value', 'all'); // switch to full text search mode
    } else {
      search_panel_mode.setAttribute('value', 'fts'); // switch to full text search mode
    }
  });

  var groupby_select = document.createElement('select');
  groupby_select.setAttribute('name', 'groupby');
  groupby_select.setAttribute('title', 'Group search results based on selected file or region metadata');
  var groupby_options = { '_none_':'No Groupings' };
  if(_vise_data['METADATA_CONF'].hasOwnProperty('groupby_aid_list')) {
    for(var aindex in _vise_data['METADATA_CONF']['groupby_aid_list']) {
      var aid = _vise_data['METADATA_CONF']['groupby_aid_list'][aindex];
      if(aid in _vise_data['METADATA_CONF']['file_attributes']) {
        groupby_options[aid] = _vise_data['METADATA_CONF']['file_attributes'][aid]['aname'];
      }
      if(aid in _vise_data['METADATA_CONF']['region_attributes']) {
        groupby_options[aid] = _vise_data['METADATA_CONF']['region_attributes'][aid]['aname'];
      }
    }
  } else {
    for(var fattr in _vise_data['METADATA_CONF']['file_attributes']) {
      if(fattr !== 'file_id' && fattr !== 'filename') {
        groupby_options[fattr] = _vise_data['METADATA_CONF']['file_attributes'][fattr]['aname'];
      }
    }
    for(var rattr in _vise_data['METADATA_CONF']['region_attributes']) {
      groupby_options[rattr] = _vise_data['METADATA_CONF']['region_attributes'][rattr]['aname'];
    }
  }
  for(var oid in groupby_options) {
    var oi = document.createElement('option');
    oi.setAttribute('value', oid);
    oi.innerHTML = groupby_options[oid];
    if(_vise_data.hasOwnProperty('FLIST_GROUPBY') &&
       _vise_data['FLIST_GROUPBY'] === oid) {
      oi.setAttribute('selected', '');
    }
    groupby_select.appendChild(oi);
  }
  groupby_select.addEventListener('change', function(e) {
    e.target.parentNode.submit();
  });

  var search_button = document.createElement('input');
  search_button.setAttribute('type', 'submit');
  search_button.setAttribute('value', 'Filter Image List');

  // help button
  var help_icon = _vise_common_get_svg_button('micon_help', 'More details about metadata search');
  var help_link = document.createElement('a');
  help_link.setAttribute('id', 'help_link');
  help_link.addEventListener('click', function() {
    document.getElementById('help_panel').classList.toggle('hide');
  });
  help_link.appendChild(help_icon);
  //search_form.appendChild(help_link); // 14-June-2021: hide until metadata search feature is release

  // clear button
  var clear_icon = _vise_common_get_svg_button('micon_clear', 'Clear all filters and go back to full file list.');
  var clear_link = document.createElement('a');
  clear_link.setAttribute('id', 'clear_link');
  clear_link.addEventListener('click', function(e) {
    reset_form.submit();
  });
  clear_link.appendChild(clear_icon);

  search_form.appendChild(search_panel_mode); // global variable
  search_form.appendChild(search_input);
  search_form.appendChild(search_button);

  // 14-June-2021: hide until metadata search feature is release
  //search_form.appendChild(groupby_select);
  //search_form.appendChild(clear_link);

  filelist_search_panel.innerHTML = '';
  filelist_search_panel.appendChild(search_form);
  filelist_search_panel.appendChild(help_panel);
}

function _vise_init_filelist_content() {
  var filelist = document.createElement('div');
  filelist.setAttribute('class', 'filelist');
  for(var i=0; i<_vise_data.FLIST_FILENAME.length; ++i) {
    var img = document.createElement('img');
    img.setAttribute('src', 'image_small/' + _vise_data.FLIST_FILENAME[i]);
    img.setAttribute('data-findex', i);
    var a = document.createElement('a');
    a.setAttribute('href', 'file?file_id=' + _vise_data.FLIST_FILE_ID[i]);
    a.setAttribute('title', '[' + _vise_data.FLIST_FILE_ID[i] + '] ' + _vise_data.FLIST_FILENAME[i] + ' : click to search using this file');
    a.appendChild(img);
    filelist.appendChild(a);
  }
  filelist_panel.innerHTML = '';
  filelist_panel.appendChild(filelist);
}
