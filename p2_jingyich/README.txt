================================================================

    README file for Project Assignment 2 - HTTP Proxy

           Name:  Anqi Dai, Jingyi Chen
 Username/Login:  daianqi, jingyich
 
================================================================

Our HTTP proxy has four helper functions:
1.socketSetup
This function sets up the socket listening to the specific port assigned by the proxy user. It accepts connections with multi-thread clients. The internal steps for setting up the socket is: create the socket, bind it to the port and listen to that port.

2.receivePacket
This is the most important part of this project. It is responsible for the communication between client and proxy and between server and proxy. This function receive packets sent by each connected client. If there is an error during the receiving process, it prints error message. It then sends the message to parseRequest, which convert the message into formatted form. Then it sends the message to the remote server, gets response from the server, and forward the response to the client as it is.

3.sendChar
This function uses the send(sock,buffer,1,0) method.

4.parseRequest
This function parses the request and checks the correctness of the input. It will convert the input into a very strict format and return it. The strict format is as follows:
GET path HTTP/1.0\nHOST: host\n\n


Some tolerable input errors:
1.if method is not "GET", but combination of upper case and lower case, it is allowed
2.inconsistent HTTP version,if the request uses http/1.1, it will convert it to http/1.0
3.extra spaces are allowed
4.if the host is not started with http://, it will respond 400-error
5.GET http://www.google.com HTTP/1.0 -> GET http://www.google.com/ HTTP/1.0

We have passed the given python proxy-test.
We have tested our proxy using Firefox browser(both pictures and videos can be displayed).