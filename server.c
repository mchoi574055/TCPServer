/* PLEASE include these headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#define MYPORT 18000 /* Avoid reserved ports */
#define BACKLOG 10 /* pending connections queue size */
#define ARRSIZE 4096
#define MAXBYTES 5000064 // To support very large files
// Buffers:
char write_buffer[MAXBYTES];
char buffer[ARRSIZE];
char request[ARRSIZE];
char content_type[ARRSIZE];
 
// Supported file types:
enum file_type {binary, html, txt, jpg, png, gif};
// Functions:
void writeRequest(char request[], size_t request_size, int file_descriptor);
void checkFile(char request[], char content_type[]);
int main (int argc, char** argv)
{
int listenfd, connfd, n;
struct sockaddr_in servaddr;
// Helper
char endMsg[9];
if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  perror("Socket error");
bzero(&servaddr, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
servaddr.sin_port = htons(MYPORT);
// servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 if ((bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) < 0)
  perror("Bind error");
if ((listen(listenfd, BACKLOG) < 0))
    printf("Listen error");
int track = 0;
while (1) {
  struct sockaddr_in addr;
  socklen_t addr_len;
   printf("waiting for a connection on port %d\n", MYPORT);
  fflush(stdout);
  connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);
 
  // Ensure buffer is empty
  memset(buffer, 0, ARRSIZE);
  memset(endMsg, 0, 9);
  memset(request, 0, ARRSIZE);
  memset(write_buffer, 0, MAXBYTES);
  // Read HTTP request from localhost
  n = read (connfd, buffer, ARRSIZE);
  printf("%s\n", buffer);
  if (n < 0)
    printf("Read error");
 // Fetch GET request from buffer
printf("********************************** FETCH GET REQUEST **********************************\n");
int i = 0;
for(i; i < strlen(buffer) - 9; i++)
{
  if (buffer[i] == ' ')
  {
    strncpy(endMsg, buffer + i, 9);
    if (strcmp(endMsg, " HTTP/1.1") == 0)
      break;
  }
}
if (strcmp(endMsg, " HTTP/1.1") != 0)
   perror("failed to detect HTTP \n");
 
printf("End message:%s\n", endMsg);
strncpy(request, buffer + 5, i - 5);
memset(buffer, 0, ARRSIZE);


// printf("%s\n", request);
printf("********************************** END FETCH **********************************\n");
// End fetch
// Write back file
//  printf("%s\n", request);
// printf("Request length: %d\n", (int) strlen(request));
memset(buffer, 0, ARRSIZE);
memset(endMsg, 0, 9);
// memset(request, 0, ARRSIZE);



writeRequest(request, strlen(request), connfd);
// End write back
track ++;
printf("Requests handled: %d\n", track);

memset(endMsg, 0, 9);
memset(request, 0, ARRSIZE);
/*
  snprintf((char*) towrite_buf, sizeof(towrite_buf), "HTTP/1.0 200 OK \r\n\r\nHellos");
   write(connfd, (char*) towrite_buf, strlen((char*) towrite_buf));
*/
/*
char temp[] = "HTTP/1.0 200 OK \r\n\r\nHello";
write(connfd, temp, (int) strlen(temp));
  memset(temp, 0, 26);
printf("%d\n", connfd);
*/
  close(connfd);
  printf("********************************** END **********************************\n");
}
}
void writeRequest(char request[], size_t request_length, int file_descriptor)
{
printf("********************************** START WRITE **********************************\n");
printf("File name: %s\n", request);
printf("Request length: %d\n", (int) strlen(request));
printf("File descriptor: %d\n",file_descriptor);
 
// Deal with file:
if (strlen(request) == 0)
{
    printf("Please add input\n");
    char NoFile[] = "HTTP/1.0 200 OK\r\n\r\n404 Page Not Found";
   write(file_descriptor, NoFile, strlen(NoFile));
   return;
}

checkFile(request, content_type);
// printf("Content: type: %s\n", content_type);
printf("File name: %s\n", request);
 
FILE* file_pointer = fopen(request, "rb");
// printf("Content: type: %s\n", content_type);

if (file_pointer == NULL) {
   perror("File not found\n");
   char NoFile[] = "HTTP/1.0 200 OK\r\n\r\n404 Page Not Found";
   write(file_descriptor, NoFile, strlen(NoFile));
}
 
else {
   printf("Content1: type: %s\n", content_type);
   fseek(file_pointer, 0L, SEEK_END);
   long file_size = ftell(file_pointer);
   rewind(file_pointer);
   memset(write_buffer, 0, MAXBYTES);
  // printf("Content 2: type: %s\n", content_type);
   fread(write_buffer, file_size, 1, file_pointer);
 
  
// memset(request, 0, ARRSIZE);
 
   // Write reponse header:
   char write2[3*ARRSIZE] = "HTTP/1.0 200 OK\r\n";
 
   // Date code referenced from stack overflow:
   time_t t = time(NULL);
   struct tm *tm = localtime(&t);
   char date[ARRSIZE];
   strftime(date, sizeof(date), "%c", tm);
   char date1[ARRSIZE] = "Date: ";
   strcat(date1, date);
   strcat(date1, "\r\n");
   strcat(write2, date1);
   memset(date, 0, ARRSIZE);
   memset(date1, 0, ARRSIZE);
 
   // Server header
   char server[] = "Server: Michael and Sraavya's web server\r\n";
   strcat(write2, server);
 
   // Content-length header
   char content_length[] = "Content-Length: ";
   char store_size[64];
   sprintf(store_size, "%d", file_size);
   strcat(content_length, store_size);
   strcat(write2, content_length);
   strcat(write2, "\r\n");
 
   // Connection header
   char connection[] = "Connection: keep-alive\r\n";
   strcat(write2, connection);
 
   // Content-Type header
   // declared globally
   strcat(write2, "Content-Type: ");
   strcat(write2, content_type);
   strcat(write2, "\r\n");
  
   // Add final /r/n
   strcat(write2, "\r\n");
 
   printf("********************************** RESPONSE SENT **********************************\n");
   printf("%s\n", write2);
 
   write(file_descriptor, write2, (int) (strlen(write2)));
// memset(temp2, 0, 26);
   write(file_descriptor, write_buffer, file_size);
 //char tmp[] = "HTTP/1.0 200 OK\r\n\r\n";
 //write(file_descriptor, tmp, sizeof(tmp));
 
   memset(write2, 0, 3*ARRSIZE);
   memset(write_buffer, 0, MAXBYTES);
   fclose(file_pointer);
   printf("********************************** END WRITE **********************************\n");
   }
   memset(write_buffer, 0, MAXBYTES);
}
 
void checkFile(char request[], char content_type[])
{
   // Check for spaces
   for (int i = 0; i < strlen(request) - 3; i++)
   {
       if ((request[i] == '%') && (request[i+1] == '2')&& (request[i+2] == '0'))
       {
           request[i] = ' ';
           printf("First string: %s,  Length: %d\n", request+i+1, strlen(request));
            printf("Rest string: %s,  Length: %d\n", request+i+3, strlen(request));
           memcpy(request + i + 1, request + i + 3, strlen(request+ i + 3)+1);
    
           // i = 0; // simply reset, inefficient but gets the job done
       }
       printf("Iteration %d: %s\n", i, request);
       
   }
   // printf("String result: %s\n", request);
 
   enum file_type request_type = binary;
   char store_file[ARRSIZE];
   // Check file type
   for (int i = 0; i < strlen(request); i++)
   {
       if (request[i] == '.')
       {
           strcpy(store_file, request + i + 1);
           break;
       }
   }
   // printf("File type: %s\n", store_file);
 
   if (strcmp(store_file, "html") == 0)
       request_type = html;
   else if (strcmp(store_file, "txt") == 0)
       request_type = txt;
   else if ((strcmp(store_file, "jpg") == 0) || (strcmp(store_file, "jpeg") == 0))
       request_type = jpg;
   else if (strcmp(store_file, "png") == 0)
       request_type = png;
   else if (strcmp(store_file, "gif") == 0)
       request_type = gif;
 
   char temp_content[ARRSIZE] = "";
   switch (request_type)
   {
      
       case html:
           strcat(temp_content,"text/html");
           break;
       case txt:
           strcat(temp_content, "text/plain");
           break;
       case jpg:
           strcat(temp_content, "image/jpeg");
           break;
       case png:
           strcat(temp_content, "image/png");
           break;
       case gif:
           strcat(temp_content, "image/gif");
           break;
       default:
           strcat(temp_content, "application/octet-stream");
           break;
   }
   memset(content_type, 0, ARRSIZE);
   strcpy(content_type, temp_content);
   // printf("%s\n", content_type);
   memset(temp_content, 0, ARRSIZE);
   memset(store_file, 0, ARRSIZE);
 
 
 
}

