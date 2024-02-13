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
