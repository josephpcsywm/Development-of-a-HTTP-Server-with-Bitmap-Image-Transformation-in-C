#include "request.h"
#include "response.h"
#include <string.h>


/******************************************************************************
 * ClientState-processing functions
 *****************************************************************************/
ClientState *init_clients(int n) {
    ClientState *clients = malloc(sizeof(ClientState) * n);
    for (int i = 0; i < n; i++) {
        clients[i].sock = -1;  // -1 here indicates available entry
    }
    return clients;
}

/* 
 * Remove the client from the client array, free any memory allocated for
 * fields of the ClientState struct, and close the socket.
 */
void remove_client(ClientState *cs) {
    if (cs->reqData != NULL) {
        free(cs->reqData->method);
        free(cs->reqData->path);
        for (int i = 0; i < MAX_QUERY_PARAMS && cs->reqData->params[i].name != NULL; i++) {
            free(cs->reqData->params[i].name);
            free(cs->reqData->params[i].value);
        }
        free(cs->reqData);
        cs->reqData = NULL;
    }
    close(cs->sock);
    cs->sock = -1;
    cs->num_bytes = 0;
}


/*
 * Search the first inbuf characters of buf for a network newline ("\r\n").
 * Return the index *immediately after* the location of the '\n'
 * if the network newline is found, or -1 otherwise.
 * Definitely do not use strchr or any other string function in here. (Why not?)
 */
int find_network_newline(const char *buf, int inbuf) {
    
    //IMPLEMENT THIS
    for (int i = 0; i < inbuf - 1; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return i + 2;
        }
    }
    return -1;
}

/*
 * Removes one line (terminated by \r\n) from the client's buffer.
 * Update client->num_bytes accordingly.
 *
 * For example, if `client->buf` contains the string "hello\r\ngoodbye\r\nblah",
 * after calling remove_line on it, buf should contain "goodbye\r\nblah"
 * Remember that the client buffer is *not* null-terminated automatically.
 */
void remove_buffered_line(ClientState *client) {
    
    //IMPLEMENT THIS
    int newline = find_network_newline(client->buf, client->num_bytes);
    if (newline > 0) {
        memmove(client->buf, client->buf + newline, client->num_bytes - newline);
        client->num_bytes -= newline;
    }
}


/*
 * Read some data into the client buffer. Append new data to data already
 * in the buffer.  Update client->num_bytes accordingly.
 * Return the number of bytes read in, or -1 if the read failed.

 * Be very careful with memory here: there might be existing data in the buffer
 * that you don't want to overwrite, and you also don't want to go past
 * the end of the buffer, and you should ensure the string is null-terminated.
 */
int read_from_client(ClientState *client) {
    
    //IMPLEMENT THIS
    if (client->num_bytes >= MAXLINE - 1) {
        return 0;
    }

    int read_bytes = read(client->sock, client->buf + client->num_bytes, MAXLINE - 1 - client->num_bytes);
    if (read_bytes > 0) {
        client->num_bytes += read_bytes;
        client->buf[client->num_bytes] = '\0';
        return read_bytes;
    } else if (read_bytes == 0) {
        return 0;
    } else {
        perror("read error in read_from_client");
        return -1;
    }

}

    

/*****************************************************************************
 * Parsing the start line of an HTTP request.
 ****************************************************************************/
// Helper function declarations.
void parse_query(ReqData *req, const char *str);
void update_fdata(Fdata *f, const char *str);
void fdata_free(Fdata *f);
void log_request(const ReqData *req);


/* If there is a full line (terminated by a network newline (CRLF)) 
 * then use this line to initialize client->reqData
 * Return 0 if a full line has not been read, 1 otherwise.
 */
int parse_req_start_line(ClientState *client) {

    //IMPLEMENT THIS
    int newline = find_network_newline(client->buf, client->num_bytes);
    if (newline < 0) {
        return 0;
    }

    char line[newline + 1];
    strncpy(line, client->buf, newline);
    line[newline] = '\0';

    char *method = strtok(line, " ");
    char *path = strtok(NULL, " ");

    if (method && path) {
        ReqData *reqData = (ReqData *)malloc(sizeof(ReqData));
        if (reqData) {
            reqData->method = NULL;
            reqData->path = NULL;
            for (int i = 0; i < MAX_QUERY_PARAMS; i++) {
                reqData->params[i].name = NULL;
                reqData->params[i].value = NULL;
            }

            client->reqData = reqData;
        }

        if (client->reqData) {
            client->reqData->method = strdup(method);

            char *query = strchr(path, '?');
            if (query) {
                *query = '\0';
                query++;
                parse_query(client->reqData, query);
            }

            client->reqData->path = strdup(path);

            if (!client->reqData->method || !client->reqData->path) {
                if (client->reqData->method) free(client->reqData->method);
                if (client->reqData->path) free(client->reqData->path);
                free(client->reqData);
                client->reqData = NULL;
                return 0;
            }

        }

    } else {
        return 0;
    }

    remove_buffered_line(client);

    // This part is just for debugging purposes.
    log_request(client->reqData);
    return 1;
}


/*
 * Initializes req->params from the key-value pairs contained in the given 
 * string.
 * Assumes that the string is the part after the '?' in the HTTP request target,
 * e.g., name1=value1&name2=value2.
 */
void parse_query(ReqData *req, const char *str) {
    
    //IMPLEMENT THIS
    char *token = strtok((char *)str, "&");
    int index = 0;
    while (token != NULL && index < MAX_QUERY_PARAMS) {
        char *key = token;
        char *value = strchr(token, '=');
        if (value) {
            *value = '\0';
            value++;
            req->params[index].name = strdup(key);
            req->params[index].value = strdup(value);

            if (!req->params[index].name || !req->params[index].value) {
                if (req->params[index].name) free(req->params[index].name);
                if (req->params[index].value) free(req->params[index].value);
                return;
            }
        } else {
            req->params[index].name = NULL;
            req->params[index].value = NULL;
            
            fprintf(stderr, "Invalid key-value pair format: %s\n", key);
        }
        index++;
        token = strtok(NULL, "&");
    }
}


/*
 * Print information stored in the given request data to stderr.
 */
void log_request(const ReqData *req) {

    fprintf(stderr, "Request parsed: [%s] [%s]\n", req->method, req->path);
    for (int i = 0; i < MAX_QUERY_PARAMS && req->params[i].name != NULL; i++) {
        fprintf(stderr, "  %s -> %s\n", 
                req->params[i].name, req->params[i].value);
    }
}


/******************************************************************************
 * Parsing multipart form data (image-upload)
 *****************************************************************************/

char *get_boundary(ClientState *client) {
    int len_header = strlen(POST_BOUNDARY_HEADER);

    while (1) {
        int where = find_network_newline(client->buf, client->num_bytes);
        if (where > 0) {
            if (where < len_header || strncmp(POST_BOUNDARY_HEADER, client->buf, len_header) != 0) {
                remove_buffered_line(client);
            } else {
                // We've found the boundary string!
                // We are going to add "--" to the beginning to make it easier
                // to match the boundary line later
                char *boundary = malloc(where - len_header + 1);
                strncpy(boundary, "--", where - len_header + 1);
                strncat(boundary, client->buf + len_header, where - len_header - 1);
                boundary[where - len_header] = '\0';
                return boundary;
            }
        } else {
            // Need to read more bytes
            if (read_from_client(client) <= 0) {
                // Couldn't read; this is a bad request, so give up.
                return NULL;
            }
        }
    }
    return NULL;
}


char *get_bitmap_filename(ClientState *client, const char *boundary) {
    int len_boundary = strlen(boundary);

    // Read until finding the boundary string.
    while (1) {
        int where = find_network_newline(client->buf, client->num_bytes);
        if (where > 0) {
            if (where < len_boundary + 2 ||
                    strncmp(boundary, client->buf, len_boundary) != 0) {
                remove_buffered_line(client);
            } else {
                // We've found the line with the boundary!
                remove_buffered_line(client);
                break;
            }
        } else {
            // Need to read more bytes
            if (read_from_client(client) <= 0) {
                // Couldn't read; this is a bad request, so give up.
                return NULL;
            }
        }
    }

    int where = find_network_newline(client->buf, client->num_bytes);

    client->buf[where-1] = '\0';  // Used for strrchr to work on just the single line.
    char *raw_filename = strrchr(client->buf, '=') + 2;
    int len_filename = client->buf + where - 3 - raw_filename;
    char *filename = malloc(len_filename + 1);
    strncpy(filename, raw_filename, len_filename);
    filename[len_filename] = '\0';

    // Restore client->buf
    client->buf[where - 1] = '\n';
    remove_buffered_line(client);
    return filename;
}

/*
 * Read the file data from the socket and write it to the file descriptor
 * file_fd.
 * You know when you have reached the end of the file in one of two ways:
 *    - search for the boundary string in each chunk of data read 
 * (Remember the "\r\n" that comes before the boundary string, and the 
 * "--\r\n" that comes after.)
 *    - extract the file size from the bitmap data, and use that to determine
 * how many bytes to read from the socket and write to the file
 */
int save_file_upload(ClientState *client, const char *boundary, int file_fd) {
    // Read in the next two lines: Content-Type line, and empty line
    remove_buffered_line(client);
    remove_buffered_line(client);
    
    // IMPLEMENT THIS
    ssize_t write_result = write(file_fd, client->buf, client->num_bytes);
    if (write_result < 0) {
        perror("write");
        return -1;
    }
    client->num_bytes = 0;

    int boundary_len = strlen(boundary);
    char end_boundary[boundary_len + 6]; 
    sprintf(end_boundary, "\r\n%s--\r\n", boundary);    

    // Initialize a buffer
    char buffer[MAXLINE]; 

    size_t end_boundary_len = strlen(end_boundary);
    size_t end_boundary_found = 0;

    size_t end_boundary_found_prev = 0;
    char buffer_prev[MAXLINE];
    size_t write_bytes_prev = 0;
    
    while (1) {
        ssize_t read_bytes = read(client->sock, buffer, sizeof(buffer));

        if (read_bytes <= 0) {
            break;
        }

        // Search for the end boundary
        for (size_t i = 0; i < read_bytes; i++) {
            if (buffer[i] == end_boundary[end_boundary_found]) {
                end_boundary_found++;
                if (end_boundary_found == end_boundary_len) {
                    break;
                }
            } else {
                end_boundary_found = 0;
            }
        }      

        if (end_boundary_found == end_boundary_len) {
            if (end_boundary_found_prev) {
                ssize_t write_bytes = write_bytes_prev - end_boundary_found_prev;
                if (write_bytes > 0) {
                    write_result = write(file_fd, buffer_prev, write_bytes);
                    if (write_result < 0) {
                        perror("write");
                        break;
                    }
                }               
                break;
            }
            ssize_t write_bytes = read_bytes - end_boundary_found;
            if (write_bytes > 0) {
                write_result = write(file_fd, buffer, write_bytes);
                if (write_result < 0) {
                    perror("write");
                    break;
                }
            }
            break;
        } else if (end_boundary_found > 3) {
            // if the end boundary is found in the two buffer
            end_boundary_found_prev = end_boundary_found;
            strncpy(buffer_prev, buffer, read_bytes);
            write_bytes_prev = read_bytes;
        } else {
            write_result = write(file_fd, buffer, read_bytes);
            if (write_result < 0) {
                perror("write");
                break;
            }
        }
    }
  
    return 0;

}
