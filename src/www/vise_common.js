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
