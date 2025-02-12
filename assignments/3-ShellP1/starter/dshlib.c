#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int parse_commands(char *cmd_line, command_list_t *clist)
{
    return build_cmd_list(cmd_line, clist);
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    memset(clist, 0, sizeof(command_list_t));

    while (isspace(*cmd_line))
        cmd_line++;

    if (*cmd_line == '\0')
    {
        return WARN_NO_CMDS;
    }

    char *saveptr1;
    char *cmd = strtok_r(cmd_line, "|", &saveptr1);

    while (cmd != NULL)
    {
        if (clist->num >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        while (isspace(*cmd))
            cmd++;

        char *saveptr2; 
        char *token = strtok_r(cmd, " \t", &saveptr2);

        if (token != NULL)
        {
            if (strlen(token) >= EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcpy(clist->commands[clist->num].exe, token);

            clist->commands[clist->num].args[0] = '\0';
            token = strtok_r(NULL, " \t", &saveptr2);

            while (token != NULL)
            {
                if (strlen(clist->commands[clist->num].args) + strlen(token) + 2 >= ARG_MAX)
                {
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }

                if (clist->commands[clist->num].args[0] != '\0')
                {
                    strcat(clist->commands[clist->num].args, " ");
                }

                strcat(clist->commands[clist->num].args, token);
                token = strtok_r(NULL, " \t", &saveptr2);
            }

            clist->num++;
        }

        cmd = strtok_r(NULL, "|", &saveptr1);
        if (cmd)
        {
            while (isspace(*cmd))
                cmd++;
        }
    }

    return OK;
}