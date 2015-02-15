package main
 
import (
    "github.com/gorilla/websocket"
    "net/http"
    "net"
    "fmt"
    "flag"
    //"time"
    //"io"
)

var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
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

 for {
    // Send pending data to tcp socket
    n,buffer,err := wsconn.ReadMessage()
    if err != nil {
      fmt.Printf("WS Read Failed %d",n)
      return
    } else {
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
  n,c,err := wsconn.ReadMessage()
  if err != nil {
    fmt.Printf("address read error");
    fmt.Printf("read %d bytes",n);
    return
  } else {
    address = string(c[:len(c)-1])
  }

  fmt.Printf("address: %s\n",address);

  conn, err := net.Dial("tcp", address)
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
  flag.BoolVar  (&usessl  , "usessl"  , false      , "use ssl?")
  flag.StringVar(&sslcert, "sslcert", "cert.pem", "SSL Certificate filename")
  flag.StringVar(&sslkey , "sslkey",  "key.pem" , "SSL Key filename")
  flag.Parse()

  fmt.Printf("ussssl %s\n",usessl)

  http.HandleFunc("/con", wsProxyHandler)
  http.Handle("/", http.FileServer(http.Dir(".")))

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
