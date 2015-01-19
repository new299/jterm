  var c=0;
  var m_on_recv;

  function webssh_phase2() {
    Module.ccall('webssh2_fingerprint','number',[],[]);
    c=0;
    queue_authcheck();
  }
  
  function queue_connect(on_recv_in) {
    jss_recv_cb(function() {});
    m_on_recv = on_recv_in;
    setTimeout(function() {
      result = Module.ccall('webssh2_connect','number',[],[]);
      if(result != 0) { 
          if(jss_closed() != true) queue_connect(); 
          console.debug("connect failed ret: " + result);
        } else {
          console.debug("connect completed successfully ret: " + result);
          if(jss_closed() != true) queue_handshake();
        }
    }, 100);
  }

  function queue_handshake() {
    setTimeout(function() {
      result = Module.ccall('webssh2_handshake','number',[],[]);
      if(result != 0) { 
          console.debug("handshake failed ret: " + result);
          if(jss_closed() != true) {
             if(c < 100) {
               queue_handshake();
               c=c+1;
             }
          }
        } else {
          console.debug("handshake completed successfully ret: " + result);
          if(jss_closed() != true) webssh_phase2();
        }
    }, 100);
  }

  function queue_authcheck() {
    setTimeout(function() {
      result = Module.ccall('webssh2_authcheck','number',[],[]);
      if(result == 0) { 
        console.debug("authcheck failed: " + result);
        if(c < 50) {
          if(jss_closed() != true) queue_authcheck();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("authcheck completed successfully");
        queue_authenticate();
      }
    }, 100);
  }
  
  function queue_authenticate() {
    setTimeout(function() {
      result = Module.ccall('webssh2_authenticate','number',[],[]);
      if(result == 0) { 
        console.debug("authenatication failed: " + result);
        if(c < 50) {
          if(jss_closed() != true) queue_authenticate();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("authentication completed successfully");
        queue_requestshell();
      }
    }, 100);
  }

  function queue_requestshell() {
    setTimeout(function() {
      result = Module.ccall('webssh2_requestshell','number',[],[]);
      if(result == 0) { 
        console.debug("requestshell failed: " + result);
        if(c < 50) {
          if(jss_closed() != true) queue_requestshell();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("requestshell completed successfully");
        queue_setenv();
      }
    }, 100);
  }

  function queue_setenv() {
    setTimeout(function() {
      result = Module.ccall('webssh2_setenv','number',[],[]);
      if(result == 0) { 
        console.debug("setenv failed: " + result);
        if(c < 50) {
          if(jss_closed() != true) queue_setenv();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("setenv completed successfully");
        queue_setterm();
      }
    }, 100);
  }
  
  function queue_setterm() {
    setTimeout(function() {
      result = Module.ccall('webssh2_setterm','number',[],[]);
      if(result == 0) { 
        console.debug("setterm failed: " + result);
        if(c < 10) {
          if(jss_closed() != true) queue_setterm();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("setterm completed successfully");
        queue_getshell();
      }
    }, 500);
  }
  
  function queue_getshell() {
    setTimeout(function() {
      result = Module.ccall('webssh2_getshell','number',[],[]);
      if(result == 0) { 
        console.debug("getshell failed: " + result);
        if(c < 10) {
          if(jss_closed() != true) queue_getshell();
          c=c+1;
        }
      } else {
        c=0;
        console.debug("getshell completed successfully");
        queue_read();
      }
    }, 500);
  }


  function queue_read() {
    jss_recv_cb(m_on_recv);
//    setTimeout(function() {
//      m_on_recv();
//      queue_read();
//    }, 2000);
  }


  var strptr = Module._malloc(1024);
  function ssh_send(data) {
    var mystring = data;// + "\r\n";
//    var strptr = Module._malloc(mystring.length+1);
    Module.writeAsciiToMemory(mystring, strptr);
    setValue(strptr+mystring.length+1,0,'i8');
  
    result = Module.ccall('webssh2_write','number',['number','number'],[strptr,mystring.length]);
  }
