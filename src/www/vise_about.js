/**
 *
 * @desc code to build HTML user interface for /about endpoint (i.e. About VISE)
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 10 Mar. 2020
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
var about_container = document.createElement('div');
about_container.setAttribute('id', 'about_container');
content.appendChild(about_container);

document.body.appendChild(toolbar);
document.body.appendChild(content);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  pname.innerHTML = '';

  document.title = 'About';
  _vise_settings_init();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_settings_init() {
  _vise_settings_init_toolbar();
  _vise_show_about();
}

function _vise_settings_init_toolbar() {
  pageinfo.innerHTML = '';

  var home_icon = _vise_common_get_svg_button('micon_home', 'Home');
  var home_link = document.createElement('a');
  home_link.setAttribute('href', '/home');
  home_link.appendChild(home_icon);

  var settings_icon = _vise_common_get_svg_button('micon_settings', 'Settings');
  var settings_link = document.createElement('a');
  settings_link.setAttribute('href', '/settings');
  settings_link.appendChild(settings_icon);

  var help_icon = _vise_common_get_svg_button('micon_help', 'About');
  var help_link = document.createElement('a');
  help_link.setAttribute('href', '/about');
  help_link.appendChild(help_icon);

  pageinfo.appendChild(home_link);
  pageinfo.appendChild(settings_link);
  pageinfo.appendChild(help_link);
}

function _vise_show_about() {
  about_container.innerHTML = '';
  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 600 200');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'id', 'vise_logo_large');

  var vise_name = document.createElement('p');
  vise_name.setAttribute('id', 'vise_name');
  vise_name.innerHTML = _vise_data.VISE_FULLNAME + ' - ' + _vise_data.VERSION;


  var vise_info = document.createElement('div');
  vise_info.setAttribute('id', 'vise_info');

  var p1 = document.createElement('p');
  p1.innerHTML = 'VGG Image Search Engine (VISE) is a standalone application for visual search of images. VISE has been developed by the Visual Geometry Group (<a href="http://www.robots.ox.ac.uk/~vgg/">VGG</a>) and released under the BSD <a href="https://opensource.org/licenses/BSD-3-Clause">license</a> which allows it to be useful for both academic projects and commercial applications.'
  var p2 = document.createElement('p');
  p2.innerHTML = 'VISE builds on the C++ codebase developed by <a href="http://www.robots.ox.ac.uk/~relja/">Relja Arandjelovic</a> during his DPhil / Postdoc at the Visual Geometry Group, Department of Engineering Science, University of Oxford. VISE is developed and maintained by <a href="mailto:adutta _at_ robots.ox.ac.uk">Abhishek Dutta</a>.';

  var p3 = document.createElement('p');
  p3.innerHTML = 'For more details, visit <a href="https://www.robots.ox.ac.uk/~vgg/software/vise/">https://www.robots.ox.ac.uk/~vgg/software/vise/</a>';

  vise_info.appendChild(p1);
  vise_info.appendChild(p2);
  vise_info.appendChild(p3);

  about_container.appendChild(vise_logo);
  about_container.appendChild(vise_name);
  about_container.appendChild(vise_info);
}
