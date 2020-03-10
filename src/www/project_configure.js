/**
 *
 * @desc code to build HTML user interface for /{PNAME}/configure endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 21 Feb. 2020
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

var tool_container = document.createElement('div');
tool_container.setAttribute('id', 'info_container');
var files_container = document.createElement('div');
files_container.setAttribute('id', 'files_container');
var settings_container = document.createElement('div');
settings_container.setAttribute('id', 'settings_container');
var settings_table = document.createElement('table');

content.appendChild(files_container);
content.appendChild(tool_container);
content.appendChild(settings_container);

var local_file_selector = document.createElement('input');
local_file_selector.setAttribute('type', 'file');
local_file_selector.setAttribute('multiple', '');
local_file_selector.setAttribute('accept', 'image/*');
local_file_selector.addEventListener('change', _vise_configure_upload_selected_files);
var user_selected_files;
var upload_success_filename_list = [];
var upload_error_filename_list = [];
var upload_progress = document.createElement('progress');
var file_add_status = document.createElement('div');
file_add_status.setAttribute('id', 'file_add_status');
file_add_status.classList.add('hide');

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
  pname_link.setAttribute('title', 'Home of ' + _vise_data.PNAME + ' project');
  pname_link.innerHTML = _vise_data.PNAME;

  pname.innerHTML = '';
  pname.appendChild(home_link);
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_show_configure_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_configure_ui() {
  var form = document.createElement('form');
  form.setAttribute('method', 'POST');
  form.setAttribute('action', '/' + _vise_data.PNAME + '/_index_create');
  var start = document.createElement('input');
  start.setAttribute('type', 'submit');
  start.setAttribute('value', 'Create Visual Search Engine');
  form.appendChild(start);
  tool_container.appendChild(form);

  _vise_init_configure_toolbar();
  _vise_init_configure_files();
  _vise_init_configure_settings();
}

function _vise_init_configure_toolbar() {
  pageinfo.innerHTML = 'Project contains ' + _vise_data.IMAGE_SRC_COUNT + ' files.';
}

function _vise_project_file_count_on_error() {
  pageinfo.innerHTML = 'Failed to get number of files in this project.';
}

function _vise_project_file_count_update() {
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
    switch(xhr.statusText) {
    case 'OK':
      pageinfo.innerHTML = 'Project contains ' + xhr.responseText + ' files.';
      break;
    default:
      _vise_project_file_count_on_error();
    }
  });
  xhr.addEventListener('timeout', _vise_project_file_count_on_error);
  xhr.addEventListener('error', _vise_project_file_count_on_error);

  var endpoint = '/' + _vise_data.PNAME + '/_image_src_count';
  xhr.open('GET', endpoint);
  xhr.send();
}

function _vise_init_configure_settings() {
  settings_container.innerHTML = '';
  _vise_configure_show_use_default(true);
  settings_container.appendChild(settings_table);
}

function _vise_init_configure_files() {
  files_container.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = "Add Images";
  files_container.appendChild(title);

  var local = document.createElement('div');
  var label1 = document.createElement('span');
  label1.innerHTML = 'Select local files&nbsp;';
  local.appendChild(label1);
  local.appendChild(local_file_selector); // local_file_selector already initialized as global var.
  upload_progress.setAttribute('class', 'hide');
  local.appendChild(upload_progress);

  var folder = document.createElement('div');
  var label2 = document.createElement('span');
  label2.innerHTML = 'or, directly import from this folder';
  var input2 = document.createElement('input');
  input2.setAttribute('type', 'text');
  input2.setAttribute('placeholder', 'e.g. C:\\Documents\\data\\images');
  input2.setAttribute('id', 'import_folder_input');
  var add = document.createElement('button');
  add.innerHTML = 'Add';
  add.addEventListener('click', _vise_configure_import_files_from_folder);
  folder.appendChild(label2);
  folder.appendChild(input2);
  folder.appendChild(add);

  file_add_status.innerHTML = '';
  files_container.appendChild(local);
  files_container.appendChild(folder);
  files_container.appendChild(file_add_status);
}

function _vise_configure_show_use_default(is_checked) {
  settings_table.innerHTML = '';
  var tr = document.createElement('tr');
  var col1 = document.createElement('td');
  col1.innerHTML = 'Use default settings';
  var col2 = document.createElement('td');
  var checkbox = document.createElement('input');
  checkbox.setAttribute('type', 'checkbox');
  if(is_checked) {
    checkbox.setAttribute('checked', '');
  }
  checkbox.addEventListener('change', function(e) {
    if(this.checked) {
      _vise_configure_show_use_default(true);
    } else {
      _vise_configure_show_all_settings();
    }
  });
  col2.appendChild(checkbox);

  tr.appendChild(col1);
  tr.appendChild(col2);
  settings_table.appendChild(tr);
}

function _vise_configure_show_all_settings() {
  _vise_configure_show_use_default(false);

  var no_edit_key_list = ['cover_image_filename', 'data_dir', 'search_engine'];

  var pidrow = document.createElement('tr');
  pidrow.innerHTML = '<td>Project Id</td><td>' + _vise_data.PNAME + '</td>';
  settings_table.appendChild(pidrow);
  for(var key in _vise_data.PCONF) {
    var desc = _vise_configure_get_var_desc(key);
    var tr = document.createElement('tr');
    var keycol = document.createElement('td');
    keycol.innerHTML = desc[0];
    keycol.setAttribute('title', desc[1]);
    var valcol = document.createElement('td');
    var input = document.createElement('input');
    input.setAttribute('type', 'text');
    input.setAttribute('name', key);
    input.setAttribute('value', _vise_data.PCONF[key]);
    input.setAttribute('title', _vise_data.PCONF[key]);

    if(no_edit_key_list.includes(key)) {
      input.setAttribute('disabled', '');
    }
    valcol.appendChild(input);

    tr.appendChild(keycol);
    tr.appendChild(valcol);
    settings_table.appendChild(tr);
  }
  var control = document.createElement('tr');
  var col = document.createElement('td');
  col.setAttribute('colspan', '2');
  var save = document.createElement('button');
  save.innerHTML = 'Save Settings';
  save.addEventListener('click', _vise_configure_on_save_settings);

  var config_save_status = document.createElement('span');
  config_save_status.setAttribute('id', 'config_save_status');
  col.appendChild(save);
  col.appendChild(config_save_status);
  control.appendChild(col);
  settings_table.appendChild(control);
}


function _vise_configure_on_save_settings() {
  var input = settings_table.getElementsByTagName("input");

  var formdata = [];
  for(var i=0; i<input.length; ++i) {
    if(input[i].getAttribute('type') === 'text') {
      var key = input[i].getAttribute('name');
      var val = input[i].value;
      formdata.push(key + '=' + val);
      _vise_data.PCONF[key] = val;
    }
  }

  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
    switch(xhr.statusText) {
    case 'OK':
      document.getElementById('config_save_status').innerHTML = 'Saved';
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
  var endpoint = '/' + _vise_data.PNAME + '/_config_save';
  xhr.open('POST', endpoint);
  xhr.send(formdata.join('\n'));
  document.getElementById('config_save_status').innerHTML = 'Saving ...';
}

function _vise_configure_get_var_desc(variable_name) {
  var desc = [];
  switch(variable_name) {
  case 'bow_cluster_count':
    desc = [
      'BoW Cluster Count',
      'number of words in visual vocabulary (i.e. number of clusters in descriptor space).'
    ];
    break;
  case 'bow_descriptor_count':
    desc = [
      'BoW Descriptor Count',
      'number of descriptors to use for clustering to generate visual vocabulary (set -1 to use all available descriptors)'
    ];
    break;
  case 'cluster_num_iteration':
    desc = [
      'BoW Cluster Iterations',
      'number of iterations of kmeans based clustering'
    ];
    break;

  case 'hamm_embedding_bits':
    desc = [
      'Hamming Embeddings Bits',
      'number of bits to use for hamming embedding. (use 32 bits for extremely large datasets, and 64 bits for smaller datasets)'
    ];
    break;

  case 'sift_scale_3':
    desc = [
      'Scale SIFT',
      'Scale SIFT features by 3 (a heuristic that results in good performance)'
    ];
    break;

  case 'use_root_sift':
    desc = [
      'Use RootSIFT',
      'Use RootSIFT descriptor instead of SIFT descriptor'
    ];
    break;

  case 'search_engine':
    desc = [
      'Search Engine',
      'Name of search engine used to index and search images'
    ];
    break;

  case 'cover_image_filename':
    desc = [
      'Cover Image',
      'name of image to use as cover image for this project'
    ];
    break;

  case 'data_dir':
    desc = [
      'Data Dir.',
      'folder location where VISE stores all application data associated with this project'
    ];
    break;

  case 'resize_dimension':
    desc = [
      'Resize Images to (in pixels)',
      'Resize original images to provided width x height dimension before indexing.'
    ];
    break;

  default:
    desc = [variable_name, ''];
  }
  return desc;
}

function _vise_configure_upload_selected_files(e) {
  user_selected_files = e.target.files;
  upload_success_filename_list = [];
  upload_error_filename_list = [];

  upload_progress.setAttribute('max', user_selected_files.length);
  upload_progress.setAttribute('value', '0');
  upload_progress.classList.remove('hide');
  file_add_status.classList.add('hide');

  var parallel_upload_count = 4;
  var start_index = 0;
  _vise_configure_continue_file_upload(start_index, parallel_upload_count);
}

function _vise_configure_continue_file_upload(start_index, parallel_upload_count) {
  var upload_promise_list = [];
  var end_index = Math.min(user_selected_files.length, start_index + parallel_upload_count);
  console.log('_vise_configure_continue_file_upload: ' + start_index + ' to ' + end_index);
  for(var i=start_index; i<end_index; ++i) {
    upload_promise_list.push(_vise_configure_upload_file(i, user_selected_files[i]));
  }
  Promise.all(upload_promise_list).then( function(ok_file_list) {
    upload_progress.setAttribute('value', end_index);
    if(start_index < user_selected_files.length) {
      _vise_configure_continue_file_upload(end_index, parallel_upload_count);
    } else {
      _vise_project_file_count_update();
      if(upload_error_filename_list.length) {
        file_add_status.classList.remove('hide');
        file_add_status.innerHTML = 'Failed to add ' + upload_error_filename_list.length + ' files<br/>';
        for(var i=0; i<upload_error_filename_list.length; ++i) {
          file_add_status.innerHTML += '[' + i + '] ' + upload_error_filename_list[i] + '<br/>';
        }
      }
    }
  }, function(err_file_list) {
    console.log('upload promise error');
    console.log(err_file_list);
  });

}

function _vise_configure_upload_file(file_index, file) {
  return new Promise(function(ok_callback, err_callback) {
    var xhr = new XMLHttpRequest();

    xhr.addEventListener('load', function() {
      upload_progress.setAttribute('value', file_index);

      switch(xhr.statusText) {
      case 'OK':
        upload_success_filename_list.push(file.name);

        ok_callback(xhr.statusText);
        break;
      default:
        upload_error_filename_list.push(file.name);
        err_callback(xhr.statusText);
      }
    });
    xhr.addEventListener('timeout', function(e) {
      upload_progress.setAttribute('value', file_index);
      upload_error_filename_list.push(file.name);
      err_callback('timeout');
    });
    xhr.addEventListener('error', function(e) {
      upload_progress.setAttribute('value', file_index);
      upload_error_filename_list.push(file.name);
      err_callback('error')
    });
    xhr.open('PUT', '/' + _vise_data.PNAME + '/' + file.name);
    xhr.send(file);
  });
}

function _vise_configure_import_files_from_folder() {
  upload_progress.setAttribute('class', 'hide');
  file_add_status.innerHTML = '';
  var import_folder = document.getElementById('import_folder_input').value;

  file_add_status.classList.remove('hide');
  file_add_status.innerHTML = 'Adding files from folder ' + import_folder + '... (please wait)<br/>';

  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
    switch(xhr.statusText) {
    case 'OK':
      var response = JSON.parse(xhr.responseText);
      file_add_status.innerHTML += 'Added ' + response.ADDED_FILENAME_LIST.length + ' files.<br/>';
      if(response.DISCARDED_FILENAME_LIST.length) {
        file_add_status.innerHTML += 'Failed to add the following ' + response.DISCARDED_FILENAME_LIST.length + ' files:<br/>';
        for(var i=0; i<response.DISCARDED_FILENAME_LIST.length; ++i) {
          file_add_status.innerHTML += '[' + i + '] ' + response.DISCARDED_FILENAME_LIST[i] + '<br/>';
        }
      }
      break;
    default:
      file_add_status.innerHTML += xhr.statusText + ' : ' + xhr.responseText + '<br/>';
    }
    _vise_project_file_count_update();
  });
  xhr.addEventListener('timeout', function(e) {
    file_add_status.innerHTML += 'Failed to add files from folder ' + import_folder + '... [SERVER TIMEOUT]<br/>';
  });
  xhr.addEventListener('error', function(e) {
    file_add_status.innerHTML += 'Failed to add files from folder ' + import_folder + '... [SERVER ERROR]<br/>';
  });
  var endpoint = '/' + _vise_data.PNAME + '/_file_add?';
  endpoint += 'source_type=local_folder&response_format=json&source_loc=' + import_folder;
  xhr.open('POST', endpoint);
  xhr.send();
}
