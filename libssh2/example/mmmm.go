package main
 
import (
    "golang.org/x/net/websocket"
    "net/http"
    "net"
    "fmt"
)
      
func print_binary(s []byte) {
  fmt.Printf("Received b:");
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
 

func wsProxyHandler(w http.ResponseWriter, r *http.Request) {

  conn, err := upgrader.Upgrade(w, r, nil)

  if err != nil {
    //log.Println(err)
    return
  }


  for {
    messageType, p, err := conn.ReadMessage()
    if err != nil {
      return
    }

    print_binary(p)

    err = conn.WriteMessage(messageType, p);
    if  err != nil {
      return
    }
  }


  // get connection address and port
  address := make([]byte, 16)

  n,err := .Read(address)
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


  // forward traffic to TCP socket
  for {
    buffer := make([]byte, 1024)

    // Send pending data to socket
    n,err := ws.Read(buffer)
    if err == nil {
      s := string(buffer[:n])
      fmt.Printf("Received t: %d bytes: %s\n",n,s)
      print_binary(buffer)
      fmt.Fprintf(conn,s);
    }

    // Receive and forward pending data to socket
    n,err = conn.Read(buffer)
    if err == nil {
      fmt.Printf("Forawrding t: %d bytes: %s\n",n,buffer)
      print_binary(buffer)
      ws.Write(buffer[:n])
    }
  }
}
 
func main() {
  http.Handle("/echo", wsProxyHandler)
  http.Handle("/", http.FileServer(http.Dir(".")))
  err := http.ListenAndServe(":8080", nil)
  if err != nil {
    panic("Error: " + err.Error())
  }
}
