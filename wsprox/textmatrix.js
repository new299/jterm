function add_textmatrix(x,y,loc) {

  for(var cy=0;cy<y;cy++) {

    cline = "<pre style=\"display: inline\" id=\"l" + cy + "\">"
    for(var cx=0;cx<x;cx++) {
      cline += "A";
    }
    cline += "</pre><br>";
    document.getElementById('tml').innerHTML += cline;
  }
}

function stridx(str,index, character) {
    return str.substr(0, index) + character + str.substr(index+character.length);
}

function textmatrix_setpos(x,y,c) {

  var line = document.getElementById('l' + y).innerHTML;

  var newline = stridx(line,x,c);

  document.getElementById('l' + y).innerHTML = newline;
}
