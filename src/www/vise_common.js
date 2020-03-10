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
