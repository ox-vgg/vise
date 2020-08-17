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
var pageinfo = document.createElement('div');
pageinfo.setAttribute('class', 'pageinfo');
toolbar.appendChild(pageinfo);

var content = document.createElement('div');
content.setAttribute('id', 'content');
var filelist_panel = document.createElement('div');
filelist_panel.setAttribute('class', 'filelist_panel');
content.appendChild(filelist_panel);

document.body.appendChild(toolbar);
document.body.appendChild(content);

const FILE_PER_PAGE = 50;

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
  _vise_show_filelist_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_filelist_ui() {
  _vise_init_filelist_toolbar();
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
  start_input.setAttribute('title', 'Enter the page number to jump to that page');
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
      window.location.href = '/' + _vise_data['PNAME'] + '/filelist?start=' + new_start + '&end=' + new_end;
    }
  });

  var label2 = document.createElement('span');
  label2.innerHTML = 'to ' + _vise_data['FLIST_END'] + '&nbsp;';
  pageinfo.appendChild(label1);
  pageinfo.appendChild(start_input);
  pageinfo.appendChild(label2);
  pageinfo.appendChild(sep.cloneNode(true));

  var prev_start = Math.max(0, _vise_data.FLIST_START - FILE_PER_PAGE);
  var prev_end = _vise_data['FLIST_START'];
  var prev;
  if(_vise_data['FLIST_START'] === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    prev.setAttribute('href', 'filelist?start=' + prev_start + '&end=' + prev_end);
  }
  prev.innerHTML = 'Prev';
  pageinfo.appendChild(prev);
  pageinfo.appendChild(sep.cloneNode(true));

  var next_start = _vise_data['FLIST_END'];
  var next_end = Math.min(_vise_data['FLIST_SIZE'], _vise_data['FLIST_END'] + FILE_PER_PAGE);
  console.log('next_end=' + next_end)
  var next;
  if(_vise_data['FLIST_END'] === _vise_data['FLIST_SIZE']) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    next.setAttribute('href', 'filelist?start=' + next_start + '&end=' + next_end);
  }
  next.innerHTML = 'Next';
  pageinfo.appendChild(next);
}

function _vise_init_filelist_content() {
  var filelist = document.createElement('div');
  filelist.setAttribute('class', 'filelist');
  for(var i=0; i<_vise_data.FLIST.length; ++i) {
    var img = document.createElement('img');
    img.setAttribute('src', '/' + _vise_data.PNAME + '/' + _vise_data.FLIST[i]);
    img.setAttribute('data-findex', i);
    var a = document.createElement('a');
    a.setAttribute('href', '/' + _vise_data.PNAME + '/file?file_id=' + (_vise_data.FLIST_START + i));
    a.setAttribute('title', '[' + (_vise_data.FLIST_START + i) + '] ' + _vise_data.FLIST[i] + ' : click to search using this file');
    a.appendChild(img);
    filelist.appendChild(a);
  }
  filelist_panel.innerHTML = '';
  filelist_panel.appendChild(filelist);
}
