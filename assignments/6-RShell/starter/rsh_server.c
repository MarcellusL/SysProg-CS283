
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
#include <signal.h>
#include <pthread.h>
static int use_threads = 0;
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    use_threads = is_threaded;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int svr_socket;
    int ret;
    
    struct sockaddr_in addr;

    // TODO set up the socket - this is very similar to the demo code
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        close (svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }


    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret > 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket){
    int     cli_socket;
    int     rc = OK;   
    
    //new 
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    extern int use_threads;

    while(1){
        // TODO use the accept syscall to create cli_socket 
        // and then exec_client_requests(cli_socket)
        cli_socket = accept(svr_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
    

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s:%d\n", client_ip, ntohs(cli_addr.sin_port));

        if (use_threads) {
            pthread_t thread_id;
            int *client_sock = malloc(sizeof(int));
            *client_sock = cli_socket;

            if (pthread_create(&thread_id, NULL, handle_client, client_sock) != 0) {
                perror("pthread_create");
                free(client_sock);
                stop_server(cli_socket);
                printf("Failed to create thread for client\n");
            }
        } else {

            rc = exec_client_requests(cli_socket);
            
            stop_server(cli_socket);

            if (rc == OK_EXIT) {
                printf("%s", RCMD_MSG_SVR_STOP_REQ);
                break;
            } else {
                printf("%s", RCMD_MSG_CLIENT_EXITED);
            }
        }  
    }

    return rc;
}

    /*
     * exec_client_requests(cli_socket)
     *      cli_socket:  The server-side socket that is connected to the client
     *
     *  This function handles accepting remote client commands. The function will
     *  loop and continue to accept and execute client commands.  There are 2 ways
     *  that this ongoing loop accepting client commands ends:
     *
     *      1.  When the client executes the `exit` command, this function returns
     *          to process_cli_requests() so that we can accept another client
     *          connection.
     *      2.  When the client executes the `stop-server` command this function
     *          returns to process_cli_requests() with a return code of OK_EXIT
     *          indicating that the server should stop.
     *
     *  Note that this function largely follows the implementation of the
     *  exec_local_cmd_loop() function that you implemented in the last
     *  shell program deliverable. The main difference is that the command will
     *  arrive over the recv() socket call rather than reading a string from the
     *  keyboard.
     *
     *  This function also must send the EOF character after a command is
     *  successfully executed to let the client know that the output from the
     *  command it sent is finished.  Use the send_message_eof() to accomplish
     *  this.
     *
     *  Of final note, this function must allocate a buffer for storage to
     *  store the data received by the client. For example:
     *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
     *  And since it is allocating storage, it must also properly clean it up
     *  prior to exiting.
     *
     *  Returns:
     *
     *      OK:       The client sent the `exit` command.  Get ready to connect
     *                another client.
     *      OK_EXIT:  The client sent `stop-server` command to terminate the server
     *
     *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
     *                or receive errors.
     */
    int exec_client_requests(int cli_socket)
    {
        int io_size;
        command_list_t cmd_list;
        int rc;
        int cmd_rc;
        int last_rc;
        char *io_buff;

        memset(&cmd_list, 0, sizeof(command_list_t));

        io_buff = malloc(RDSH_COMM_BUFF_SZ);
        if (io_buff == NULL)
        {
            return ERR_RDSH_SERVER;
        }

        while (1)
        {
            // TODO use recv() syscall to get input

            // TODO build up a cmd_list

            // TODO rsh_execute_pipeline to run your cmd_list

            // TODO send appropriate respones with send_message_string
            // - error constants for failures
            // - buffer contents from execute commands
            //  - etc.

            // TODO send_message_eof when done
            io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);

            if (io_size <= 0)
            {
                if (io_size <= 0)
                {
                    perror("recv");
                }
                free(io_buff);
                return ERR_RDSH_COMMUNICATION;
            }

            io_buff[io_size] = '\0';

            printf(RCMD_MSG_SVR_EXEC_REQ, io_buff);

            if (strcmp(io_buff, EXIT_CMD) == 0)
            {
                char *exit_msg = "Exiting...\n";
                send_message_string(cli_socket, exit_msg);
                break;
            }
            else if (strcmp(io_buff, "stop-server") == 0)
            {
                char *stop_msg = "Server shutting down...\n";
                send_message_string(cli_socket, stop_msg);
                free(io_buff);
                return OK_EXIT;
            }
            else
            {
                if (strchr(io_buff, PIPE_CHAR))
                {

                    FILE *fp = popen(io_buff, "r");
                    if (fp)
                    {
                        char buffer[1024];
                        while (fgets(buffer, sizeof(buffer), fp))
                        {
                            send_message_string(cli_socket, buffer);
                        }
                        pclose(fp);

                        send_message_eof(cli_socket);
                    }
                    else
                    {
                        send_message_string(cli_socket, "Error executing pipeline command\n");
                    }
                    continue;
                }

                rc = build_cmd_list(io_buff, &cmd_list);

                if (rc == WARN_NO_CMDS)
                {
                    send_message_string(cli_socket, CMD_WARN_NO_CMD);
                    continue;
                }
                else if (rc == ERR_TOO_MANY_COMMANDS)
                {
                    char error_msg[100];
                    sprintf(error_msg, CMD_ERR_PIPE_LIMIT, CMD_MAX);
                    send_message_string(cli_socket, error_msg);
                    continue;
                }
                else if (rc != OK)
                {
                    char error_msg[100];
                    sprintf(error_msg, CMD_ERR_RDSH_ITRNL, rc);
                    send_message_string(cli_socket, error_msg);
                    continue;
                }

                if (cmd_list.num == 1)
                {
                    cmd_buff_t *cmd = &(cmd_list.commands[0]);

                    if (cmd->argc > 0 && strcmp(cmd->argv[0], "cd") == 0)
                    {
                        if (cmd->argc > 1)
                        {
                            if (chdir(cmd->argv[1]) != 0)
                            {
                                char error_msg[100];
                                sprintf(error_msg, "cd: %s: No such file or directory\n", cmd->argv[1]);
                                send_message_string(cli_socket, error_msg);
                            }
                            else
                            {
                                send_message_string(cli_socket, "");
                            }
                        }
                        else
                        {
                            send_message_string(cli_socket, "");
                        }

                        free_cmd_list(&cmd_list);
                        continue;
                    }
                }

                cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);
                last_rc = cmd_rc;

                printf("DEBUG: Pipeline execution complete, about to send EOF\n");
                fflush(stdout);

                free_cmd_list(&cmd_list);
                send_message_eof(cli_socket);
            }
        }

        free(io_buff);
        return last_rc;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */

int send_message_eof(int cli_socket)
{
    printf("DEBUG: send_message_eof called, cli_socket: %d\n", cli_socket); // Added cli_socket
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len)
    {
        printf("DEBUG: send_message_eof failed, sent_len: %d, expected: %d\n", sent_len, send_len); // Added sent_len
        perror("send");
        return ERR_RDSH_COMMUNICATION;
    }
    printf("DEBUG: send_message_eof success, sent_len: %d, EOF char: %x\n", sent_len, RDSH_EOF_CHAR); // Added sent_len and EOF char
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    //TODO implement writing to cli_socket with send()
    int send_len = strlen(buff);
    int sent_len; 

    sent_len = send(cli_socket, buff, send_len, 0) ;
    
    if (sent_len != send_len) {
        if (sent_len < 0) {
            perror("send");
        } else {
            printf(CMD_ERR_RDSH_SEND, sent_len, send_len);
        }
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];  // Array of pipes
    pid_t pids[clist->num];
    int  pids_st[clist->num];         // Array to store process IDs
    Built_In_Cmds bi_cmd;
    int exit_code;

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_RDSH_CMD_EXEC;
        }
    }

        // TODO this is basically the same as the piped fork/exec assignment, except for where you connect the begin and end of the pipeline (hint: cli_sock)

        // TODO HINT you can dup2(cli_sock with STDIN_FILENO, STDOUT_FILENO, etc.

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num; i++) {
        cmd_buff_t *cmd = &(clist->commands[i]);
        
        if (i > 0) {
            bi_cmd = rsh_built_in_cmd(cmd);
            if (bi_cmd != BI_NOT_BI) {
                char *error_msg = "Built-in commands cannot be used in pipelines\n";
                send_message_string(cli_sock, error_msg);

                for (int j = 0; j < clist->num - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                return ERR_RDSH_CMD_EXEC;
            }
        }
        
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            char *error_msg = "Failed to create process\n";
            send_message_string(cli_sock, error_msg);
        
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return ERR_RDSH_CMD_EXEC;
        } else if (pids[i] == 0) {
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            } else {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            if (i == clist->num -1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            close(cli_sock);

            execvp(cmd->argv[0], cmd->argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    printf("DEBUG: All children forked, about to wait\n");
    fflush(stdout);

    // Wait for all children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    printf("DEBUG: All children completed\n");
    fflush(stdout);


    //by default get exit code of last process
    //use this as the return value
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    for (int i = 0; i < clist->num; i++) {
        //if any commands in the pipeline are EXIT_SC
        //return that to enable the caller to react
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC)
            exit_code = EXIT_SC;
    }
    return exit_code;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    // case BI_CMD_DRAGON:
    //     print_dragon();
    //     return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}


void *handle_client(void *arg) {
    int cli_socket = *((int *)arg);
    free(arg);

    exec_client_requests(cli_socket);
    pthread_detach(pthread_self());
    
    return NULL;
}
