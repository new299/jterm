
var serversocket;
var ws_buffer = [];
var m_jss_ready = false;
var m_jss_closed = false;
var jss_first = true;

var m_server_address;

function format_for_ws(buffer,length) {

  var send_buffer = new Uint8Array(length);
  dbg_buffer = [];
  for(n=0;n<length;n++) {
    dbg_buffer.push(getValue(buffer+n));
    send_buffer[n] = getValue(buffer+n);
  }


  return send_buffer;
}

function unformat_from_ws(buffer) {
  return buffer;
}

function addr_to_string(addr) {
  return addr;
}

function jss_send(sock,buffer,length,flags) {

  if(serversocket.readyState == 1) m_jss_ready = true;
                              else m_jss_ready = false;

  if(serversocket.readyState == 3) {
    m_jss_closed = true;
    return -107; //Connection closed
  }

  if(m_jss_ready == false) {return -11;} //EAGAIN

  var data = format_for_ws(buffer,length);
  serversocket.send(data);

  return length;
}

var jss_recv_callback = 101;

function jss_recv(sock,buffer,length,flags) {

  if(serversocket.readyState == 3) {
    m_jss_closed = true;
    return -107; // connection closed
  }
 
  if(ws_buffer.length == 0) {return -11;} //EAGAIN

  var recv_len = 0;
  for(n=0;n<length;n++) {
    if(n<ws_buffer.length) {
      setValue(buffer+n,ws_buffer[n],'i8');
      recv_len++;
    }
  }

  ws_buffer = ws_buffer.slice(recv_len,ws_buffer.length);

  if(jss_recv_callback != 101) {
    jss_recv_callback();
  }
 
  return recv_len;
}

function jss_recv_cb(recv_cb) {
  jss_recv_callback = recv_cb;
}

function jss_socket(domain,type,protocol) {
  return 1;
}

//function jss_gethostbyname(name) {
//  var xmlHttp = new XMLHttpRequest();
//  xmlHttp.open("GET","http://localhost:8080/dns?name=" + name, false);
//  xmlHttp.send(null);
//  console.debug("hostname: " + xmlHttp.responseText);
//  return xmlHttp.responseText;
//}

function jss_set_server_address(addr) {
  m_server_address = addr;
}

function get_wsurl() {
  var loc = window.location, new_uri;
  if (loc.protocol === "https:") {
    new_uri = "wss:";
  } else {
    new_uri = "ws:";
  }
  new_uri += "//" + loc.host;
  new_uri += loc.pathname + "con";
  console.debug("ws url: " + new_uri);
  return new_uri;
}

function jss_connect(sockfd,addr,addrlen) {

  //serversocket = new WebSocket("ws://localhost:8080/con");
  serversocket = new WebSocket(get_wsurl());
  serversocket.binaryType = "arraybuffer";
  serversocket.onopen = function() {

    var send_buffer = new Uint8Array(m_server_address.length+1);
    for(var n=0;n<m_server_address.length;n++) {
      send_buffer[n] = m_server_address[n].charCodeAt();
    }
    send_buffer[m_server_address.length]=0;
    serversocket.send(send_buffer);
    m_jss_ready = true;
  }

  serversocket.onclose = function (e) {
    m_jss_ready = false;
    m_jss_closed = true;
    disconnected();
  }

  // Write message on receive
  serversocket.onmessage = function(e) {
    var array = new Uint8Array(e.data);

    for(n=0;n<array.length;n++) ws_buffer.push(array[n]);

    if(typeof(jss_recv_callback == "function")) {
      jss_recv_callback();
    }
  };

  return 0; // success
}

function jss_closed() {
  return m_jss_closed;
}

function jss_ready() {
  return m_jss_ready;
}

function jss_close() {
  serversocket.onclose = function () {}; // disable onclose handler first
  m_jss_ready=false;
  m_jss_closed=true;
  serversocket.close();
}
