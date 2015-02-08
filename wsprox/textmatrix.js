var textmatrix_displaydata = []; // array arranged by y x

var textmatrix_cursor_x = 0;
var textmatrix_cursor_y = 0;
var textmatrix_textwidth = 10;
var textmatrix_textheight = 10;

var textmatrix_rows = 25;
var textmatrix_cols = 80;

function textmatrix_setsize_px(x,y) {
  textmatrix_set_textsize();
  
  textmatrix_cols = Math.floor(x/textmatrix_textwidth);
  textmatrix_rows = Math.floor(y/textmatrix_textheight);
  console.debug("x in: " + x);
  console.debug("y in: " + y);
  console.debug("rows: " + textmatrix_rows);
  console.debug("cols: " + textmatrix_cols);
}

function textmatrix_set_textsize() {
  var d = document.createElement("span");
  d.style.position = "absolute";
  d.style.top      = "-100px";
  d.style.display  = "inline-block";
  d.innerHTML = "A";

  textmatrix_textwidth  = d.offsetWidth;
  textmatrix_textheight = d.offsetHeight;

  if(textmatrix_textwidth  == 0) { textmatrix_textwidth = 8;   }
  if(textmatrix_textheight == 0) { textmatrix_textheight = 16; }
}

function textmatrix_add_px(x,y) {
  textmatrix_setsize_px(x,y);
  textmatrix_add_tx(textmatrix_cols,textmatrix_rows);
}

function textmatrix_add_tx(x,y) {

  for(var cy=0;cy<y;cy++) {

    // create this via DOM
    cline = "<div style=\"margin-top: 0px; margin-bottom: 0px; padding:0; display: inline-block\" id=\"l" + cy + "\">"

    textmatrix_displaydata.push([]);
    for(var cx=0;cx<x;cx++) {
      textmatrix_displaydata[cy].push({"char":"A", "fg":65535, "bg":0});
    }
    cline += "</div><br>";
    document.getElementById('tml').innerHTML += cline;
    textmatrix_redraw_line(cy);
  }
}

function textmatrix_get_cols() {
  return textmatrix_cols;
}

function textmatrix_get_rows() {
  return textmatrix_rows;
}

function textmatrix_setpos(x,y,c,bg,fg) {

  textmatrix_displaydata[y][x].char = c;
  textmatrix_displaydata[y][x].bg   = bg;
  textmatrix_displaydata[y][x].fg   = fg;

}

function textmatrix_redraw_line(y) {

  var newline;

  newline = textmatrix_getline(y);
  
  document.getElementById('l' + y).innerHTML = newline;
}

function textmatrix_setline(y,newline) {
  for(var cx=0;cx<newline.length;cx++) {
    textmatrix_displaydata[y][cx].char = newline.substr(cx,1);
  }
  textmatrix_redraw_line(y);

}

function textmatrix_getline(y) {
  var data ="";

  var fgcol = textmatrix_displaydata[y][0].fg.toString(16);
  var bgcol = textmatrix_displaydata[y][0].bg.toString(16);
  var fgcolpad = fgcol;
  for(var n=fgcol.length;n<6;n++) fgcolpad = "0" + fgcolpad;

  var bgcolpad = bgcol;
  for(var n=bgcol.length;n<6;n++) bgcolpad = "0" + bgcolpad;
  data = data + "<p style=\"margin:0; padding:0; color: #" + fgcolpad + "; background-color: #" + bgcolpad + "; display: inline-block\">";
  data = data + textmatrix_displaydata[y][0].char;

  for(var cx=1;cx<textmatrix_displaydata[y].length;cx++) {

    if((cx == textmatrix_cursor_x) && (y == textmatrix_cursor_y)) {
      data = data + "<u>";
    }

    if((textmatrix_displaydata[y][cx-1].bg != textmatrix_displaydata[y][cx].bg) ||
       (textmatrix_displaydata[y][cx-1].fg != textmatrix_displaydata[y][cx].fg)) {
      data = data + "</p>";
      var fgcol = textmatrix_displaydata[y][cx].fg.toString(16);
      var bgcol = textmatrix_displaydata[y][cx].bg.toString(16);
 
      var fgcolpad = fgcol;
      for(var n=fgcol.length;n<6;n++) fgcolpad = "0" + fgcolpad;

      var bgcolpad = bgcol;
      for(var n=bgcol.length;n<6;n++) bgcolpad = "0" + bgcolpad;

      data = data + "<p style=\"color: #" + fgcolpad + "; background-color: #" + bgcolpad + "; display: inline\">";
      data = data + textmatrix_displaydata[y][cx].char;
    } else {
      data = data + textmatrix_displaydata[y][cx].char;
    }

    if((cx == textmatrix_cursor_x) && (y == textmatrix_cursor_y)) {
      data = data + "</u>";
    }

  }


  data = data + "</p>";

  return data;
}

function textmatrix_getline_txt(y) {
  var data = "";
  for(var cx=0;cx<textmatrix_displaydata[y].length;cx++) {
      data = data + textmatrix_displaydata[y][cx].char;
  }
  return data;
}

function textmatrix_draw_cursor(x,y) {
  if((x != textmatrix_cursor_x) || (y != textmatrix_cursor_y)) {
    redraw_y = textmatrix_cursor_y;
  }

  textmatrix_cursor_x = x;
  textmatrix_cursor_y = y;

  textmatrix_redraw_line(redraw_y);
}
