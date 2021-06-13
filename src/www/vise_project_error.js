/**
 *
 * @desc code to create VISE project error page
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 30 Nov. 2020
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

document.body.appendChild(toolbar);
document.body.appendChild(content);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 240 80');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'style', 'height:0.8em; padding-right:1em;');

  var home_link = document.createElement('a');
  home_link.setAttribute('href', 'home');
  home_link.appendChild(vise_logo);

  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'filelist');
  pname_link.setAttribute('title', 'Home of ' + _vise_data.PNAME + ' project');
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(home_link);
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
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

  var tools = document.createElement('p');
  var back = document.createElement('button');
  back.innerHTML = 'Back';
  back.setAttribute('onclick', 'history.back();');
  tools.appendChild(back);

  content.appendChild(error);
  content.appendChild(tools);
}
