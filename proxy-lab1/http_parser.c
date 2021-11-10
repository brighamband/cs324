#include<stdio.h>
#include<string.h>

#define HTTP_REQUEST_MAX_SIZE 4096
#define HOSTNAME_MAX_SIZE 512
#define PORT_MAX_SIZE 6
#define URI_MAX_SIZE 4096
#define METHOD_SIZE 32

// Returns 1 if complete, 0 if incomplete
int is_complete_request(const char *request) {
	char *found_ptr = strstr(request, "\r\n\r\n");
	if (found_ptr != NULL) {
		return 1;
	}
	return 0;
}

char* parse_http_request(char *request) {
	char method[METHOD_SIZE];
	char hostname[HOSTNAME_MAX_SIZE];
	char port[PORT_MAX_SIZE];
	char uri[URI_MAX_SIZE];
	char* saveptr;

	// If client has not sent the full request, return 0 to show the request is not complete.
	if (is_complete_request(request) == 0) {
		return NULL;
	}

	// Make non-const request variable
	char temp_request[500];
	strcpy(temp_request, request);

	char* token = strtok_r(temp_request, "\r\n", &saveptr);
	
	while (token != NULL) {
		printf("TOKEN: %s\n", token);

		// If on first line
		if (strstr(token, "GET")) {
			// Method
			strcpy(method, "GET");

			// URI
			token = token + strlen(method) + sizeof(char);	// Move past method and space
			size_t len = strcspn(token, " ");
			strncpy(uri, token, len);
		}

		// Host
		if (strstr(token, "Host:")) {
			token += strlen("Host: ");	// Skip past Host and space
			strcpy(hostname, token);
		}

		token = strtok_r(NULL, "\r\n", &saveptr);
	}
	// // Grab method
	// char* temp_ptr = strtok(temp_request, " ");
	// strcpy(method, temp_ptr);

	// // Grab uri
	// temp_ptr = temp_request + strlen(temp_ptr) + 1;
	// strtok(temp_ptr, " ");
	// strcpy(uri, temp_ptr);

	// // Host
	// temp_ptr = temp_request + strlen(temp_ptr);
	// strtok(temp_ptr, " ");
	// strtok(temp_ptr, " ");
	// strcpy(hostname, temp_ptr);

	// Grab entire path
	// temp_ptr = temp_request + strlen(temp_ptr) + 1;  // Move past method
	// temp_ptr += 7;	// Move past http://
	// temp_ptr = strtok(temp_ptr, " ");

	// // Grab everything before slash (Hostname and Port)
	// char host_port_str[500];
	// strcpy(host_port_str, temp_ptr);
	// strtok(host_port_str, "/");

	// // Host and Port
	// char* port_str = strstr(host_port_str, ":");
	// char host_str[500];
	// strcpy(host_str, host_port_str);

	// // If port was specified
	// if (port_str != NULL) {
	// 	strtok(host_str, ":");
	// 	strcpy(hostname, host_str);
	// 	strcpy(port, port_str + 1);	// Copy port_str into port, skipping the : colon
	// } 
	// // If just hostname
	// else {	// If not colon, make port the default
	// 	strcpy(hostname, host_str);
	// 	strcpy(port, "80");	// Default port number
	// }

	// // Grab everything after slash (URI), if there is a specific uri
	// if (strstr(temp_ptr, "/")) {
	// 	char* uri_str = temp_ptr + strlen(host_port_str) + 1;  // Make uri be everything past the host, port and slash (the +1)
	// 	strcpy(uri, uri_str);
	// }

	printf("\nrequest: %s\n", request);
	printf("method: %s\n", method);
	printf("hostname: %s\n", hostname);
	printf("port: %s\n", port);
	printf("uri: %s\n", uri);

	return NULL;	// Return 1 string that's the request
}