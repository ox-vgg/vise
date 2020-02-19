function _vise_show_existing_projects(div_element) {
  div_element.innerHTML = '';
  _vise_xhr('GET', '/_project_list').then( function(response) {
    var project_list = JSON.parse(response);
    for(var pname in project_list) {
      var a = document.createElement('a');
      a.setAttribute('href', '/' + pname + '/index.html');
      var c = document.createElement('div');
      c.setAttribute('class', 'project');
      var img = document.createElement('img');
      img.setAttribute('src', project_list[pname]['cover_image_filename']);
      var imgcontainer = document.createElement('div');
      imgcontainer.setAttribute('class', 'imgcontainer');
      imgcontainer.appendChild(img);
      c.appendChild(imgcontainer);
      var desc = document.createElement('p');
      desc.innerHTML = pname;
      c.appendChild(desc);
      a.appendChild(c);
      div_element.appendChild(a);
    }
  }, function(err_response) {
    div_element.innerHTML = 'Error: ' + err_response;
  });
}

function _vise_xhr(method, uri, payload) {
  return new Promise( function(ok_callback, err_callback) {
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
      err_callback('server response wait timeout');
    });
    xhr.addEventListener('error', function(e) {
      err_callback('server failed to respond')
    });
    xhr.open(method, _vise_conf['SERVER'] + uri);
    if(typeof(payload) === 'undefined') {
      xhr.send();
    } else {
      xhr.send(payload);
    }
  });
}
