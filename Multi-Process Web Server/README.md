## Objectives
* Learn how to follow a network protocol (in this case, http)
* Learn network/socket programming
* Learn some simple multithreading

For this assignment I wrote a simple a simple multi-threaded Web server.

server program will receive two arguments: 
1. the port number it should listen on for incoming connections
2. the directory out of which it will serve files (often called the document root in production Web servers). For example:
$ ./homework5 8080 WWW

This command will tell your Web server to listen for connections on port 8080 and serve files out of the WWW directory. That is, the WWW directory is considered ‘/’ when responding to requests. For example, if you’re asked for /index.html, you should respond with the file that resides in WWW/index.html. If you’re asked for /dir1/dir2/file.ext, you should respond with the file WWW/dir1/dir2/file.ext.

In addition to serving requested files, server should handle at least the following cases:

* HTML, text, and image files all display properly. Returns the proper HTTP Content-Type header in response, based on the file ending. Also, other binary file types like PDFs should be handled correctly. Able to handle files with html, txt, jpeg, gif, png, and pdf extensions.

* If asked for a file that does not exist, responds with a 404 error code with a readable error page, just like a Web server would.

* Some clients may be slow to complete a connection or send a request. Server should be able to serve multiple clients concurrently, not just back-to-back. For this lab, used multithreading with pthreads to handle concurrent connections. 

* If the path requested by the client is a directory, you should handle the request as if it was for the file “index.html” inside that directory.

My port number for this project was 8009.