#include "cproxy.h"



void exit_program(full_URL* my_url)
{
    free_full_URL(my_url);
    exit(EXIT_FAILURE);
}


full_URL* initialize_full_URL()
{
    full_URL* my_url = (full_URL*)malloc(sizeof(full_URL));
    if (my_url == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return NULL;
    }

    // Initialize members to default values
    my_url->host = NULL;
    my_url->path = NULL;
    my_url->port = 80; // Default port for HTTP
    my_url->is_legal_path = -1;

    return my_url;
}


int get_path(const char* str, full_URL* my_url)
{
    if (my_url == NULL || str == NULL)
    {
        fprintf(stderr, "Given input is not valid.\n");
        return -1;
    }


    const char* http_pos = strstr(str, "http://");
    if (http_pos == NULL)
    { // No 'http://' found, invalid URL
        fprintf(stderr, "URL format is invalid.\n");
        return -1;
    }

    http_pos += 7; // Skip 'http://'

    // Find the start of the path, which is after the host and optional port
    const char* path_start = strchr(http_pos, '/');
    if (path_start == NULL)
    {
        // URL ends after host (no port, no path)
        my_url->path = (char*)malloc(2); // Allocate memory for "/"
        if (my_url->path == NULL)
        {
            fprintf(stderr, "Malloc failed\n");
            return -1;
        }
        strcpy(my_url->path, "/");
        my_url->is_legal_path = !PATH_EXISTS;
        return 0;
    }

    // Allocate memory for path
    my_url->path = (char*)malloc(strlen(path_start) + 1);
    if (my_url->path == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return -1;
    }

    strcpy(my_url->path, path_start);

    // Check if path is just '/'
    if (strcmp(my_url->path, "/") == 0)
        my_url->is_legal_path = !PATH_EXISTS; // Indicate that the path is just '/'
    else
        my_url->is_legal_path = PATH_EXISTS; // Indicate that it's a full path

    return 0;
}


void free_full_URL(full_URL* my_url)
{
    if (my_url != NULL)
    {
        if (my_url->host != NULL)
            free(my_url->host);

        if (my_url->path != NULL)
            free(my_url->path);

        free(my_url);
    }
}


int get_host(const char* str, full_URL* my_url)
{

    if (str == NULL || my_url == NULL)
        return -1;


    const char* http_pos = strstr(str, "http://");
    if (http_pos == NULL)
        return -1; // No 'http://' found, invalid URL

    http_pos += 7; // Skip 'http://'


    // Find the position of the first '/' after 'http://'
    const char* slash_pos = strchr(http_pos, '/');
    // Check if there is a port in the URL
    const char* port_pos = strchr(http_pos, ':');

    const char* end_pos;

    if (port_pos != NULL && (slash_pos == NULL || port_pos < slash_pos)) // Port found and before any slash
        end_pos = port_pos;
    else  // Slash found or no port
        end_pos = (slash_pos != NULL) ? slash_pos : http_pos + strlen(http_pos);


    size_t host_length = end_pos - http_pos; // Calculate length of host part
    my_url->host = (char*)malloc(host_length + 1); // Allocate memory for host
    if (my_url->host == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return -1;
    }

    strncpy(my_url->host, http_pos, host_length);
    my_url->host[host_length] = '\0';

    return 0;
}


int get_port(const char* str, full_URL* my_url)
{

    if (my_url == NULL || str == NULL)
        return -1;


    const char* http_pos = strstr(str, "http://");
    if (http_pos == NULL)
        return -1; // No 'http://' found, invalid URL

    http_pos += 7; // Skip 'http://'

    // Find the position of the colon (if any), after skipping 'http://'
    const char* colon_pos = strchr(http_pos, ':');
    if (colon_pos == NULL)
    {
        my_url->port = 80; // Default port if no colon found
        return 0; // Success
    }

    char* end_ptr;
    // Convert the string portion representing the port number to a long integer
    long port_number = strtol(colon_pos + 1, &end_ptr, 10); // base 10 for decimal, skip ':'

    // Check for errors
    if ((colon_pos + 1) == end_ptr)
    {
        my_url->port = 80; // No number found, default to port 80
        return 0; // Success
    }

    if (errno == ERANGE && (port_number == LONG_MAX || port_number == LONG_MIN))
    {
        perror("The number is out of range for a long integer.");
        return -1;
    }

    if (port_number < 0 || port_number > 65535)
    {
        printf("Invalid port number. Port number should be between 0 and 65535.\n");
        return -1;
    }

    // If everything is fine, set the port number in my_url
    my_url->port = (int)port_number;
    return 0;
}



int set_connection(full_URL* my_url)
{
    int sd; // socket descriptor
    struct hostent* server_info; // Structure to store information about the given host
    struct sockaddr_in socket_info; // Structure to store the socket information

    // Create socket with IPv4 and TCP
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket\n"); // Print error if socket creation fails
        close(sd); // Close the socket
        return -1; // Return -1 to indicate failure
    }

    // Initialize socket_info memory with zeros
    memset(&socket_info, 0, sizeof(struct sockaddr_in));

    socket_info.sin_family = AF_INET; // Set family to IPv4
    server_info = gethostbyname(my_url->host); // Get server IP address using the host name
    if (server_info == NULL) // Check if gethostbyname succeeded
    {
        herror("gethostbyname\n"); // Print error if host name resolution fails
        close(sd); // Close the socket
        return -1; // Return -1 to indicate failure
    }

    // Set the socket's IP address
    socket_info.sin_addr.s_addr = ((struct in_addr*) server_info->h_addr)->s_addr;
    // Convert port number from host byte order to network byte order and set the socket's port
    socket_info.sin_port = htons(my_url->port);

    // Attempt to connect to the server
    if(connect(sd, (struct sockaddr*) &socket_info, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect\n"); // Print error if connection attempt fails
        close(sd); // Close the socket
        return -1; // Return -1 to indicate failure
    }

    return sd; // Return the socket descriptor for the successful connection
}


size_t write_to_connection(int sd, full_URL* my_url)
{
    size_t total_written_bytes = 0; // Counter for total bytes successfully written

    // Buffer to hold the constructed HTTP request
    char request[50 + strlen(my_url->path) + strlen(my_url->host)];

    // Construct the HTTP GET request with the host and path
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", my_url->path, my_url->host);

    size_t data_length = strlen(request); // Length of the request to be sent

    // Debug print to show the constructed HTTP request and its length
    printf("HTTP request =\n%s\nLEN = %zu\n", request, strlen(request));

    do {
        // Attempt to write the request to the socket
        ssize_t wrote_bytes = write(sd, request + total_written_bytes, data_length - total_written_bytes);

        if (wrote_bytes < 0)
        {
            perror("write\n"); // Print error if write fails
            close(sd); // Close the socket
            exit(EXIT_FAILURE); // Exit with failure
        }

        // Increment the count of total bytes written
        total_written_bytes += wrote_bytes;

    } while (total_written_bytes < data_length); // Continue until the entire request is sent

    return total_written_bytes; // Return the count of total bytes written
}


int starts_with_http(const char* input)
{
    return strncmp(input, "http://", strlen("http://")) == 0;
}


char* get_file_full_path(full_URL* my_url, const char* directories_path)
{
    // Find the last occurrence of '/' in the path
    char* last_slash_ptr = strrchr(my_url->path, '/');
    char* path_to_use = my_url->path;

    // If a '/' is found, and it's not the first character, adjust the path_to_use to start from the last '/'
    if (last_slash_ptr != NULL && last_slash_ptr != my_url->path)
        path_to_use = last_slash_ptr;

    // Calculate the required size for full_path
    unsigned long file_path_size = strlen(path_to_use) + 1 + strlen(directories_path); // +1 for null terminator
    if (my_url->is_legal_path == !PATH_EXISTS)
        file_path_size += strlen(DEFAULT_PATH); // add length of DEFAULT_PATH if needed

    char* full_path = (char*)malloc(file_path_size);
    if (full_path == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return NULL;
    }

    strcpy(full_path, directories_path);
    strcat(full_path, path_to_use); // Concatenate the part from the last slash

    if (my_url->is_legal_path == !PATH_EXISTS)
        strcat(full_path, DEFAULT_PATH); // Append DEFAULT_PATH if path is "/"

    return full_path;
}


FILE* get_file(char* full_path)
{
    // Open the file for writing
    FILE* file = fopen(full_path, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file %s for writing.\n", full_path);
        return NULL;
    }

    return file;
}


char* get_directories_path(full_URL* my_url)
{
    unsigned long full_path_size = strlen(my_url->host) + strlen(my_url->path) + strlen(DEFAULT_PATH) + 1;
    // Allocate memory for the full path string
    char* full_path = (char*) malloc(full_path_size);

    // Check if memory allocation is successful
    if (full_path == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return NULL;
    }

    // Construct the full path by concatenating host, and path
    strcpy(full_path, my_url->host);

    strcat(full_path, my_url->path);

    // Allocate memory for directories_path
    char* directories_path = (char*) malloc(strlen(full_path) + 1);
    // Check if memory allocation is successful
    if (directories_path == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        free(full_path); // Free the previously allocated full_path
        return NULL; // Return NULL to indicate failure
    }

    // Copy the full path into directories_path
    strcpy(directories_path, full_path);
    // Find the last occurrence of '/' to isolate the directories part
    char* last_slash = strrchr(directories_path, '/');
    // If a slash is found, null-terminate the string at that position to exclude the file name
    if (last_slash != NULL)
        last_slash[0] = '\0';

    // Free the memory allocated for full_path as it's no longer needed
    free(full_path);
    // Return the directories path
    return directories_path;
}


int read_from_connection(int sd, full_URL* my_url)
{
    char buffer[READ_BUFFER_SIZE]; // Buffer to store the data read from the connection
    size_t total_read_bytes = 0; // Counter for total bytes read
    int header_end_found = 0; // Flag to indicate the end of the header part
    char* full_file_path = NULL; // String to hold the full file path
    char* directories_path = NULL; // String to hold the directories path
    int save_file_flag = 1; // Flag to indicate if the file should be saved
    ssize_t read_bytes; // Variable to store the count of bytes read in each read operation
    FILE* file = NULL; // File pointer for the file to be written

    while (1)
    {
        // Read data from the connection
        read_bytes = read(sd, buffer, sizeof(buffer) - 1);

        // Check for read error
        if (read_bytes < 0)
        {
            if (read_bytes < 0)
                perror("Failed to read from file descriptor\n"); // Print error message

            if (file != NULL)
                fclose(file); // Close the file if it's open

            // Free allocated memory if save_file_flag is set
            if (save_file_flag)
            {
                free(directories_path);
                free(full_file_path);
            }
            return -1;
        }

        // Finished reading
        if (read_bytes == 0)
            break;


        buffer[read_bytes] = '\0'; // Null-terminate the buffer
        total_read_bytes += read_bytes; // Update the total bytes read
        printf("%s", buffer);

        // Check if the end of the header has been found
        if (!header_end_found)
        {
            char *header_end = strstr(buffer, "\r\n\r\n"); // Find the end of the header

            if (header_end != NULL) // If the end of the header is found
            {
                header_end_found = 1; // Set the flag
                size_t header_length = header_end - buffer + 4; // Calculate the header length

                // Check if the response is OK
                if (strstr(buffer, "200 OK\r\n") == NULL)
                    save_file_flag = 0; // If not OK, set save_file_flag to 0
                else // If OK
                {
                    directories_path = get_directories_path(my_url);
                    if (directories_path == NULL)
                    {
                        free(directories_path);
                        return -1;
                    }


                    if (create_directories(directories_path) == -1)
                    {
                        free(directories_path);
                        return -1;
                    }

                    full_file_path = get_file_full_path(my_url, directories_path);
                    if (full_file_path == NULL)
                    {
                        free(directories_path);
                        return -1;
                    }

                    file = get_file(full_file_path);
                    if (file == NULL)
                    {
                        free(directories_path);
                        free(full_file_path);
                        return -1;
                    }
                }

                // Write any part of the body that's in the buffer to the file
                if (header_length < (size_t)read_bytes && save_file_flag)
                    fwrite(header_end + 4, 1, read_bytes - header_length, file);
            }
        }
        else // If the header end has been found
        {
            // Write to the file if save_file_flag is set
            if (save_file_flag)
                fwrite(buffer, 1, read_bytes, file);
        }
    }

    // Close the file if it's open
    if (save_file_flag && file != NULL)
        fclose(file);

    // Free allocated memory if save_file_flag is set
    if (save_file_flag)
    {
        free(directories_path);
        free(full_file_path);
    }

    // Print the total bytes read
    printf("\n Total response bytes: %ld\n", total_read_bytes);

    return save_file_flag;
}



int create_directories(char* path)
{
    char* path_copy = strdup(path);
    if (path_copy == NULL)
    {
        perror("Error duplicating path\n");
        return -1;
    }

    char* token = strtok(path_copy, "/");
    char current_directory[256] = ".";  // Start from the current directory

    // Process all but the last component
    while (token != NULL)
    {
        strcat(current_directory, "/");
        strcat(current_directory, token);

        if (mkdir(current_directory, 0777) != 0 && errno != EEXIST) {
            perror("Error creating directory\n");
            free(path_copy);
            return -1;
        }

        token = strtok(NULL, "/");
    }

    free(path_copy);
    return 1;
}



int open_file(full_URL* my_url)
{
    unsigned long full_path_size = strlen(my_url->path) + strlen(my_url->host) + strlen(DEFAULT_PATH) + 1;
    char* full_path = (char*) malloc(full_path_size);
    if (full_path == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return -1;
    }

    strcpy(full_path, my_url->host);
    strcat(full_path, my_url->path);

    if (my_url->is_legal_path == !PATH_EXISTS)
        strcat(full_path, DEFAULT_PATH);

    // Check if the file exists
    if (access(full_path, F_OK) != -1)
    {
        // File exists, open it
        FILE* file = fopen(full_path, "rb");

        if (file != NULL)
        {
            size_t read_size;
            unsigned char buffer[READ_BUFFER_SIZE];
            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            printf("File is given from local filesystem\n");

            // Prepare the header format and the initial part of the header
            const char* header_format = "HTTP/1.0 200 OK\r\nContent-Length: %ld\r\n\r\n";

            char header[256];
            int header_len = snprintf(header, sizeof(header), header_format, file_size);

            // Print the header with the correct Content-Length
            size_t header_written_size = fwrite(header, 1, header_len, stdout);

            size_t total_written_size = header_written_size; // Track total written bytes

            // Read the file and print to the screen in chunks
            while ((read_size = fread(buffer, 1, sizeof(buffer), file)) > 0)
            {
                size_t chunk_written_size = fwrite(buffer, 1, read_size, stdout);
                total_written_size += chunk_written_size; // Add the size of the written chunk
            }

            // Close the file when done
            fclose(file);
            free(full_path);

            printf("\n Total response bytes: %ld\n", total_written_size); // Print the total written bytes
            return 1;
        }
        else
            printf("fopen failed\n");

    }

    free(full_path);
    return -1;
}


int is_legal_URL(char* url, char* flag)
{
    // Check if flag is legal
    if (strcmp(flag, "") != 0 && strcmp(flag, "-s") != 0)
        return 0;

    if (starts_with_http(url) == 0)
        return 0;

    return 1;
}


int main(int argc, char* argv[])
{

    // Verify the correct number of command-line arguments
    if (argc < 2 || argc > 3)
    {
        printf("Usage: cproxy <URL> [-s]");
        exit(EXIT_FAILURE);
    }

    char* input_string = argv[1]; // Assign the first argument as the input URL
    char* flag = ""; // Initialize a default flag
    int is_saved = 1;

    // If there are three arguments, use the third one as the flag
    if (argc == 3)
        flag = argv[2];

    // Validate the URL and flag, if not legal, print usage and exit
    if (!is_legal_URL(input_string, flag))
    {
        printf("Usage: cproxy <URL> [-s]\n");
        exit(EXIT_FAILURE);
    }

    full_URL* my_url = initialize_full_URL();
    if (my_url == NULL)
        exit(EXIT_FAILURE);

    if (get_host(input_string, my_url) == -1)
        exit_program(my_url);

    if (get_port(input_string, my_url) == -1)
        exit_program(my_url);

    if (get_path(input_string, my_url) == -1)
        exit_program(my_url);


    // Check if the file is accessible, if not, establish a connection to the host
    if (open_file(my_url) == -1)
    {
        int sd = set_connection(my_url); // Set up the connection to the specified host and port
        // Check for a successful connection
        if (sd == -1)
            exit_program(my_url);

        // Send the request to the connection
        write_to_connection(sd, my_url);
        // Read the response from the connection
        is_saved = read_from_connection(sd, my_url);
        // Close the socket descriptor
        close(sd);
    }


    // If the "-s" flag is set, use the system call to open the URL in the default web browser
    if (strcmp(flag, "-s") == 0 && is_saved == 1)
    {
        // Allocate memory for the command string to be used with system call
        char* copy_str = (char*) malloc( strlen(my_url->host) + strlen(my_url->path) + 50);
        if (copy_str == NULL)
        {
            fprintf(stderr, "Malloc failed\n");
            exit_program(my_url);
        }

        strcpy(copy_str, "xdg-open ");
        strcat(copy_str, "http://");
        strcat(copy_str, my_url->host);
        strcat(copy_str, my_url->path);
        system(copy_str);
        free(copy_str);
    }

    free_full_URL(my_url);

    return 0;
}