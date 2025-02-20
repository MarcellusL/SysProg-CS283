1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Why we used execvp directly, the shell process would replace the shell process entirely. How this happens is ls could be processed and after the shell would be gone. Instead, we can use fork/exec to create a copy of the process and execute a child process. 

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() system call fails, the child process isn't created with the original process continuing to execute. How my implementation handles this scenario is to print the perror letting me know it was a fork error along with the argument value that failed.  

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  The execvp() finds the command to execute by finding the path of the name provided. If not found, it would go into other path directories until the command is found or until there are no more directories. The PATH environment variable plays a role in this process tgat execvp() uses. 

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose calling wait() in the parent process after forking because the child process doesn't go away. What happens is that without the wait, it would wait for the next command to be entered in.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() gets the exit code from waitpid. This information is important because the exit code is used to determine success or failure. 

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My build_cmd_buff() handles quoted arguments by creating extra spacing if the quoted text has spacing. If the quotes didn't exist, it would separate the texts according to the spaces into individuals indices. Why quote handling is necessary because it could be needed for commands in the shell. 

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: Changes I made to my parsing logic was removed pipe handling, changed command_list_t to cmd_buff_t, and changed from concatenatinng args into a string to building an argv array. 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  The purpose of signals in a linux system is to handle processes to either stop, continue, or terminate the program. How signals differ from other forms of IPCs by features for data transfers between processes. 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL kills the signal right away and cannot be caught, blocked, or killed. SIGTERM terminates the signal, process will shut down and allows for cleanup. SIGINT is an interrupt from keyboard (ctrl + c).

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process recieves SIGSTOP, the process execution immediately stops, can be resumed later with SIGCONT, SIGSTOP cannot be caught. Why SIGSTOP cannot be caught because it can be resumed.
