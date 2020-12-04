/**
 ** Absolutely minimal manual image annotation javascript library
 **
 ** Author: Abhishek Dutta <adutta _at_ robots.ox.ac.uk>
 ** Date: 7 Feb. 2020
 */

function _via0(container, img_src) {
  this.c = container;
  this.img_src = img_src;

  // state
  this.is_region_draw_ongoing = false;
  this.xy = [];
  this.scale = 1.0;

  this.SVG_NS = "http://www.w3.org/2000/svg";
  this.img = null;
  this.rshape = null;
  this.rshape_rect_region = null;
  this.user_input_layer = null;

  this.hook_list = { 'region_add':[] };
  this.hook_payload = { 'region_add':[] };
  this._init();
}

_via0.prototype._init = function() {
  this.img = new Image();
  this.img.addEventListener('load', this._on_img_load.bind(this));
  this.img.src = this.img_src;
}

_via0.prototype._on_img_load = function(e) {
  /*
  var container_css = [];
  container_css.push('position:relative');
  container_css.push('border:none');
  this.c.setAttribute('style', container_css.join(';'));
  */
  if(this.img.naturalHeight > this.c.clientHeight) {
    this.scale = this.img.naturalHeight / this.c.clientHeight;
    this.sheight = this.c.clientHeight;
    this.swidth = Math.floor(this.img.naturalWidth / this.scale);
    if(this.swidth > this.c.clientWidth) {
      this.swidth = this.c.clientWidth;
      this.scale = this.img.naturalWidth / this.c.clientWidth;
      this.sheight = Math.floor(this.img.naturalHeight / this.scale);
    }
  } else {
    //this.sheight = this.img.naturalHeight;
    //this.swidth = this.img.naturalWidth;
    this.sheight = this.c.clientHeight;
    this.scale = this.img.naturalHeight / this.c.clientHeight;
    this.swidth = Math.floor(this.img.naturalWidth / this.scale);
    if(this.swidth > this.c.clientWidth) {
      this.swidth = this.c.clientWidth;
      this.scale = this.img.naturalWidth / this.c.clientWidth;
      this.sheight = Math.floor(this.img.naturalHeight / this.scale);
    }
  }
  this.left = Math.floor(this.c.clientWidth/2 - this.swidth/2); // center align

  var img_css = [];
  img_css.push('position:absolute');
  img_css.push('-webkit-user-select:none;-moz-user-select:none;user-select:none');
  img_css.push('top:0');
  img_css.push('left:' + this.left + 'px');
  img_css.push('height:' + this.sheight + 'px');
  img_css.push('width:' + this.swidth + 'px');
  this.img.setAttribute('style', img_css.join(';'));
  this.c.appendChild(this.img);

  this.rshape = document.createElementNS(this.SVG_NS, 'svg');
  var rshape_css = img_css.slice();
  rshape_css.push('fill:none');
  rshape_css.push('stroke:blue');
  rshape_css.push('stroke-width:4');
  rshape_css.push('z-index:1');
  this.rshape.setAttribute('style', rshape_css.join(';'));
  this.rshape_rect = document.createElementNS(this.SVG_NS, 'rect');
  this.rshape_rect.setAttributeNS(null, 'id', 'rshape_rect');
  this.rshape.appendChild(this.rshape_rect);
  this.c.appendChild(this.rshape);

  this.user_input_layer = document.createElement('div');
  var user_input_layer_css = img_css.slice();
  user_input_layer_css.push('z-index:2');
  user_input_layer_css.push('cursor:crosshair');
  this.user_input_layer.setAttribute('style', user_input_layer_css.join(';'));
  //this.user_input_layer.setAttribute('title', 'Click and drag mouse to define a search query region');
  this.user_input_layer.addEventListener('mousedown', this._on_mousedown.bind(this));
  this.user_input_layer.addEventListener('mouseup', this._on_mouseup.bind(this));
  this.user_input_layer.addEventListener('mousemove', this._on_mousemove.bind(this));
  this.c.appendChild(this.user_input_layer);
}

_via0.prototype._on_mousedown = function(e) {
  e.preventDefault();
  this.xy = [e.offsetX, e.offsetY];
  this.rshape_rect.setAttributeNS(null, 'x', this.xy[0]);
  this.rshape_rect.setAttributeNS(null, 'y', this.xy[1]);
  this.rshape_rect.setAttributeNS(null, 'width', 0);
  this.rshape_rect.setAttributeNS(null, 'height', 0);
  this.is_region_draw_ongoing = true;
}

_via0.prototype._on_mousemove = function(e) {
  e.preventDefault();
  if(this.is_region_draw_ongoing) {
    var rect = [this.xy[0], this.xy[1], e.offsetX, e.offsetY];
    // ensure that (x,y) is top-left coordinate
    if( rect[0] > rect[2] ) {
      if( rect[1] > rect[3] ) {
        this.rshape_rect.setAttributeNS(null, 'y', rect[3]);
      }
      this.rshape_rect.setAttributeNS(null, 'x', rect[2]);
    } else {
      if( rect[1] > rect[3] ) {
        this.rshape_rect.setAttributeNS(null, 'y', rect[3]);
      }
    }
    this.rshape_rect.setAttributeNS(null, 'width', Math.abs(rect[2]-rect[0]));
    this.rshape_rect.setAttributeNS(null, 'height', Math.abs(rect[3]-rect[1]));
  }
}

_via0.prototype._on_mouseup = function(e) {
  e.preventDefault();
  this.is_region_draw_ongoing = false;
  this.xy = [];

  var x = parseInt(this.rshape_rect.getAttribute('x'));
  var y = parseInt(this.rshape_rect.getAttribute('y'));
  var w = parseInt(this.rshape_rect.getAttribute('width'));
  var h = parseInt(this.rshape_rect.getAttribute('height'));

  var region = { 'x':x*this.scale,
                 'y':y*this.scale,
                 'width':w*this.scale,
                 'height':h*this.scale
               };
  if(this.hook_list['region_add'].length) {
    for(var i=0; i < this.hook_list['region_add'].length; ++i) {
      var payload = this.hook_payload['region_add'][i];
      if(typeof(payload) !== 'undefined') {
        for(var key in payload) {
          region[key] = payload[key];
        }
      }
      this.hook_list['region_add'][i].call(this, region);
    }
  }
}

_via0.prototype.add_hook = function(event_name, method_ref, payload) {
  if(event_name === 'region_add') {
    this.hook_list['region_add'].push(method_ref);
    this.hook_payload['region_add'].push(payload);
  }
}
