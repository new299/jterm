package main
 
import (
    "github.com/gorilla/websocket"
    "net/http"
    "net"
    "fmt"
    //"time"
    "io"
)

var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
}


      
func print_binary(s []byte) {
  fmt.Printf("print b:");
  for n := 0;n < len(s);n++ {
    fmt.Printf("%d,",s[n]);
  }
  fmt.Printf("\n");
}

func address_decode(address_bin []byte) (string,string) {
  
  var host string = "127.0.0.1"
  var port string = "22";

  return host,port
}
 

func forwardtcp(wsconn *websocket.Conn,conn net.Conn) {

  for {
    // Receive and forward pending data from tcp socket to web socket
    tcpbuffer := make([]byte, 1024)

    n,err := conn.Read(tcpbuffer)
    if err == io.EOF { fmt.Printf("TCP Read failed"); break; }
    if err == nil {
      wsconn.WriteMessage(websocket.BinaryMessage,tcpbuffer[:n])
    }
  }
}

func forwardws (wsconn *websocket.Conn,conn net.Conn) {

 for {
    // Send pending data to tcp socket
    n,buffer,err := wsconn.ReadMessage()
    if err == io.EOF { fmt.Printf("WS Read Failed %d",n); break; }
    if err == nil {
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
  } else {
    address = string(c[:len(c)-1])
  }

  fmt.Printf("address: %s",address);
 

  //TODO: validate. Server can fall over here if input is incorrect
  conn, err := net.Dial("tcp", address)
  if err != nil {
	// handle error
    return
  }

  go forwardtcp(wsconn,conn)
  go forwardws(wsconn,conn)

  fmt.Printf("websocket closed");
}

func main() {
  http.HandleFunc("/con", wsProxyHandler)
  http.Handle("/", http.FileServer(http.Dir(".")))
  err := http.ListenAndServe(":8080", nil)
  if err != nil {
    panic("Error: " + err.Error())
  }
}
