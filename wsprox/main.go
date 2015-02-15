package main
 
import (
    "github.com/gorilla/websocket"
    "github.com/daaku/go.httpgzip"
    "net/http"
    "net"
    "fmt"
    "flag"
    "strings"
    "encoding/binary"
    "bytes"
    "errors"
    "strconv"
)
  
var usesocks string

var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
}

func socks_connect(conn net.Conn, address string) (error) {

	host, portstr, err := net.SplitHostPort(address)
	port, err2 := strconv.Atoi(portstr)
        if err2 != nil {
          return errors.New("Unable to parse port")
        }

	version := []byte{0x04} // socks version 4
	cmd := []byte{0x01}     // socks stream mode
	buffer := bytes.NewBuffer([]byte{})
	binary.Write(buffer, binary.BigEndian, version)
	binary.Write(buffer, binary.BigEndian, cmd)
	binary.Write(buffer, binary.BigEndian, uint16(port))
	binary.Write(buffer, binary.BigEndian, []byte{0x00, 0x00, 0x00, 0x01})
	binary.Write(buffer, binary.BigEndian, []byte{0x00})
	binary.Write(buffer, binary.BigEndian, []byte(host))
	binary.Write(buffer, binary.BigEndian, []byte{0x00})
	binary.Write(conn, binary.BigEndian, buffer.Bytes())
	
	data := make([]byte, 8) // socks responses are 8 bytes
	count, err := conn.Read(data)

        if err != nil {
          return errors.New("Unable to connect to socks server.")
        }
        if count == 0 {
          return errors.New("Unable to connect to socks server.")
        }
	if data[1] == 0x5a { // success
		return nil
	}
        
	return errors.New("Unable to connect to socks server.")
}

func forwardtcp(wsconn *websocket.Conn,conn net.Conn) {

  for {
    // Receive and forward pending data from tcp socket to web socket
    tcpbuffer := make([]byte, 1024)

    n,err := conn.Read(tcpbuffer)
    if err != nil {
      fmt.Printf("TCP Read failed")
      return
    } else {
      wsconn.WriteMessage(websocket.BinaryMessage,tcpbuffer[:n])
    }
  }
}

func forwardws (wsconn *websocket.Conn,conn net.Conn) {

 var first = true
 for {
    // Send pending data to tcp socket
    _,buffer,err := wsconn.ReadMessage()
    if err != nil {
      return
    } else {
      if first == true {
        str := string(buffer[:])
        if strings.Contains(str,"SSH-2.0-libssh") == false {
          return
        }
        first = false
      }
      conn.Write(buffer)
    }
  }
}

func wsProxyHandler(w http.ResponseWriter, r *http.Request) {

  wsconn, err := upgrader.Upgrade(w, r, nil)

  if err != nil {
    return
  }

  // get connection address and port
  var address string;
  _,c,err := wsconn.ReadMessage()
  if err != nil {
    fmt.Printf("address read error");
    return
  } else {
    address = string(c[:len(c)-1])
  }

  var conn net.Conn
  if (usesocks == "") || strings.Contains(address,"localhost") {
    conn, err = net.Dial("tcp", address)
  } else {
    conn, err = net.Dial("tcp", usesocks)
    if err != nil {
      fmt.Printf("Socks server connection error");
    }
    err = socks_connect(conn, address)
    if err != nil {
      fmt.Printf("Socks proxy connection error");
    }
  }

  if err != nil {
    // handle error
    return
  }

  go forwardtcp(wsconn,conn)
  go forwardws(wsconn,conn)

}

func main() {
  var usessl bool
  var sslcert string
  var sslkey string
  flag.BoolVar  (&usessl  , "usessl"  , false           , "use ssl?")
  flag.StringVar(&sslcert , "sslcert" , "cert.pem"      , "SSL Certificate filename")
  flag.StringVar(&sslkey  , "sslkey"  , "key.pem"       , "SSL Key filename")
  flag.StringVar(&usesocks, "usesocks", ""              , "Forward traffic via a SOCKS server")
  flag.Parse()

  http.HandleFunc("/con", wsProxyHandler)
  http.Handle("/", httpgzip.NewHandler(http.FileServer(http.Dir("."))))

  go func() {
    err := http.ListenAndServe(":80", nil)
    if err != nil {
      panic("Error: " + err.Error())
    }
  }()

  if usessl == true {
    fmt.Printf("SSL enabled\n")
    err := http.ListenAndServeTLS(":443", sslcert, sslkey, nil)
    if err != nil {
      panic("Error: " + err.Error())
    }
  } else {
    fmt.Printf("SSL not enabled\n")
  }

}
