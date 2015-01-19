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

    //conn.SetReadDeadline(time.Now().Add(1 * time.Second))
    //conn.SetReadDeadline(time.Now().Add(10000000 * time.Nanosecond))
    n,err := conn.Read(tcpbuffer)
    if err == io.EOF { fmt.Printf("TCP Read failed"); break; }
    if err == nil {
//      fmt.Printf("Forwarding from tcp to ws: %d bytes: %s\n",n,tcpbuffer)
//      print_binary(tcpbuffer)
      wsconn.WriteMessage(websocket.BinaryMessage,tcpbuffer[:n])
    }
  }
}

func forwardws (wsconn *websocket.Conn,conn net.Conn) {

 for {
    // Send pending data to tcp socket
    //wsconn.SetReadDeadline(time.Now().Add(1 * time.Second))
    //wsconn.SetReadDeadline(time.Now().Add(10000000 * time.Nanosecond))
    n,buffer,err := wsconn.ReadMessage()
    if err == io.EOF { fmt.Printf("WS Read Failed %d",n); break; }
    if err == nil {
//      s := string(buffer[:len(buffer)])
//      fmt.Printf("Received (from ws) forwarding to tcp: %d bytes: %s %d\n",len(buffer),s,n)
//      print_binary(buffer)
      conn.Write(buffer)
    }
  }
}

func wsProxyHandler(w http.ResponseWriter, r *http.Request) {

  wsconn, err := upgrader.Upgrade(w, r, nil)

  if err != nil {
    //log.Println(err)
    return
  }

  // get connection address and port
  address := make([]byte, 16)

  n,address,err := wsconn.ReadMessage()
  if err != nil {
    fmt.Printf("address read error");
    fmt.Printf("read %d bytes",n);  
  }

  print_binary(address)

  host, port := address_decode(address)

  conn, err := net.Dial("tcp", host + ":" + port)
  if err != nil {
	// handle error
  }


//  fmt.Fprintf(conn, "GET / HTTP/1.0\r\n\r\n")
//  status, err := bufio.NewReader(conn).ReadString('\n')

  go forwardtcp(wsconn,conn)
  go forwardws(wsconn,conn)

// forward traffic to TCP socket
// for {
//
//
//    fmt.Printf("loop\n");
//  }
  fmt.Printf("websocket closed");
}
 
func main() {
  http.HandleFunc("/echo", wsProxyHandler)
  http.Handle("/", http.FileServer(http.Dir(".")))
  err := http.ListenAndServe(":8080", nil)
  if err != nil {
    panic("Error: " + err.Error())
  }
}
