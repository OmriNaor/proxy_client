#ifndef CPROXY_CPROXY_H
#define CPROXY_CPROXY_H


// Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <limits.h>


#define READ_BUFFER_SIZE 4096 // Define the size of the read buffer
#define DEFAULT_PATH "index.html"
#define PATH_EXISTS 1

typedef struct{
    char* host; // URL host
    char* path; // URL path
    int is_legal_path; // 1 if path exists, else 0
    int port; // URL port
} full_URL;

// Function prototypes


/**
 * Creates directories recursively based on the given path.
 *
 * @param path: The path for which directories need to be created.
 * @return 1 on success, or -1 on failure.
 */
int create_directories(char*);

/**
 * Extracts the host part from a given URL and stores it in the provided 'my_url' structure.
 *
 * @param str: A pointer to a string containing the URL.
 * @param my_url: A pointer to a 'full_URL' structure where the host part will be stored.
 * @return
 *   - 0 if the host part is successfully extracted and stored in 'my_url'.
 *   - -1 if there are invalid inputs (e.g., NULL pointers) or if 'str' does not contain 'http://'.
 */
int get_host(const char*, full_URL*);

/**
 * Determines the port number from a given URL and stores it in the provided 'my_url' structure.
 *
 * @param str: A pointer to a string containing the URL.
 * @param my_url: A pointer to a 'full_URL' structure where the port number will be stored.
 * @return
 *   - 0 if the port number is successfully extracted and stored in 'my_url'.
 *   - -1 if there are invalid inputs (e.g., NULL pointers) or if 'str' does not contain 'http://'.
 *   - -1 if the port number is out of range (not between 0 and 65535) or if it cannot be extracted.
 */
int get_port(const char*, full_URL*);

/**
 * Extracts and sets the path part from a given URL in a 'full_URL' structure.
 *
 * @param str: A string containing the URL.
 * @param my_url: A pointer to a 'full_URL' structure to store the extracted path.
 * @return
 *   - 0 if the path is successfully extracted and set.
 *   - -1 if memory allocation fails or if the URL is invalid.
 */
int get_path(const char*, full_URL*);


/**
 * Establishes a TCP connection to the specified host and port.
 *
 * @param my_url: A pointer to a 'full_URL' structure containing the host name and port to connect to.
 * @return
 *   - The socket descriptor for the established connection if successful.
 *   - -1 if the connection fails. In case of failure, the function also prints an error message.
 */
int set_connection(full_URL*);


/**
 * Establishes a TCP connection to the specified host and port.
 *
 * @param my_url: A pointer to a 'full_URL' structure containing the host name and port to connect to.
 * @return
 *   - The socket descriptor for the established connection if successful.
 *   - -1 if the connection fails. In case of failure, the function also prints an error message.
 */
size_t write_to_connection(int, full_URL*);

/**
 * Reads data from the established connection, handles the HTTP response, and writes the data to the appropriate file.
 *
 * @param sd: The socket descriptor of the established connection.
 * @param my_url: A pointer to a 'full_URL' structure containing the host name, path, and port.
 * @return 1 if a file was saved, else return 0.
 *
 * @note
 *   - This function handles the HTTP response, including header parsing and content saving.
 *   - It's the caller's responsibility to close the socket and manage file resources when they are no longer needed.
 */
int read_from_connection(int, full_URL*);

/**
 * Checks if the given input URL starts with 'http://'.
 *
 * @param input: A string containing the URL.
 * @return 1 if the URL starts with 'http://', 0 otherwise.
 */
int starts_with_http(const char*);

/**
 * Constructs the full file path from a given URL path and directories path.
 * The function identifies the last segment of the URL path and appends it to the directories path.
 * If the URL path is not considered legal (is_legal_path is 0), a default path is also appended.
 *
 * @param my_url: A pointer to a structure representing the URL, including the URL path, and flags indicating the legality of the path.
 * @param directories_path: A string representing the base directory path to which the URL path will be appended.
 * @return A pointer to a dynamically allocated string containing the full file path, or NULL if memory allocation fails.
 */
char* get_file_full_path(full_URL*, const char*);

/**
 * Opens a file for writing.
 *
 * @param full_path: The full path to the file to be opened.
 * @return A file pointer to the opened file, or NULL if the file could not be opened.
 */
FILE* get_file(char*);

/**
 * Constructs the full path for the directories based on the given path and host.
 *
 * @param my_url: A pointer to a 'full_URL' structure containing the path and host parts of the URL.
 * @return A pointer to a string containing the full path for the directories, or NULL if memory allocation fails.
 */
char* get_directories_path(full_URL*);

/**
 * Opens a file if it exists in the local file system and sends its contents as an HTTP response.
 *
 * @param my_url: A pointer to a 'full_URL' structure containing the host name, path, and port.
 * @return
 *   - 1 if the file exists and its contents were successfully sent as an HTTP response.
 *   - -1 if the file doesn't exist or if there was an error in the process.
 *
 * @note
 *   - This function is responsible for checking the existence of a file, opening it, and sending its contents as an HTTP response.
 *   - It prepares an HTTP header with the correct Content-Length before sending the file contents.
 */
int open_file(full_URL*);

/**
 * Checks whether the given url and flag are legal
 *
 * @param url: A string containing the URL.
 * @param flag: A string containing the flag.
 * @return 1 if the given url and flag are legal, else return 0.
 */
int is_legal_URL(char*, char*);

/**
 * Frees the memory allocated for a 'full_URL' structure, including its host and path.
 *
 * @param my_url: A pointer to a 'full_URL' structure to be freed. It can be NULL.
 */
void free_full_URL(full_URL*);

/**
 * Frees the memory allocated for a 'full_URL' structure and exits the program with a failure status.
 *
 * @param my_url: A pointer to a 'full_URL' structure to be freed.
 */
void exit_program(full_URL*);


/**
 * Initializes a 'full_URL' structure and allocates memory for it.
 *
 * @return A pointer to the newly initialized 'full_URL' structure, or NULL if memory allocation fails.
 */
full_URL* initialize_full_URL();



#endif //CPROXY_CPROXY_H
