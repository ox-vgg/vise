/**
 *
 * @desc common JS code used by all project_*.js modules
 * @author Abhishek Dutta <adutta@robots.ox.ac.uk>
 * @date 20 Feb. 2020
 *
 */

'use strict'

var _VISE_SVG_NS = "http://www.w3.org/2000/svg";

function _vise_common_get_svg_button(icon_id, title, id) {
  var el = document.createElementNS(_VISE_SVG_NS, 'svg');
  el.setAttributeNS(null, 'viewBox', '0 0 24 24');
  el.innerHTML = '<use xlink:href="#' + icon_id + '"></use><title>' + title + '</title>';
  el.setAttributeNS(null, 'class', 'svg_button');
  if ( typeof(id) !== 'undefined' ) {
    el.setAttributeNS(null, 'id', id);
  }
  return el;
}

function _vise_sanitize_rect(x0, y0, x1, y1) {
  if(x1 > x0) {
    if(y1 > y0) {
      return [x0, y0, x1, y1];
    } else {
      return [x0, y1, x1, y0];
    }
  } else {
    if(y1 > y0) {
      return [x1, y0, x0, y1];
    } else {
      return [x1, y1, x0, y0];
    }
  }
}

function _vise_set_project_pagetools(container, pagename) {
  var filelist_icon = _vise_common_get_svg_button('micon_grid', 'View list of all files in this project');
  var filelist_link = document.createElement('a');
  filelist_link.setAttribute('href', 'filelist');
  filelist_link.appendChild(filelist_icon);

  var external_search_icon = _vise_common_get_svg_button('micon_image_search', 'Search using your image');
  var external_search_link = document.createElement('a');
  external_search_link.setAttribute('href', 'external_search');
  external_search_link.appendChild(external_search_icon);

  container.innerHTML = '';
  container.appendChild(filelist_link);
  container.appendChild(external_search_link);
}

function _vise_save_data_to_local_file(data, filename) {
  var a      = document.createElement('a');
  a.href     = URL.createObjectURL(data);
  a.download = filename;

  // simulate a mouse click event
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true
  });
  a.dispatchEvent(event);
}
