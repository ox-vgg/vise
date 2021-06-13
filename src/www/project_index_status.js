/**
 *
 * @desc code to build HTML user interface for /{PNAME}/index_status endpoint
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 4 Mar. 2020
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
var progress_container = document.createElement('div');
progress_container.setAttribute('id', 'progress_container');
var message_container = document.createElement('div');
message_container.setAttribute('id', 'message_container');
content.appendChild(progress_container);
content.appendChild(message_container);

var STATUS_UPDATE_INTERVAL = 1000; // ms
var _vise_is_first_run = true;

document.body.appendChild(toolbar);
document.body.appendChild(content);

// check existence of everything we need
if( !_vise_self_check_is_ok()) {
  console.log('self check failed');
} else {
  var pname_link = document.createElement('a');
  pname_link.setAttribute('href', 'index_status');
  pname_link.setAttribute('title', 'Home of ' + _vise_data.PNAME + ' project');
  pname_link.innerHTML = _vise_data.PNAME;

  var vise_logo = document.createElementNS(_VISE_SVG_NS, 'svg');
  vise_logo.setAttributeNS(null, 'viewBox', '0 0 240 80');
  vise_logo.innerHTML = '<use xlink:href="#vise_logo"></use><title>VGG Image Search Engine (VISE)</title>';
  vise_logo.setAttributeNS(null, 'style', 'height:0.8em; padding-right:1em;');

  var home_link = document.createElement('a');
  home_link.setAttribute('href', '../home');
  home_link.appendChild(vise_logo);

  pname.innerHTML = '';
  pname.appendChild(home_link);
  pname.appendChild(pname_link);

  document.title = _vise_data.PNAME;
  _vise_show_index_status_ui();
}

function _vise_self_check_is_ok() {
  if( typeof(_vise_data) === 'object' ) {
    return true;
  }
  return false;
}

function _vise_show_index_status_ui() {
  var title = document.createElement('h3');
  title.innerHTML = 'Creating Visual Search Engine ...';
  progress_container.appendChild(title);

  _vise_is_first_run = true;
  //_vise_fetch_index_progress();
  setTimeout(_vise_fetch_index_progress, STATUS_UPDATE_INTERVAL);
}

function _vise_init_index_progress() {
  var progress_list = document.createElement('table');
  for ( var task_id in _vise_data.INDEX_STATUS.task_progress) {
    var task = _vise_data.INDEX_STATUS.task_progress[task_id];
    var task_desc = _vise_index_status_task_description(task.name);
    var tr = document.createElement('tr');
    var col1 = document.createElement('td');
    col1.innerHTML = task_desc[0];
    var col2 = document.createElement('td');
    var progress = document.createElement('progress');
    progress.setAttribute('id', 'progress_' + task.name);
    progress.setAttribute('value', task.value);
    progress.setAttribute('max', task.max);
    col2.appendChild(progress);

    var col3 = document.createElement('td');
    col3.setAttribute('class', 'progress_label');
    col3.setAttribute('id', 'progress_label_' + task.name);

    tr.appendChild(col1);
    tr.appendChild(col2);
    tr.appendChild(col3);
    progress_list.appendChild(tr);
  }
  progress_container.appendChild(progress_list);
}

function _vise_index_status_task_description(task_name) {
  switch(task_name) {
  case 'preprocess':
    return ['Pre-processing Images', ''];
    break;
  case 'traindesc':
    return ['Extracting Visual Descriptors', ''];
    break;
  case 'cluster':
    return ['Building Visual Vocabulary', ''];
    break;
      case 'assign':
    return ['Assigning Visual Descriptors', ''];
    break;
      case 'hamm':
    return ['Generating Hamming Embeddings', ''];
    break;
  case 'index':
    return ['Indexing Images', ''];
    break;
  }
}

function _vise_update_index_progress() {
  for ( var task_id in _vise_data.INDEX_STATUS.task_progress) {
    var task = _vise_data.INDEX_STATUS.task_progress[task_id];
    var progress = document.getElementById('progress_' + task.name);
	  if(progress === null) {
		  continue;
	  }
    progress.setAttribute('value', task.value);
    progress.setAttribute('max', task.max);
    progress.setAttribute('title', task.value + ' of ' + task.max);
    progress.innerHTML = task.value + ' of ' + task.max;

    var progress_label = document.getElementById('progress_label_' + task.name);
    if(task.is_complete !== 1 && task.has_started === 1) {
      progress_label.innerHTML = task.value + ' of ' + task.max;
	    if(task.max !== '18000000') { // indicates max. no. of training descriptors
	      if(task['elapsed_ms'] > 0 && task['value'] !== 0) {
		      var elapsed_ms = parseFloat(task['elapsed_ms']);
		      var avg_time_per_step = elapsed_ms / parseInt(task.value);
		      var remaining_step = parseInt(task.max) - parseInt(task.value) - 1;
		      var remaining_time_ms = remaining_step * avg_time_per_step;
          try {
		        var hhmmss = new Date(remaining_time_ms).toISOString().substr(11, 8);
		        progress_label.innerHTML += ' (remaining time ' + hhmmss + ')';
          } catch(err) {
            console.error(err)
            console.log(remaining_time_ms);
            console.log(elapsed_ms)
            console.log(remaining_step)
            console.log(task);
          }
	      }
	    }
    } else {
	    if(task.is_complete === 1) {
        var elapsed_sec = Number.parseFloat(task.elapsed_ms/1000.0).toFixed(1);
        progress_label.innerHTML = 'completed in ' + elapsed_sec + ' sec.';
	    }
    }
  }
}

function _vise_fetch_index_progress() {
  var xhr = new XMLHttpRequest();
  xhr.addEventListener('load', function() {
	  switch(this.status) {
	  case 200:
	    _vise_data = JSON.parse(this.responseText);
	    if(_vise_data.INDEX_STATUS.index_is_done === 1 &&
         _vise_data.INDEX_STATUS.index_is_ongoing === 0 ) {
		    window.location.replace('filelist');
	    } else {
		    if(_vise_is_first_run) {
		      _vise_is_first_run = false;
		      _vise_init_index_progress();
		    }
		    try {
		      _vise_update_index_progress();
          message_container.innerHTML = '';
		    } catch(error) {
          message_container.innerHTML = 'Error updating status: ' + error;
        }
        setTimeout(_vise_fetch_index_progress, STATUS_UPDATE_INTERVAL);
      }
      break;
    default:
      message_container.innerHTML = 'Error fetching status: ' + this.statusText;
    }
  });
  xhr.addEventListener('timeout', function(e) {
    message_container.innerHTML = 'Error fetching status: server timeout';
  });
  xhr.addEventListener('error', function(e) {
    message_container.innerHTML = 'Error fetching status: server error';
  });

  xhr.open('GET', 'index_status?response_format=json');
  xhr.send();
}
