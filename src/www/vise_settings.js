/**
 *
 * @desc code to build HTML user interface for /settings endpoint (i.e. VISE Settings)
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
var settings_container = document.createElement('div');
settings_container.setAttribute('id', 'settings_container');
var admintool_container = document.createElement('div');
admintool_container.setAttribute('id', 'admintool_container');
content.appendChild(settings_container);
content.appendChild(admintool_container);

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
  home_link.setAttribute('href', '/home');
  home_link.appendChild(vise_logo);

  pname.innerHTML = '';
  pname.appendChild(home_link);

  document.title = 'Settings';
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
  _vise_show_settings();
  _vise_show_admintool();
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

function _vise_show_settings() {
  settings_container.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = 'VISE Settings';
  settings_container.appendChild(title);

  var table = document.createElement('table');
  for(var key in _vise_data.SETTINGS) {
    var desc = _vise_settings_get_var_desc(key);

    var tr = document.createElement('tr');
    var col1 = document.createElement('td');
    col1.innerHTML = desc[0];
    col1.setAttribute('title', desc[1]);

    var col2 = document.createElement('td');
    var input = document.createElement('input');
    input.setAttribute('type', 'text');
    input.setAttribute('name', key);
    input.setAttribute('value', _vise_data.SETTINGS[key]);
    input.setAttribute('title', _vise_data.SETTINGS[key]);
    col2.appendChild(input);

    tr.appendChild(col1);
    tr.appendChild(col2);
    table.appendChild(tr);
  }

  var control = document.createElement('tr');
  var col = document.createElement('td');
  col.setAttribute('colspan', '2');
  var save = document.createElement('button');
  save.innerHTML = 'Save Settings';
  save.addEventListener('click', _vise_on_save_settings);

  var config_save_status = document.createElement('span');
  config_save_status.setAttribute('id', 'config_save_status');
  col.appendChild(save);
  col.appendChild(config_save_status);
  control.appendChild(col);
  table.appendChild(control);

  settings_container.appendChild(table);
}

function _vise_on_save_settings() {
  var input = settings_container.getElementsByTagName("input");

  var formdata = [];
  for(var i=0; i<input.length; ++i) {
    if(input[i].getAttribute('type') === 'text') {
      var key = input[i].getAttribute('name');
      var val = input[i].value;
      formdata.push(key + '=' + val);
      _vise_data.SETTINGS[key] = val;
    }
  }

  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
    switch(this.status) {
    case 200:
      document.getElementById('config_save_status').innerHTML = this.responseText;
      break;
    default:
      document.getElementById('config_save_status').innerHTML = xhr.statusText + ':' + xhr.responseText;
    }
  });
  xhr.addEventListener('timeout', function(e) {
    document.getElementById('config_save_status').innerHTML = 'Server Timeout';
  });
  xhr.addEventListener('error', function(e) {
    document.getElementById('config_save_status').innerHTML = 'Server Error';
  });
  var endpoint = '/_settings_update';
  xhr.open('POST', endpoint);
  xhr.send(formdata.join('\n'));
  document.getElementById('config_save_status').innerHTML = 'Saving ...';
}

function _vise_settings_get_var_desc(variable_name) {
  var desc = [];
  switch(variable_name) {
  case 'address':
    desc = [
      'Server IP Address',
      'HTTP server of VISE is bound to this IP address. Set to "localhost" to run VISE locally on your computer.'
    ];
    break;
  case 'nthread':
    desc = [
      'Threads',
      'Number of parallel threads to handle HTTP requests'
    ];
    break;
  case 'port':
    desc = [
      'Port',
      'HTTP server listens on this port.'
    ];
    break;

  case 'vise_store':
    desc = [
      'VISE Application Store',
      'VISE application data (projects, configuration, etc.) are stored in this folder.'
    ];
    break;

  case 'www_store':
    desc = [
      'HTTP Asset Store',
      'HTTP assets (e.g. html, css, js) for VISE HTML based user interface are store in this folder.'
    ];
    break;

  case 'generic_visual_vocabulary':
    desc = [
      'Generic Visual Vocabulary',
      'Location of generic visual vocabulary.'
    ];
    break;

  case 'asset_store':
    desc = [
      'VISE Asset Store',
      'VISE assets (e.g. data files, etc.) are stored in this folder.'
    ];
    break;
  default:
    desc = [variable_name, ''];
  }
  return desc;
}

function _vise_show_admintool() {
  admintool_container.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = 'Administrative Tools';
  var warning = document.createElement('p');
  warning.setAttribute('class', 'warning');
  warning.innerHTML = 'Warning: be careful when using these administrative tools as they can potentially cause loss of data.'

  var project_del_container = document.createElement('div');
  project_del_container.setAttribute('id', 'project_del_container');

  var form = document.createElement('form');
  form.setAttribute('method', 'POST');
  form.setAttribute('action', '/_project_delete');
  for( var pname in _vise_data.PROJECT_LIST ) {
    var checkbox = document.createElement('input');
    checkbox.setAttribute('type', 'checkbox');
    checkbox.setAttribute('name', pname);
    checkbox.setAttribute('value', '1');
    var label = document.createElement('label');
    label.setAttribute('for', pname);
    label.innerHTML = pname;

    form.appendChild(checkbox);
    form.appendChild(label);
  }
  var submit = document.createElement('input');
  submit.setAttribute('type', 'submit');
  submit.setAttribute('value', 'Delete Selected Projects');
  form.appendChild(submit)
  project_del_container.appendChild(form);

  var note = document.createElement('p');
  note.innerHTML = 'Note: Windows locks the VISE project folders when the VISE application is running and therefore currently it is not possible to delete projects using this interface in Windows. For now, we advise you to manually delete the corresponding folder. For example, to delete a project named "ABC", open File Explorer in Windows and delete the following folder: <code>C:\\Users\\USERNAME\\vgg\\vise\\store\\ABC</code>';
  project_del_container.appendChild(note);

  admintool_container.appendChild(title);
  admintool_container.appendChild(warning);
  admintool_container.appendChild(project_del_container);
}
