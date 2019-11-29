#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

/**
 * HTTP response and header for a successful request. 
 */

static char* ok_response =
    "HTTP/1.0 200 OK\n"
    "Content-type: text/html\n"
    "\n";

/**
 * HTTP response, header, and body, indicating that we didn't
 * understand the request.
 */

static char* bad_request_response = 
    "HTTP/1.0 400 Bad Request\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Bad Request</h1>\n"
    "  <p>This server did not understand your request.</p>\n"
    " </body>\n"
    "</html>\n";

/**
 * HTTP response, header, and body, template, indicating that the
 * requested document was not found.
 */

static char* not_found_response_template =
    "HTTP/1.0 404 Not Found\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Not Found</h1>\n"
    "  <p>The requested URL %s was not found on this server.</p>\n"
    " </body>\n"
    "</html>\n";

/**
 * HTTP response, header, and body template, indicating that the
 * method was not understood.
 */

static char* bad_method_response_template = 
    "HTTP/1.0 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Method Not Implemented</h1>\n"
    "  <p>The method %s is not implemented by this server.</p>\n"
    " </body>\n"
    "</html>\n";

/**
 * Handler for SIGCHILD, to clean up child process that have terminated.
 */

static void clean_up_child_process (int signal_number)
{
    int status;
    wait (&status);
}

/**
 * Process an HTTP "GET" request for PAGE, and send the result to the
 * file descriptor CONNECTION_FD.
 */

static void handle_get (int connection_fd, const char* page)
{

}

/**
 * Handle a client connection on the file descriptor CONNECTION_FD.
 */

static void handle_connection (int connection_fd)
{

}

void server_run (struct in_addr local_address, uint16_t port)
{
    struct sockaddr_in socket_address;
    int rval;
    struct sigaction sigchld_action;
    int server_socket;

    /* Install a handler for SIGCHLD that cleans up child process that 
       that have terminated. */
    memset (&sigchld_action, 0, sizeof (sigchld_action));
    sigchld_action.sa_handler  = &clean_up_child_process;
    sigaction (SIGCHLD, &sigchld_action, NULL);

    /* Create a TCP socket. */
    server_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        system_error ("socket");

    /* Construct a socket address structure for the local address on
       which we want to listen for connections. */
    memset (&socket_address, 0, sizeof (socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = port;
    socket_address.sin_addr = local_address;

    /* Bind the socket to that address. */
    rval = bind (server_socket, &socket_address, sizeof (socket_address));
    if (rval != 0)
        system_error ("bind");

    /* Instruct the socket to accept connections. */
    rval = listen (server_socket, 10);
    if (rval != 0)
        system_error ("listen");

    if (verbose) 
    {
        /* In verbose mode, display the local address and port number
           we're listening on. */
        socklen_t address_lenght;

        /* Find the socket local address. */
        address_lenght = sizeof (socket_address);
        rval = getsockname (server_socket, &socket_address, &address_lenght);
        assert (rval == 0);

        /* Print a message. The port number needs to be converted from
           network byte order (big endian) to host byte order. */
        printf ("server listening on %s:%d\n",
                inet_ntoa (socket_address.sin_addr),
                (int) ntohs (socket_address.sin_port));
    }

    /* Loop forever, handling connections. */
    while (1)
    {
        struct sockaddr_in remote_address;
        socklen_t address_length;
        int connection;
        pid_t child_pid;

        /**
         * Accept a connection.
         * This call blocks untill a connection is ready.
         */

        address_length = sizeof (remote_address);
        connection = accept (server_socket, &remote_address, &address_length);
        if (connection == -1) {
            /* The call to accept failed. */
            if (errno == EINTR)
                /* The call was interrupted by a signal. Try again. */
                continue;
            else 
                /* Something else went wrong. */
                system_error ("accept");
        }

        /**
         * We have a connection.
         * Print a message if we're running in verbose mode.
         */

        if (verbose) {
            socklen_t address_length;

            /* Get the remote address of the connection. */
            address_length = sizeof (socket_address);
            rval = getpeername (connection, &socket_address, &address_length);
            assert (rval == 0);

            /* Print message. */
            printf ("connection accepted from %s\n", 
                    inet_ntoa (socket_address.sin_addr));
        }

        /**
         * Fork a child process to handle the connection.
         */

        child_pid = fork ();
        if (child_pid == 0) { 

            /**
             * This is the child process. 
             */
            
            /* It shouldn't use stdin or stdout, so close them. */
            close (STDIN_FILENO);
            close (STDOUT_FILENO);

            /* Also this child process shouldn't do anything with the listening socket. */
            close (server_socket);

            /* Handle a request from the connection.
               We have our own copy of the connected socket descriptor. */
            handle_connection (connection);

            /* All done;
               close the connection socket, and end the child process. */
            close (connection);
            exit (0);
        }
        else if (child_pid > 0) {
            
            /**
             * This is the parent process.
             * The child process handles the connection, so we don't need our 
             * copy of the connected socket descriptor. Close it. Then continue
             * with the loop and accept another connection.
             */
            
            close (connection);
        }
        else 
            /* Call to fork failed. */
            system_error ("fork");
    }    
}
