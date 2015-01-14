
var serversocket;
var ws_buffer = [];
var m_jss_ready = false;
var m_jss_closed = false;
var jss_first = true;

var ws_addr;
var ws_addrlen;

function format_for_ws(buffer,length) {

  var send_buffer = new Uint8Array(length);
//  send_buffer = "";
  dbg_buffer = [];
  for(n=0;n<length;n++) {
    dbg_buffer.push(getValue(buffer+n));
//    send_buffer = send_buffer + String.fromCharCode(getValue(buffer+n));
    send_buffer[n] = getValue(buffer+n);
  }

  console.debug("send buffer: " + dbg_buffer);

  return send_buffer;
}

function unformat_from_ws(buffer) {
  return buffer;
}

function addr_to_string(addr) {
  return addr;
}

function jss_send(sock,buffer,length,flags) {

  console.debug("jss_send called, length: " + length);
  console.debug("send readyState: " + serversocket.readyState);
  if(serversocket.readyState == 1) m_jss_ready = true;
                              else m_jss_ready = false;

  if(serversocket.readyState == 3) {
    m_jss_closed = true;
    return -107; //Connection closed
  }

  if(m_jss_ready == false) {return -11;} //EAGAIN

  var data = format_for_ws(buffer,length);
  console.debug("sending stuff len: " + length);
  serversocket.send(data);

  return length;
}

function jss_recv(sock,buffer,length,flags) {

  if(serversocket.readyState == 3) {
    m_jss_closed = true;
    return -107; // connection closed
  }
 
  console.debug("jss_recv called, length: " + length);
  console.debug("ws_buffer size: " + ws_buffer.length);

  if(ws_buffer.length == 0) {return -11;} //EAGAIN

  var recv_len = 0;
  for(n=0;n<length;n++) {
    if(n<ws_buffer.length) {
      setValue(buffer+n,ws_buffer[n],'i8');
      console.debug("sending: " + ws_buffer[n]);
      recv_len++;
    }
  }
  console.debug("buffer: " + buffer);
  console.debug("jss_recv returning len: " + recv_len);

  ws_buffer = ws_buffer.slice(recv_len,ws_buffer.length);
 
  return recv_len;
}

function jss_socket(domain,type,protocol) {
  return 1;
}

function jss_connect(sockfd,addr,addrlen) {

  console.debug("jss_connect");
  ws_addr = addr;
  ws_addrlen = addrlen;

  serversocket = new WebSocket("ws://localhost:8080/echo");
  serversocket.binaryType = "arraybuffer";
  serversocket.onopen = function() {
    m_jss_ready = true;
    console.debug("websocket open complete");
    console.debug("open readyState: " + serversocket.readyState);
//    var address_string = addr_to_string(ws_addr,ws_addrlen);
    serversocket.send(format_for_ws(ws_addr,ws_addrlen));
  }

  serversocket.onclose = function (e) {
    m_jss_ready = false;
    m_jss_closed = true;
    console.debug("*************************** websocket was closed");
    console.debug("code: " + e.code + " " + e.reason);
  }

  console.debug("connect readyState: " + serversocket.readyState);

  // Write message on receive
  serversocket.onmessage = function(e) {
    console.debug("serversocket onmessage received data: " + e.data);
    var array = new Uint8Array(e.data);

    var s = "";
    for(n=0;n<array.length;n++) {
      s += array[n] + ",";
    }
    console.debug("serversocket onmessage received data deab: " + s);
    console.debug("buffer size now: " + ws_buffer.length);
    for(n=0;n<array.length;n++) ws_buffer.push(array[n]);
    console.debug("buffer size now: " + ws_buffer.length);
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
