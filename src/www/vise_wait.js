/**
 *
 * @desc code to create VISE error page
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 20 Feb. 2020
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
  home_link.setAttribute('href', '../index.html');
  home_link.appendChild(home_icon);

  pname.innerHTML = '';
  pname.appendChild(home_link);

  document.title = 'VISE';
  _vise_error_init();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_error_init() {
  _vise_error_init_toolbar();
  _vise_error_init_message();
}

function _vise_error_init_toolbar() {
  pageinfo.innerHTML = '';
}

function _vise_error_init_message() {
  content.innerHTML = '';
  var error = document.createElement('div');
  error.setAttribute('id', 'error_message');
  var title = document.createElement('h3');
  title.innerHTML = _vise_data.STATUS;
  error.appendChild(title);

  var message = document.createElement('p');
  message.innerHTML = _vise_data.MESSAGE;
  error.appendChild(message);

  content.appendChild(error);
}
