/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

#define MAX_CACHE_SIZE 1049000

static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *new_version = "HTTP/1.0";


void doit(int fd);
void read_requesthdrs(rio_t *rp);
// int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);


void do_request(int clientfd, char *method, char *uri_ptos, char *host);
void do_response(int connfd, int clientfd);
int parse_uri(char *uri, char *uri_ptos, char *host, char *port);



int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    
    doit(connfd);   // line:netp:tiny:doit


    Close(connfd);  // line:netp:tiny:close
  }
}



void doit(int fd)
{
  int clientfd;

  int is_static;
  
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];

  char uri_ptos[MAXLINE];
  char host[MAXLINE], port[MAXLINE];


  rio_t rio;

  // Read request line and headers 
  Rio_readinitb(&rio, fd); 
  Rio_readlineb(&rio, buf, MAXLINE); 

  printf("Request headers to proxy:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  // if(strcmp(method, "GET") && strcmp(method, "HEAD"))
  // {
  //   clienterror(fd, method, "501", "Not Implemented",
  //               "Tiny does not implement this method");
  //   return;
  // }


  read_requesthdrs(&rio);

  // Parse URI from GET request 
  // is_static = parse_uri(uri, filename, cgiargs);
  
  parse_uri(uri, uri_ptos, host, port);
  clientfd = Open_clientfd(host, port);

  do_request(clientfd, method, uri_ptos, host);     // clientfd에 Request headers 저장과 동시에 server의 connfd에 쓰여짐
  do_response(fd, clientfd);

 
  if (stat(filename, &sbuf) < 0){
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
    return;
  }
}


/*
void do_it(int connfd){
  int clientfd;
  char buf[MAXLINE],  host[MAXLINE], port[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char uri_ptos[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);                      // rio 버퍼와 fd(proxy의 connfd)를 연결시켜준다. 
  Rio_readlineb(&rio, buf, MAXLINE);                  // 그리고 rio(==proxy의 connfd)에 있는 한 줄(응답 라인)을 모두 buf로 옮긴다. 
  printf("Request headers to proxy:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);      // buf에서 문자열 3개를 읽어와 각각 method, uri, version이라는 문자열에 저장 

  // Parse URI from GET request 
  // if (!(parse_uri(uri, uri_ptos, host, port)))
  //   return -1;
  parse_uri(uri, uri_ptos, host, port);

  clientfd = Open_clientfd(host, port);             // clientfd = proxy의 clientfd (연결됨)

  do_request(clientfd, method, uri_ptos, host);     // clientfd에 Request headers 저장과 동시에 server의 connfd에 쓰여짐

  do_response(connfd, clientfd);        
  Close(clientfd);                                  // clientfd 역할 끝
}
*/



/* do_request: proxy => server */
void do_request(int clientfd, char *method, char *uri_ptos, char *host){
  char buf[MAXLINE];
  printf("Request headers to server: \n");     
  printf("%s %s %s\n", method, uri_ptos, new_version);

  /* Read request headers */        
  sprintf(buf, "GET %s %s\r\n", uri_ptos, new_version);     // GET /index.html HTTP/1.0
  sprintf(buf, "%sHost: %s\r\n", buf, host);                // Host: www.google.com     
  sprintf(buf, "%s%s", buf, user_agent_hdr);                // User-Agent: ~(bla bla)
  sprintf(buf, "%sConnections: close\r\n", buf);            // Connections: close
  sprintf(buf, "%sProxy-Connection: close\r\n\r\n", buf);   // Proxy-Connection: close

  /* Rio_writen: buf에서 clientfd로 strlen(buf) 바이트로 전송*/
  Rio_writen(clientfd, buf, (size_t)strlen(buf)); // => 적어주는 행위 자체가 요청하는거야~@!@!
}
void do_response(int connfd, int clientfd){
  char buf[MAX_CACHE_SIZE];
  ssize_t n;
  rio_t rio;

  Rio_readinitb(&rio, clientfd);  
  n = Rio_readnb(&rio, buf, MAX_CACHE_SIZE);  
  Rio_writen(connfd, buf, n);
}
int parse_uri(char *uri, char *uri_ptos, char *host, char *port){ 
  char *ptr;

  /* 필요없는 http:// 부분 잘라서 host 추출 */
  if (!(ptr = strstr(uri, "://"))) 
    return -1;                        // ://가 없으면 unvalid uri 
  ptr += 3;                       
  strcpy(host, ptr);                  // host = www.google.com:80/index.html

  /* uri_ptos(proxy => server로 보낼 uri) 추출 */
  if((ptr = strchr(host, '/'))){  
    *ptr = '\0';                      // host = www.google.com:80
    ptr += 1;
    strcpy(uri_ptos, "/");            // uri_ptos = /
    strcat(uri_ptos, ptr);            // uri_ptos = /index.html
  }
  else strcpy(uri_ptos, "/");

  /* port 추출 */
  if ((ptr = strchr(host, ':'))){     // host = www.google.com:80
    *ptr = '\0';                      // host = www.google.com
    ptr += 1;     
    strcpy(port, ptr);                // port = 80
  }  
  else strcpy(port, "80");            // port가 없을 경우 "80"을 넣어줌

  /* 
  Before Parsing (Client로부터 받은 Request Line)
  => GET http://www.google.com:80/index.html HTTP/1.1

  Result Parsing (순차적으로 host, uri_ptos, port으로 파싱됨)
  => host = www.google.com
  => uri_ptos = /index.html
  => port = 80

  After Parsing (Server로 보낼 Request Line)
  => GET /index.html HTTP/11. 
  */ 

  return 0; // function int return => for valid check
}







// HTTP 클라이언트에게 오류 응답을 보내기 위한 함수.
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n",body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body); 

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  // 
  //
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}


void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}
/*
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if (!strstr(uri, "cgi-bin"))
  { 
    // Static content 
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");
    return 1;
  }
  else
  { // Dynamic content 
    ptr = index(uri, '?');
    if (ptr)
    {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, "");

    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}
*/


void serve_static(int fd, char *filename, int filesize, char *method)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  
  //
  srcp = (char *)Malloc(filesize);
  //
  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");

  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);

  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /*
  if (!strcmp(method, "GET")) 
  {
     srcfd = Open(filename, O_RDONLY, 0);
    srcp = (char *)malloc(sizeof(filesize));
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    free(srcp);
  }
  */

  if (strcasecmp(method, "HEAD") == 0) return; 

  /* Open the file and read its content */
  srcfd = Open(filename, O_RDONLY, 0);
  Rio_readn(srcfd, srcp, filesize); // Read file content into memory
  Close(srcfd);
  /* Send response body to client */
  Rio_writen(fd, srcp, filesize);
  /* Free dynamically allocated memory */
  free(srcp);

}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0)
  { /* child */
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to client */
    Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /* Parent waits for and reaps child */
}


void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".css"))
    strcpy(filetype, "text/css");
  else if (strstr(filename, ".js"))
    strcpy(filetype, "application/javascript");
  else if (strstr(filename, ".ico"))
    strcpy(filetype, "image/x-icon");
  
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "video/mp4");
  else if (strstr(filename, ".mpg"))
    strcpy(filetype, "video/mpeg");
  else
    strcpy(filetype, "text/plain");
}