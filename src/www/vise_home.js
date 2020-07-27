/**
 *
 * @desc code to build HTML user interface for /home endpoint (i.e. VISE home)
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
  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 240 80');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'class', 'vise_logo');

  var home_link = document.createElement('a');
  home_link.setAttribute('href', 'https://www.robots.ox.ac.uk/~vgg/software/vise/');
  home_link.setAttribute('target', '__blank');
  home_link.appendChild(vise_logo);

  pname.innerHTML = '';
  pname.appendChild(home_link);

  document.title = 'VISE';
  _vise_home_init();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_home_init() {
  _vise_home_init_toolbar();
  _vise_home_show_project_create_panel();
  _vise_home_show_project_list();
}

function _vise_home_init_toolbar() {
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

function _vise_home_show_project_create_panel() {
  var newproject = document.createElement('div');
  newproject.setAttribute('id', 'create_project_panel');

  var form = document.createElement('form');
  form.setAttribute('method', 'POST');
  form.setAttribute('action', '/_project_create');
  var pname = document.createElement('input');
  pname.setAttribute('type', 'text');
  pname.setAttribute('name', 'pname');
  pname.setAttribute('placeholder', 'e.g. 15th-Century-Books');
  form.appendChild(pname);

  var test = document.createElement('input');
  test.setAttribute('type', 'hidden');
  test.setAttribute('name', 'test');
  test.setAttribute('value', 'adutta');
  form.appendChild(test);

  var create = document.createElement('button');
  create.setAttribute('type', 'submit');
  create.innerHTML = 'Create New Project';
  form.appendChild(create);
  newproject.appendChild(form);

  var message = document.createElement('p');
  message.innerHTML = 'To search a collection of images, create a new project, add those images to this new project.'
  newproject.appendChild(message);

  content.appendChild(newproject);
}

function _vise_home_show_project_list() {
  var existing_projects_panel = document.createElement('div');
  existing_projects_panel.setAttribute('id', 'existing_projects_panel');

  if(Object.keys(_vise_data.PROJECT_LIST).length) {
    var title = document.createElement('h3');
    title.innerHTML = 'Existing Projects';
    existing_projects_panel.appendChild(title);

    for(var pname in _vise_data.PROJECT_LIST) {
      var a = document.createElement('a');
      a.setAttribute('href', '/' + pname + '/');
      var c = document.createElement('div');
      c.setAttribute('class', 'project');
      var img = document.createElement('img');
      img.setAttribute('src', '/' + pname + '/_cover_image');
      var imgcontainer = document.createElement('div');
      imgcontainer.setAttribute('class', 'imgcontainer');
      imgcontainer.appendChild(img);
      c.appendChild(imgcontainer);
      var desc = document.createElement('p');
      desc.innerHTML = pname;
      c.appendChild(desc);
      a.appendChild(c);
      existing_projects_panel.appendChild(a);
    }
  } else {

  }
  content.appendChild(existing_projects_panel);
}
