
var serversocket = new WebSocket("ws://localhost:8080/echo");
var ws_buffer = [];

// Write message on receive
serversocket.onmessage = function(e) {
  var data = unformat_from_ws(ws_buffer);
  ws_buffer += e.data;
};

function format_for_ws(buffer) {
  return buffer;
}

function unformat_from_ws(buffer) {
  return buffer;
}

function jss_send(sock,buffer,length,flags) {

  var data = format_for_ws(buffer);

  serversocket.send(data);

  return length;
}

function jss_recv(sock,buffer,length,flags) {
 

  var recv_len = 0;
  for(n=0;n<length;n++) {
    if(n<data.length) {
      setValue(buffer+n,ws_buffer[n],'i8');
      recv_len++;
    }
  }

  ws_buffer.slice(recv_len,ws_buffer.length);
 
  return recv_len;
}

function jss_socket(domain,type,protocol) {
  return 1;
}

function jss_connect(sockfd,addr,addrlen) {

  serversocket.onopen = function() {
    var address_string = addr_to_string(addr,addrlen);
    serversocket.send(address_string);
  }

  return 0; // success
}
