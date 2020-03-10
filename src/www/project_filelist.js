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
  var MAX_PAGENO = Math.floor(_vise_data.FLIST_SIZE/_vise_data.FLIST_PER_PAGE) + 1;
  var CUR_PAGENO = _vise_data.FLIST_START/_vise_data.FLIST_PER_PAGE;

  var sep = document.createElement('span');
  sep.innerHTML = '&nbsp;|&nbsp;';

  var label1 = document.createElement('span');
  label1.innerHTML = '&nbsp;Showing page';
  var pgno = document.createElement('input');
  pgno.setAttribute('type', 'text');
  pgno.setAttribute('style', 'width:2em;');
  pgno.setAttribute('value', (CUR_PAGENO + 1));
  pgno.setAttribute('data-file_per_page', _vise_data.FLIST_PER_PAGE);
  pgno.setAttribute('data-max_page', MAX_PAGENO);
  pgno.setAttribute('data-pname', _vise_data.PNAME);
  pgno.setAttribute('data-filecount', _vise_data.FLIST_SIZE);
  pgno.setAttribute('title', 'Enter the page number to jump to that page');
  pgno.addEventListener('change', function(e) {
    var pname = e.target.dataset['pname'];
    var max_page = parseInt(e.target.dataset['max_page']);
    var file_per_page = parseInt(e.target.dataset['file_per_page']);
    var new_pgno = parseInt(e.target.value) - 1;
    var filecount = parseInt(e.target.dataset['filecount']);
    if(new_pgno >= 0 && new_pgno < max_page) {
      var new_start = file_per_page*new_pgno;
      var new_end = Math.min(filecount, new_start + file_per_page);
      window.location.href = '/' + pname + '/filelist?start=' + new_start + '&end=' + new_end + '&per_page=' + file_per_page;
    } else {
      e.target.value = '';
    }
  });

  var label2 = document.createElement('span');
  label2.innerHTML = 'of ' + MAX_PAGENO + ' pages&nbsp;';
  pageinfo.appendChild(label1);
  pageinfo.appendChild(pgno);
  pageinfo.appendChild(label2);
  pageinfo.appendChild(sep.cloneNode(true));

  var prev_start = Math.max(0, _vise_data.FLIST_START - _vise_data.FLIST_PER_PAGE);
  var prev_end = _vise_data.FLIST_START;
  var prev;
  if(_vise_data.FLIST_START === 0) {
    prev = document.createElement('span');
  } else {
    prev = document.createElement('a');
    prev.setAttribute('href', 'filelist?start=' + prev_start + '&end=' + prev_end);
  }
  prev.innerHTML = 'Prev';
  pageinfo.appendChild(prev);
  pageinfo.appendChild(sep.cloneNode(true));

  var next_start = _vise_data.FLIST_END;
  var next_end = Math.min(_vise_data.FLIST_SIZE, _vise_data.FLIST_END + _vise_data.FLIST_PER_PAGE);
  var next;
  if(_vise_data.FLIST_END === _vise_data.FLIST_SIZE) {
    next = document.createElement('span');
  } else {
    next = document.createElement('a');
    if(next_end === _vise_data.FLIST_SIZE) {
      // last page
      next.setAttribute('href', 'filelist?start=' + next_start + '&end=' + next_end + '&per_page=' + _vise_data.FLIST_PER_PAGE);
    } else {
      next.setAttribute('href', 'filelist?start=' + next_start + '&end=' + next_end);
    }
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
