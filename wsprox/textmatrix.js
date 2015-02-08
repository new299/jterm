var textmatrix_displaydata = []; // array arranged by y x

function add_textmatrix(x,y) {

  for(var cy=0;cy<y;cy++) {

    cline = "<div style=\"margin-top: 0px; margin-bottom: 0px; padding:0; display: inline-block\" id=\"l" + cy + "\">"

    textmatrix_displaydata.push([]);
    for(var cx=0;cx<x;cx++) {
//      cline += "A";
      textmatrix_displaydata[cy].push({"char":"A", "fg":65535, "bg":0});
    }
    cline += "</div><br>";
    document.getElementById('tml').innerHTML += cline;
  textmatrix_redraw_line(cy);
  }
}

//function stridx(str,index, character) {
//    return str.substr(0, index) + character + str.substr(index+character.length);
//}

function textmatrix_setpos(x,y,c,bg,fg) {

  textmatrix_displaydata[y][x].char = c;
  textmatrix_displaydata[y][x].bg   = bg;
  textmatrix_displaydata[y][x].fg   = fg;

}

function textmatrix_redraw_line(y) {

  var newline;

//  for(var cx=0;cx<textmatrix_displaydata[y].length;cx++) {
//    newline += textmatrix_displaydata[y][cx].char;
//  }
  newline = textmatrix_getline(y);
  
  //var newline = stridx(line,x,c);

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
