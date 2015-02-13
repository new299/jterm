  var c=0;
  var m_on_recv;

  
  function queue_connect(on_recv_in) {
    jss_recv_cb(function() {});
    m_on_recv = on_recv_in;
    setTimeout(function() {
      result = Module.ccall('webssh2_connect','number',[],[]);
      if(result != 0) { 
          if(c < 20) {
            if(jss_closed() != true) queue_connect();
            c++;
          } else {
            connect_fail("connect failed");
          }
        } else {
          c=0;
          if(jss_closed() != true) queue_handshake();
        }
    }, 25);
  }

  function queue_handshake() {
    setTimeout(function() {
      result = Module.ccall('webssh2_handshake','number',[],[]);
      if(result != 0) { 
          if(jss_closed() != true) {
             if(c < 100) {
               queue_handshake();
               c++;
             } else {
               connect_fail("handshake failed");
             }
          }
        } else {
          c=0;
          if(jss_closed() != true) webssh_phase2();
        }
    }, 100);
  }
  
  function webssh_phase2() {
    Module.ccall('webssh2_fingerprint','number',[],[]);
    c=0;
    queue_authcheck();
  }

  function queue_authcheck() {
    setTimeout(function() {
      result = Module.ccall('webssh2_authcheck','number',[],[]);
      if(result == 0) { 
        if(c < 50) {
          if(jss_closed() != true) queue_authcheck();
          c++;
        } else {
          connect_fail("authcheck failed");
        }
      } else {
        c=0;
        queue_authenticate();
      }
    }, 100);
  }
  
  function queue_authenticate() {
    setTimeout(function() {
      result = Module.ccall('webssh2_authenticate','number',[],[]);
      if(result == 0) { 
        if(c < 50) {
          if(jss_closed() != true) queue_authenticate();
          c++;
        } else {
          connect_fail("authentication failed");
        }
      } else {
        c=0;
        queue_requestshell();
      }
    }, 100);
  }

  function queue_requestshell() {
    setTimeout(function() {
      result = Module.ccall('webssh2_requestshell','number',[],[]);
      if(result == 0) { 
        if(c < 50) {
          if(jss_closed() != true) queue_requestshell();
          c++;
        } else {
          connect_fail("requesting shell failed");
        }
      } else {
        c=0;
        queue_setenv();
      }
    }, 100);
  }

  function queue_setenv() {
    setTimeout(function() {
      result = Module.ccall('webssh2_setenv','number',[],[]);
      if(result == 0) { 
        if(c < 25) {
          if(jss_closed() != true) queue_setenv();
          c++;
        } else {
          connect_fail("setenv failed");
        }
      } else {
        c=0;
        queue_setterm();
      }
    }, 25);
  }
  
  function queue_setterm() {
    setTimeout(function() {
      result = Module.ccall('webssh2_setterm','number',[],[]);
      if(result == 0) { 
        if(c < 25) {
          if(jss_closed() != true) queue_setterm();
          c++;
        } else {
          connect_fail("setterm failed");
        }
      } else {
        c=0;
        queue_getshell();
      }
    }, 25);
  }
  
  function queue_getshell() {
    setTimeout(function() {
      result = Module.ccall('webssh2_getshell','number',[],[]);
      if(result == 0) { 
        if(c < 25) {
          if(jss_closed() != true) queue_getshell();
          c++;
        } else {
          connect_fail("failed to get shell");
        }
      } else {
        c=0;
        queue_read();
      }
    }, 100);
  }


  function queue_read() {
    jss_recv_cb(m_on_recv);
  }

  var strptr = Module._malloc(2048); // malloc outside function, as emscripten can't free memory...
  function ssh_send(data) {
    var mystring = data;
    Module.writeAsciiToMemory(mystring, strptr);
    setValue(strptr+mystring.length+1,0,'i8');
  
    result = Module.ccall('webssh2_write','number',['number','number'],[strptr,mystring.length]);
  }
