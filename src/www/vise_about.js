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
  home_link.setAttribute('href', 'home');
  home_link.appendChild(home_icon);

  var settings_icon = _vise_common_get_svg_button('micon_settings', 'Settings');
  var settings_link = document.createElement('a');
  settings_link.setAttribute('href', 'settings');
  settings_link.appendChild(settings_icon);

  var help_icon = _vise_common_get_svg_button('micon_help', 'About');
  var help_link = document.createElement('a');
  help_link.setAttribute('href', 'about');
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
  p1.innerHTML = 'VGG Image Search Engine (VISE) is a free and <a href="https://gitlab.com/vgg/vise">open source</a> software for visual search of large collection of images using image region as a search query. VISE is developed and maintained by Visual Geometry Group (<a href="https://www.robots.ox.ac.uk/~vgg/">VGG</a>) in Department of Engineering Science of the Oxford University. VISE is released under a <a href="https://gitlab.com/vgg/vise/-/blob/master/src/LICENSE.txt">license</a> that allows unrestricted use in academic research projects and commercial industrial applications.';

  var p2 = document.createElement('p');
  p2.innerHTML = 'VISE builds on the C++ codebase developed by <a href="http://www.robots.ox.ac.uk/~relja/">Dr. Relja Arandjelovic</a> during his DPhil / Postdoc at the Visual Geometry Group in 2014. VISE is developed and maintained by <a href="mailto:adutta _at_ robots.ox.ac.uk">Dr. Abhishek Dutta</a>. <a href="https://www.robots.ox.ac.uk/~az/">Prof. Andrew Zisserman</a> guides and supports development of VISE.';

  var p3 = document.createElement('p');
  p3.innerHTML = 'Development and maintenance of VISE software has been supported by the following research grants:';
  var ul = document.createElement('ul');
  ul.innerHTML = '<li><a href="https://gtr.ukri.org/projects?ref=EP%2FT028572%2F1">Visual AI</a>: An Open World Interpretable Visual Transformer (EPSRC)</li><li><a href="https://gow.epsrc.ukri.org/NGBOViewGrant.aspx?GrantRef=EP/M013774/1">Seebibyte</a>: Visual Search for the Era of Big Data (EPSRC)</li>';
  p3.appendChild(ul);

  var p4 = document.createElement('p');
  p4.innerHTML = 'For more details, see the VISE software page at: <a href="https://www.robots.ox.ac.uk/~vgg/software/vise/">https://www.robots.ox.ac.uk/~vgg/software/vise/</a>.';

  vise_info.appendChild(p1);
  vise_info.appendChild(p2);
  vise_info.appendChild(p3);
  vise_info.appendChild(p4);

  var vise_links = document.createElement('div');
  vise_links.setAttribute('id', 'vise_links');
  vise_links.innerHTML = '<a href="https://gitlab.com/vgg/vise/">Code Repository</a> | <a href="https://gitlab.com/vgg/vise/-/blob/master/src/LICENSE_3rd_party.txt">Software Library Licenses</a>';

  about_container.appendChild(vise_logo);
  about_container.appendChild(vise_name);
  about_container.appendChild(vise_info);
  about_container.appendChild(vise_links);
}
