var textmatrix_displaydata = []; // array arranged by y x

function add_textmatrix(x,y) {

  for(var cy=0;cy<y;cy++) {

    cline = "<div style=\"display: inline\" id=\"l" + cy + "\">"

    textmatrix_displaydata.push([]);
    for(var cx=0;cx<x;cx++) {
      cline += "A";
      textmatrix_displaydata[cy].push({"char":"A", "fg":0xFFFFFF, "bg":0x000000});
    }
    cline += "</div><br>";
    document.getElementById('tml').innerHTML += cline;
  }
}

//function stridx(str,index, character) {
//    return str.substr(0, index) + character + str.substr(index+character.length);
//}

function textmatrix_setpos(x,y,c,fg,bg) {

  textmatrix_displaydata[y][x].char = c;
  textmatrix_displaydata[y][x].fg   = fg;
  textmatrix_displaydata[y][x].bg   = bg;

  textmatrix_redraw_line(y);
}

function textmatrix_redraw_line(y) {

  var newline;

//  for(var cx=0;cx<textmatrix_displaydata[y].length;cx++) {
//    newline += textmatrix_displaydata[y][cx].char;
//  }
  newline = textmatrix_getline(y);
  

  //<p style="color: #000000; background-color: #ffffff; display: inline">Text</p><p style="color: #ffffff; background-color: #000000; display: inline">Text</p>
  //var line = document.getElementById('l' + y).innerHTML;

  //var newline = stridx(line,x,c);

  document.getElementById('l' + y).innerHTML = newline;
}

function textmatrix_setline(y,newline) {
//  console.debug("newline: " + newline);
  for(var cx=0;cx<newline.length;cx++) {
    textmatrix_displaydata[y][cx].char = newline.substr(cx,1);
  }
  textmatrix_redraw_line(y);

//  document.getElementById('l' + y).innerHTML = newline;
}

function textmatrix_getline(y) {
  var data ="";
  for(var cx=0;cx<textmatrix_displaydata[y].length;cx++) {
    data = data + textmatrix_displaydata[y][cx].char;
  }

  return data;
//  var line = document.getElementById('l' + y).innerHTML;
//  return line;
}
