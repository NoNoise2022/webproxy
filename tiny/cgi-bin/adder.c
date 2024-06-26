/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) 
{
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    
    int n1=0, n2=0;

    if ((buf = getenv("QUERY_STRING")) != NULL) 
    { 
	p = strchr(buf, '&'); 
	*p = '\0';


    // strcpy(arg1, buf);
    // strcpy(arg2, p + 1);

	// n1 = atoi(arg1); 
	// n2 = atoi(arg2); 

    sscanf(buf, "number1 = %d", &n1);
    sscanf(p+1, "number2 = %d", &n2);

    }

    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);

    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */



/*
#include "csapp.h"
int main(void) {
    char *buf, *p, *q, *method;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    if ((buf = getenv("QUERY_STRING")) != NULL) { // 환경 변수 QUERY_STRING에 URL 파라미터로 받은 값을 저장한다.
	p = strchr(buf, '&'); // 파라미터가 2개 이므로 처음 &가 나오고 나서부터 자른다.
    // Exampl url
    // request url : http://localhost:5000/cgi-bin/adder?number1=33&number2=22
	*p = '\0'; // 맨 앞에 널 문자를 추가하는 이유? -> strchr는 ch 문자가 나온 뒤의 문자열 자리부터 리턴하기 떄문에
    q = strchr(buf, '=');
    *q = '\0';
    strcpy(arg1, q + 1);
    q = strchr(p + 1, '=');
    *q = '\0';
    strcpy(arg2, q + 1);

	n1 = atoi(arg1); // atoi : 공백이거나 숫자가 아닌 수가 등장할 때까지 변환
	n2 = atoi(arg2); // atoi2 : 공백이거나 숫자가 아닌 수가 등장할 때까지 변환2
    }

    method = getenv("REQUEST_METHOD");
    if (strcasecmp(method, "HEAD")){
        sprintf(content, "%sWelcome to add.com: ", content);
        sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
        sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
            content, n1, n2, n1 + n2);
        sprintf(content, "%sThanks for visiting!\r\n", content);
    }
    // 응답시 알아야하는 헤더가 content-length, content-type이므로 
    // content-length와 content-type을 출력해본다.
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    exit(0);
}
*/