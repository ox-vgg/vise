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
var settings_mode_selector = document.createElement('div');

var settings_table = document.createElement('table');
var settings_details = document.createElement('div');
settings_details.setAttribute('class', 'settings_details');

content.appendChild(files_container);
content.appendChild(settings_container);
content.appendChild(tool_container);

var local_file_selector = document.createElement('input');
local_file_selector.setAttribute('type', 'file');
local_file_selector.setAttribute('multiple', '');
local_file_selector.setAttribute('accept', 'image/*');
local_file_selector.addEventListener('change', _vise_configure_upload_selected_files);
var user_selected_files;
var upload_success_filename_list = [];
var upload_error_filename_list = [];
var upload_progress = document.createElement('progress');
var upload_progress_message = document.createElement('span');

var file_add_status = document.createElement('div');
file_add_status.setAttribute('id', 'file_add_status');
file_add_status.classList.add('hide');

document.body.appendChild(toolbar);
document.body.appendChild(content);

var create_search_engine_button; // button is disabled until files are added

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 240 80');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'style', 'height:0.8em; padding-right:1em;');

  var home_link = document.createElement('a');
  home_link.setAttribute('href', '../home');
  home_link.appendChild(vise_logo);

  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'configure');
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
  form.setAttribute('action', '_index_create');
  create_search_engine_button = document.createElement('input');
  create_search_engine_button.setAttribute('type', 'submit');
  create_search_engine_button.setAttribute('value', 'Create Visual Search Engine');
  if(_vise_data.IMAGE_SRC_COUNT === 0) {
    create_search_engine_button.setAttribute('disabled', '');
  }
  form.appendChild(create_search_engine_button);
  tool_container.appendChild(form);

  _vise_init_configure_toolbar();
  _vise_init_configure_files();
  _vise_init_configure_settings();

  if(_vise_data.IMAGE_SRC_COUNT > 0) {
		create_search_engine_button.removeAttribute('disabled');
    document.getElementById('preset_conf_2').removeAttribute('disabled');
	}
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
	    var file_count = parseInt(xhr.responseText);
      pageinfo.innerHTML = 'Project contains ' + file_count + ' files.';
	    if(file_count > 0) {
		    create_search_engine_button.removeAttribute('disabled');
        document.getElementById('preset_conf_2').removeAttribute('disabled');
	    }
      break;
    default:
      _vise_project_file_count_on_error();
    }
  });
  xhr.addEventListener('timeout', _vise_project_file_count_on_error);
  xhr.addEventListener('error', _vise_project_file_count_on_error);

  var endpoint = '_image_src_count';
  xhr.open('GET', endpoint);
  xhr.send();
}

function _vise_init_configure_settings() {
  settings_container.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = "Project Settings";
  settings_container.appendChild(title);

  settings_container.appendChild(settings_mode_selector);
  settings_container.appendChild(settings_table);

  _vise_configure_init_settings_mode_selector();
}

function _vise_init_configure_files() {
  files_container.innerHTML = '';
  var title = document.createElement('h3');
  title.innerHTML = "Add Images";
  files_container.appendChild(title);

  var local = document.createElement('div');
  var label1 = document.createElement('span');
  label1.innerHTML = 'Select local files (press <span class="key">Ctrl</span> + <span class="key">A</span> to select all files in a folder)&nbsp;';
  local.appendChild(label1);
  local.appendChild(local_file_selector); // local_file_selector already initialized as global var.
  upload_progress.setAttribute('class', 'hide');
  upload_progress_message.setAttribute('class', 'hide');
  local.appendChild(upload_progress);
  local.appendChild(upload_progress_message);

  var folder = document.createElement('div');
  var label2 = document.createElement('span');
  label2.innerHTML = 'or, directly import from this folder';
  var input2 = document.createElement('input');
  input2.setAttribute('type', 'text');
  input2.setAttribute('maxlength', '512');
  input2.setAttribute('placeholder', 'e.g. C:\\Documents\\data\\images');
  input2.setAttribute('title', 'The process of adding a very large number of files (e.g. 50,000 files) may not succeed as it will take a very long time. For such cases, you can split your files into subfolders (e.g. 10 subfolders each containing 5,000 files) and add each subfolder in turn.');
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

function _vise_configure_init_settings_mode_selector() {
  settings_mode_selector.innerHTML = '';
  for(var i in _vise_data.PCONF_PRESET_LIST) {
    var preset_id = _vise_data.PCONF_PRESET_LIST[i];
    var row = document.createElement('p');
    var preset_radio = document.createElement('input');
    preset_radio.setAttribute('type', 'radio');
    preset_radio.setAttribute('name', 'settings');
    preset_radio.setAttribute('value', preset_id);
    preset_radio.setAttribute('id', preset_id);
    if(preset_id === 'preset_conf_2') {
      preset_radio.setAttribute('disabled', '');
    }
    if(preset_id === _vise_data.PRESET_CONF_ID) {
      preset_radio.setAttribute('checked', '');
      _vise_configure_init_settings_details(preset_id);
    }
    preset_radio.addEventListener('change', function(e) {
      settings_table.innerHTML = '';
      var preset_conf_id = this.getAttribute('value');
      _vise_configure_use_preset(preset_conf_id);
    });
    var preset_label = document.createElement('label');
    preset_label.setAttribute('for', preset_id);
    preset_label.innerHTML = _vise_configure_preset_id_to_title(preset_id);
    row.appendChild(preset_radio);
    row.appendChild(preset_label);
    settings_mode_selector.appendChild(row);
  }
  settings_mode_selector.appendChild(settings_details);
}

function _vise_configure_preset_id_to_title(preset_id) {
  switch(preset_id) {
  case 'preset_conf_1':
    return 'Preset 1: Fast indexing but visual search may be less accurate';
    break;
  case 'preset_conf_2':
    return 'Preset 2: Indexing is slow but search is accurate if project contains large  (e.g. 500) number of files (only select after adding all files)';
    break;
  case 'preset_conf_auto':
    return 'Auto: Automatic selection of configuration parameters';
    break;
  case 'preset_conf_manual':
    return 'Manually set all options (only for advanced users)';
    break;
  default:
    return 'Unknown';
  }
}

function _vise_configure_init_settings_details(preset_id) {
  switch(preset_id) {
  case 'preset_conf_1':
    settings_details.innerHTML = '<details><summary>More details about this setting</summary><ul><li>A precomputed generic visual vocaulary is used and therefore visual search engine gets created quickly.</li><li>The project uses less disk space.</li><li>Visual search may be less accurate because the project\'s images may contain visual patterns that are not represented in the generic visual vocabulary</li></ul></details>';
    break;
  case 'preset_conf_2':
    settings_details.innerHTML = '<details><summary>More details about this setting</summary><p><ul><li>Visual vocabulary is computed from the images added to the project and therefore the visual search engine creation process takes longer to complete.</li><li>The project uses more disk space.</li><li>You must add all the images before selecting this option because the most of the configuration parameters are estimated using the number of images.</ul></p></details>';
    break;
  case 'preset_conf_auto':
    settings_details.innerHTML = '<details><summary>More details about this setting</summary><p><ul><li>Visual vocabulary is computed from the images added to the project and therefore the visual search engine creation process takes longer to complete.</li><li>Configuration parameters (e.g. number of clusters in visual vocabulary) are automatically inferred from the number of images contained in the project.</li><li>Visual search is more accurate because the generate visual vocabulary is capable of representing most of the visual patterns contained in project\'s images.</li><li>Automatic parameter selection may fail for some projects (e.g. projects with very small number of images)</ul></p></details>';
    break;
  case 'preset_conf_manual':
    settings_details.innerHTML = '';
    break;
  default:
    return 'Unknown';
  }
}

function _vise_configure_init_manual_conf_editor(confdata_str) {
  var confdata = JSON.parse(confdata_str)
  settings_table.innerHTML = '';
  var no_edit_key_list = ['cover_image_filename', 'data_dir', 'search_engine', 'preset_conf_id', 'project_name'];

  var pidrow = document.createElement('tr');
  pidrow.innerHTML = '<td>Project Id</td><td>' + _vise_data.PNAME + '</td>';
  settings_table.appendChild(pidrow);
  for(var key in confdata) {
    var desc = _vise_configure_get_var_desc(key);
    var tr = document.createElement('tr');
    var keycol = document.createElement('td');
    keycol.innerHTML = desc[0];
    keycol.setAttribute('title', desc[1]);
    var valcol = document.createElement('td');
    var input = document.createElement('input');
    input.setAttribute('type', 'text');
    input.setAttribute('name', key);
    input.setAttribute('value', confdata[key]);
    input.setAttribute('title', confdata[key]);

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

function _vise_configure_use_preset(preset_conf_id) {
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
    switch(xhr.statusText) {
    case 'OK':
      if(preset_conf_id === 'preset_conf_manual') {
        _vise_configure_get_current_conf().then(function(confdata_str) {
          _vise_configure_init_manual_conf_editor(confdata_str);
        }, function(err) {
          settings_details.innerHTML = 'Error: failed to get configuration';
        });
      }
      _vise_configure_init_settings_details(preset_conf_id);
      break;
    default:
      settings_details.innerHTML = '<span style="color:red;">Error: ' + xhr.responseText + '</span>';
    }
  });
  xhr.addEventListener('timeout', function(e) {
    settings_details.innerHTML = 'Error: Server Timeout';
  });
  xhr.addEventListener('error', function(e) {
    settings_details.innerHTML = 'Error: Server Error';
  });
  var endpoint = '_config_use_preset';
  xhr.open('POST', endpoint);
  xhr.send(preset_conf_id);
}

function _vise_configure_get_current_conf() {
  return new Promise(function(ok_callback, err_callback) {
    var xhr = new XMLHttpRequest();
    xhr.addEventListener('load', function() {
      switch(xhr.statusText) {
      case 'OK':
        ok_callback(xhr.responseText);
        break;
      default:
        err_callback(xhr.responseText);
      }
    });
    xhr.addEventListener('timeout', function(e) {
      err_callback('Error: Server Timeout');
    });
    xhr.addEventListener('error', function(e) {
      err_callback('Error: Server Error');
    });
    var endpoint = '_conf';
    xhr.open('GET', endpoint);
    xhr.send();
  });
}

function _vise_configure_on_save_settings() {
  var input = settings_table.getElementsByTagName("input");

  var formdata = [];
  for(var i=0; i<input.length; ++i) {
    if(input[i].getAttribute('type') === 'text') {
      var key = input[i].getAttribute('name');
      var val = input[i].value;
      formdata.push(key + '=' + val);
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
  var endpoint = '_config_save';
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
      'number of words in visual vocabulary (i.e. number of clusters in descriptor space). For very small datasets (e.g. 100 or 300 images), then set this to 1000. For medium sized datasets (e.g. 10,000 images), set this value to ~10,000. For very large datasets (e.g. 1,000,000 images), set this to 100,000.'
    ];
    break;
  case 'bow_descriptor_count':
    desc = [
      'BoW Descriptor Count',
      'number of descriptors to use for clustering to generate visual vocabulary (set -1 to use all available descriptors). If you have very large number of images (e.g. 1,000,000), then set this value to 1000000.'
    ];
    break;
  case 'cluster_num_iteration':
    desc = [
      'BoW Cluster Iterations',
      'number of iterations of kmeans based clustering. For small datasets, set this to 5. For very large datasets, set this to 30 (will take longer to complete).'
    ];
    break;

  case 'hamm_embedding_bits':
    desc = [
      'Hamming Embeddings Bits',
      'number of bits to use for hamming embedding. If memory and storage requirements are a concern for you, set this to 32 (may result in less accurate search results) otherwise set this to 64 for high accuracy (may require large storage and RAM).'
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
      'Resize original images such that their width and height does not exceed the specified dimension. To prevent image resize, set the value to -1'
    ];
    break;

  case 'preset_conf_id':
    desc = [
      'Preset Configuration Id',
      'Identifier for preset configuration'
    ];
    break;

  case 'project_name':
    desc = [
      'Name of project',
      'Project name (can only contain characters a-z, A-Z, dash (-) and underscore (_)'
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

  upload_progress_message.innerHTML = ''
  upload_progress_message.classList.add('hide');

  var parallel_upload_count = 4;
  var start_index = 0;
  _vise_configure_continue_file_upload(start_index, parallel_upload_count);
}

function _vise_configure_continue_file_upload(start_index, parallel_upload_count) {
  var upload_promise_list = [];
  var end_index = Math.min(user_selected_files.length, start_index + parallel_upload_count);
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
	    upload_progress.classList.add('hide');
	    upload_progress_message.innerHTML = 'Finished uploading ' + upload_success_filename_list.length + ' files.';
	    upload_progress_message.classList.remove('hide');
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
    xhr.open('PUT', file.name);
    xhr.send(file);
  });
}

function _vise_configure_import_files_from_folder() {
  upload_progress.setAttribute('class', 'hide');
  file_add_status.innerHTML = '';
  var import_folder = document.getElementById('import_folder_input').value;

  file_add_status.classList.remove('hide');
  file_add_status.innerHTML = 'Adding files from folder ' + import_folder + '... <br/>Please wait ... (adding 1,000 files usually takes around 20 seconds)<br/>';

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
  var endpoint = '_file_add?';
  endpoint += 'source_type=local_folder&response_format=json&source_loc=' + import_folder;
  xhr.open('POST', endpoint);
  xhr.send();
}
