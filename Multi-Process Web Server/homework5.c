#include <fnmatch.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BACKLOG (10)

void serve_request(void*);

char * request_str = "HTTP/1.0 200 OK\r\n"
        "Content-type: text/html; charset=UTF-8\r\n\r\n";

char *request_notFound = "HTTP/1.0 404 Not Found\r\n"
        "Content-type: text/html; charset=UTF-8\r\n\r\n";

char notFound[] = "<!DOCTYPE html><html><head><title>Not Found</title><link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\"><style type=\"text/css\">body{background-image:url(\"https://cdn.pixabay.com/photo/2017/09/08/08/57/binary-2728117_960_720.jpg\");background-position: center;background-repeat: no-repeat;background-size: cover;}.container{color: white;text-align: center;margin: auto;width: 66.6%;height: 66.6%;}</style></head><body><div class=\"container\"><h1>404 NOT FOUND</h1><h2>You Broke the Internet</h2><img src=\"https://cdn.mos.cms.futurecdn.net/9vkE5D4ytKM8So2A75Ekbk-970-80.jpg\"></div></body></html>";

char * index_hdr = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html>"
        "<title>Directory listing for %s</title>"
"<body>"
"<h2>Directory listing for %s</h2><hr><ul>";

// snprintf(output_buffer,4096,index_hdr,filename,filename);


char * index_body = "<li><a href=\"%s\">%s</a>";

char * index_ftr = "</ul><hr></body></html>";

/* char* parseRequest(char* request)
 * Args: HTTP request of the form "GET /path/to/resource HTTP/1.X" 
 *
 * Return: the resource requested "/path/to/resource"
 *         0 if the request is not a valid HTTP request 
 * 
 * Does not modify the given request string. 
 * The returned resource should be free'd by the caller function. 
 */
char* parseRequest(char* request) {
  //assume file paths are no more than 256 bytes + 1 for null. 
  char *buffer = malloc(sizeof(char)*257);
  memset(buffer, 0, 257);
  
  if(fnmatch("GET * HTTP/1.*",  request, 0)) return 0; 

  sscanf(request, "GET %s HTTP/1.", buffer);
  return buffer; 
}

// takes an array and reverses it
// 'Khan' becomes 'nahK'
char *reverseArr(char *arr)
{
  char *tmp = (char*)malloc(sizeof(char) * 10);
  int length = strlen(arr) - 1;
  int counter = strlen(arr) - 1;
  int i;
  for (i = 0; i <= length; i++)
  {
    tmp[i] = arr[counter];
    counter--;
  }
  tmp[strlen(arr)] = '\0';
  return tmp;
}

// takes in the requested_file URL,
// starts from the back of the string,
// and stores every character up until the first period
// so if the extension is '.html', it gets 'lmth'
// then calls the reverse function to reverse it to 'html'
// and then returns it
char *parseType(char *requested_file)
{
  int counter = 0;
  char *tmp = (char*)malloc(sizeof(char) * 10);
  int i;
  for (i = strlen(requested_file) - 1; requested_file[i] != '.'; i--)
  {
    tmp[counter] = requested_file[i];
    counter++;
  }
  tmp[counter] = '\0';

  return reverseArr(tmp);
}

char begin[] = "HTTP/1.0 200 OK\r\nContent-type: ";
char html[] = "text/html; charset=UTF-8\r\n\r\n";
char jpg[] = "image/jpg; charset=UTF-8\r\n\r\n";
char jpeg[] = "image/jpeg; charset=UTF-8\r\n\r\n";
char gif[] = "image/gif; charset=UTF-8\r\n\r\n";
char png[] = "image/png; charset=UTF-8\r\n\r\n";
char text[] = "text/plain; charset=UTF-8\r\n\r\n";
char pdf[] = "application/pdf; charset=UTF-8\r\n\r\n";

// takes in an extension, and sees what it is
// depending on what the extension is, it will
// create a new 'requested_str' and returns it
char *requestStrChange(char *arr)
{
  char *full = (char*)malloc(sizeof(char) * 60);

  strcat(full, begin);

  if (strcmp(arr, "html") == 0)
    strcat(full, html);
  else if (strcmp(arr, "jpg") == 0)
    strcat(full, jpg);
  else if (strcmp(arr, "jpeg") == 0)
    strcat(full, jpeg);
  else if (strcmp(arr, "gif") == 0)
    strcat(full, gif);
  else if (strcmp(arr, "png") == 0)
    strcat(full, png);
  else if (strcmp(arr, "txt") == 0)
    strcat(full, text);
  else if (strcmp(arr, "pdf") == 0)
    strcat(full, pdf);
  else if (strcmp(arr, "ico") == 0)
    strcpy(full, "ico");
  else
    strcpy(full, "404");

  return full;
}

void serve_request(void* client_fd1)
{
  int client_fd = *((int*)client_fd1);
  int read_fd;
  int bytes_read;
  int file_offset = 0;
  char client_buf[4096];
  char send_buf[4096];
  char filename[4096];
  char * requested_file;
  memset(client_buf,0,4096);
  memset(filename,0,4096);

  while(1)
  {
    file_offset += recv(client_fd,&client_buf[file_offset],4096,0);
    if(strstr(client_buf,"\r\n\r\n"))
      break;
  }

  requested_file = parseRequest(client_buf);

  char lastLetter = requested_file[strlen(requested_file) - 1];
  if (lastLetter == 'W')
    strcat(requested_file, "/index.html");
  else if (lastLetter == '/')
    strcat(requested_file, "index.html");

  request_str = requestStrChange(parseType(requested_file));

  if (strcmp(request_str, "ico") == 0)
  {
    // favicon handler
    return;
  }
  else if (strcmp(request_str, "404") == 0)
  {
    // file not found
    send(client_fd, request_notFound, strlen(request_notFound), 0);
    send(client_fd, notFound, strlen(notFound), 0);
  }
  else
  {
    // take requested_file, add a . to beginning, open that file
    filename[0] = '.';
    strncpy(&filename[1],requested_file,4095);

    read_fd = open(filename,0,0);

    // if the file extension is valid, but the actual file was not found
    if (read_fd < 0)
    {
      send(client_fd, request_notFound, strlen(request_notFound), 0);
      send(client_fd, notFound, strlen(notFound), 0);
    }
    else
    {
      send(client_fd,request_str,strlen(request_str),0);
      // the file extension is valid and the file exists: open it
      
      while(1)
      {
        bytes_read = read(read_fd,send_buf,4096);
        if(bytes_read == 0)
          break;

        send(client_fd,send_buf,bytes_read,0);
      } 
    }
  }
  close(read_fd);
  close(client_fd);

  free(request_str);

  return;
}








/* Your program should take two arguments:
 * 1) The port number on which to bind and listen for connections, and
 * 2) The directory out of which to serve files.
 */
int main(int argc, char** argv) {
    /* For checking return values. */
    int retval;

    /* Read the port number from the first command line argument. */
    int port = atoi(argv[1]);
    char dir[200] = "./";
    strcat(dir, argv[2]);
    int ret_ch_dir= chdir(dir);
    if (ret_ch_dir!=0){
      perror("error changing dir");
      exit(1);
     }
    

    /* Create a socket to which clients will connect. */
    int server_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(server_sock < 0) {
        perror("Creating socket failed");
        exit(1);
    }

    /* A server socket is bound to a port, which it will listen on for incoming
     * connections.  By default, when a bound socket is closed, the OS waits a
     * couple of minutes before allowing the port to be re-used.  This is
     * inconvenient when you're developing an application, since it means that
     * you have to wait a minute or two after you run to try things again, so
     * we can disable the wait time by setting a socket option called
     * SO_REUSEADDR, which tells the OS that we want to be able to immediately
     * re-bind to that same port. See the socket(7) man page ("man 7 socket")
     * and setsockopt(2) pages for more details about socket options. */
    int reuse_true = 1;
    retval = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));
    if (retval < 0) {
        perror("Setting socket option failed");
        exit(1);
    }

    /* Create an address structure.  This is very similar to what we saw on the
     * client side, only this time, we're not telling the OS where to connect,
     * we're telling it to bind to a particular address and port to receive
     * incoming connections.  Like the client side, we must use htons() to put
     * the port number in network byte order.  When specifying the IP address,
     * we use a special constant, INADDR_ANY, which tells the OS to bind to all
     * of the system's addresses.  If your machine has multiple network
     * interfaces, and you only wanted to accept connections from one of them,
     * you could supply the address of the interface you wanted to use here. */
    
   
    struct sockaddr_in6 addr;   // internet socket address data structure
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port); // byte order is significant
    addr.sin6_addr = in6addr_any; // listen to all interfaces

    
    /* As its name implies, this system call asks the OS to bind the socket to
     * address and port specified above. */
    retval = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    if(retval < 0) {
        perror("Error binding to port");
        exit(1);
    }

    /* Now that we've bound to an address and port, we tell the OS that we're
     * ready to start listening for client connections.  This effectively
     * activates the server socket.  BACKLOG (#defined above) tells the OS how
     * much space to reserve for incoming connections that have not yet been
     * accepted. */
    retval = listen(server_sock, BACKLOG);
    if(retval < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    while(1) {
        /* Declare a socket for the client connection. */
        int *sock=malloc(sizeof(int));
        char buffer[256];

        /* Another address structure.  This time, the system will automatically
         * fill it in, when we accept a connection, to tell us where the
         * connection came from. */
        struct sockaddr_in remote_addr;
        unsigned int socklen = sizeof(remote_addr); 

        /* Accept the first waiting connection from the server socket and
         * populate the address information.  The result (sock) is a socket
         * descriptor for the conversation with the newly connected client.  If
         * there are no pending connections in the back log, this function will
         * block indefinitely while waiting for a client connection to be made.
         * */
        *sock = accept(server_sock, (struct sockaddr*) &remote_addr, &socklen);
        if(sock < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        /* At this point, you have a connected socket (named sock) that you can
         * use to send() and recv(). */

        /* ALWAYS check the return value of send().  Also, don't hardcode
         * values.  This is just an example.  Do as I say, not as I do, etc. */
        pthread_t p = NULL;
         int retval = pthread_create(&p, NULL, (void *) serve_request, sock);

         if (retval) {
            printf("pthread_create() failed\n");
            exit(1);
        }
        //serve_request(sock);

        /* Tell the OS to clean up the resources associated with that client
         * connection, now that we're done with it. */
        //close(sock);
    }

    close(server_sock);
}
