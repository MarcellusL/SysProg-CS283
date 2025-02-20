#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

int alloc_cmd_buff(cmd_buff_t *cmd_buff){
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX * sizeof(char));
    if (cmd_buff->_cmd_buffer == NULL){
        return ERR_MEMORY;
    }

    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++){
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer != NULL){
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++){
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff){
    while (isspace(*cmd_line)) cmd_line++;
    
    if (*cmd_line == '\0') {
        return WARN_NO_CMDS;
    }

    strcpy(cmd_buff->_cmd_buffer, cmd_line);

    cmd_buff->argc = 0;

    char *token = cmd_buff->_cmd_buffer;
    bool in_quotes = false;
    char *start = token;

    while (*token != '\0' && cmd_buff->argc < CMD_ARGV_MAX - 1){
        if (*token == '"'){
            if (!in_quotes){
                start = token + 1;
                in_quotes = true;
            } else {
                *token = '\0';
                cmd_buff->argv[cmd_buff->argc++] = start;
                in_quotes = false;
                start = token + 1;
            }
        } else if (!in_quotes && isspace(*token)){
            if (start != token){
                *token = '\0';
                cmd_buff->argv[cmd_buff->argc++] = start; 
            }
            start = token + 1;
        }
        token++;
    }

    if (start < token && *start != '\0' && cmd_buff->argc < CMD_ARGV_MAX - 1){
        cmd_buff->argv[cmd_buff->argc++] = start;
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;

    return OK;
}

Built_In_Cmds match_command(const char *input){
    if (strcmp(input, EXIT_CMD) == 0){
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0){
        return BI_CMD_CD;
    }
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd){
    if (cmd->argc == 0) return BI_NOT_BI;
    Built_In_Cmds type = match_command(cmd->argv[0]);
    
    switch (type){
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        
        case BI_CMD_CD:
            if (cmd->argc > 1){
                if (chdir(cmd->argv[1]) != 0){
                    perror("cd");
                }
            }
            return BI_EXECUTED;

        default:
            return BI_NOT_BI;
    }
}

int exec_cmd(cmd_buff_t *cmd){
   pid_t pid = fork();

   if (pid < 0){
      perror("fork");
      fprintf(stderr, "Failed to create process for command '%s'\n", cmd->argv[0]);
      return ERR_EXEC_CMD;
   }

   if (pid == 0) {
      execvp(cmd->argv[0], cmd->argv);
      perror("execvp");
      exit(1);
   } else {
      int status; 
      waitpid(pid, &status, 0);
      return OK;
   }
}

int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t cmd;

    // TODO IMPLEMENT MAIN LOOP

    // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff

    // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
    // the cd command should chdir to the provided directory; if no directory is provided, do nothing

    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

    cmd_buff = malloc(SH_CMD_MAX * sizeof(char));
    if (cmd_buff == NULL){
        return ERR_MEMORY;
    }

    rc = alloc_cmd_buff(&cmd);
    if (rc != OK){
        free(cmd_buff);
        return rc; 
    }

    while(1){
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        rc = build_cmd_buff(cmd_buff, &cmd);

        if (rc == WARN_NO_CMDS){
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        Built_In_Cmds bi_rc = exec_built_in_cmd(&cmd);
        if (bi_rc == BI_CMD_EXIT){
            rc = OK_EXIT;
            break;
        } else if (bi_rc == BI_EXECUTED){
            continue;
        }

        rc = exec_cmd(&cmd);
        if (rc != OK){
            printf("Failed executing command\n");
        }

       // clear_cmd_buff(&cmd);
    }

    free(cmd_buff);
    free_cmd_buff(&cmd);

    return rc;
}
