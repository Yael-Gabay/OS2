# OS2
In this exercise we were asked to deal with the SIMPLE HTTP communication protocol, a protocol which is actually a very limited version of HTTP.
This protocol contains two instructions GET and POST.
## Part 1- server:
We were asked to write a server that supports this protocol.
The server receives only one parameter (in argv) describing the root directory of the server. The server supports multiple simultaneous downloads using multiple processes. This server is based on TCP communication for transferring data on the Internet.

Now we will explain about the server code:
At the beginning of the code we defined constants that affect the conduct of the program.
+ PORT: This constant indicates the port where the server is ready and waiting to receive requests from clients to connect. This is the initial step in the process of establishing a TCP connection between a server and a client. Every client that wants to connect to the server must use the same port to connect.
+ BACKLOG: This constant specifies the size of the connection request queue. When there are many clients who want to connect to the server at the same time, BACKLOG indicates the maximum size of the queue where the requests are waiting to be processed.
+ BUFFER_SIZE: This constant specifies the size of the buffer in which the program receives and sends data over the network. It does this with a certain amount of data that is defined in advance. The buffer size indicates the amount of data that can be read or written at a time.

Then we defined some functions that are needed in the server program:
 + *void handle_client(int client_socket, char *root_dir):* The handle_client function handles the client that is connected to the server. The function receives the client's socket ID and the server's primary path. Initializes a character array with the size of BUFFER_SIZE and resets it. First receives data from the appropriate socket of the client, enters the created array and returns a RECV value if the call was successful then continues to process the data received from the client otherwise prints an error and exits the function. The function breaks down the request by spaces and separates the first field (the request action, GET or POST) and the second field (the path of the file or resource required for the request). If the first operation in the request is GET, it receives the desired path and calls the function handle_get_request in order to handle the request. If the first action in the request is POST, it receives the desired path and calls the handle_post_request function to handle the request.
 + *void handle_get_request(int client_socket, char *remote_path, char *root_dir):* This function handles the GET instruction.
First establish the full file path: this function receives the client's socket ID (client_socket), the requested file path (remote_path) and the server's main path (root_dir). It uses these two paths to build the full path of the file to be read from the server. This is done by concatenating the two paths using the snprintf function. After the full path is built, the function tries to open the file using the open function. The read opens a read-only file (O_RDONLY). If the opening failed (the open function returns a negative value), the function prints an error, and if the reason was that the file was not found, it sends a "404 FILE NOT FOUND" error response. If the opening was successful, the function uses the fstat function to get the file details (its size and other details). If the fstat call fails, the function sends an error response to the client. If all tests pass successfully, the function prepares the response head using the snprintf function. The header contains the correct code "200 OK" and the length of the requested file. The response header also incorporates the file length using the file_stat.st_size variable. It then sends the response header to the client using the send_response function. After the response header is sent, the function uses sendfile to send the contents of the file to the client. This function receives the file ID, the client's socket ID, and the location in the file to send the data from. 
 + *void handle_post_request(int client_socket, char *remote_path, char *root_dir):*
 + *void send_response(int client_socket, char *response):* 

