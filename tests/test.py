import socket
import threading

# Create socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("localhost", 8000))
sock.listen(10)

def process(con_sock):
    print("process():", con_sock)

    # Blocking read
    req = con_sock.recv(4096).decode()
    print(req)

    # Send HTTP response
    response = (
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 13\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, World!"
    )
    con_sock.sendall(response.encode())
    con_sock.close()

while True:
    try:
        con_sock, addr = sock.accept()

        # Create and start a new thread
        th = threading.Thread(target=process, args=(con_sock,))
        th.start()

    except Exception as e:
        print("Error:", e)
