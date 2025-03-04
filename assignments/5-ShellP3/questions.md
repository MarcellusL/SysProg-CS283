1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes? 

My implementation ensures that all child processes complete before the shell continues accepting user input by looping through all the process IDs in the pids array and call waitpid() on each one. If waitpid() was forgotten to be called on all child processes, several issues could occur. Zombie processes would occur by the child processes would terminate but their exit status information would remain in the kernal. Another issue is the shell prompting for new input before previous pipeline completes.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

It's necessary to close unused pipe ends after calling dup2() to prevent read blocking since in a pipe, if the write end remains open, a process reading from read end will continue waiting for data and won't receive an EOF causing the process to not stop. Another issue is due to the process hanging on indefinitely if pipes are left open is high memory usage. 

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The "cd" command is implemented as a built-in rather than an external process command because chaning directories affects a process's state, and an external program can only change its own working directory, not its parent's. If cd were implemented as external process, the directory change would occur in the child process but would not affect the shell itself, making navigation impossible. 

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

How I would modify my code to allow for an arbitrary number of piped command while still handling memory allocation efficiently would be to replaced the fixed-size arrays with implementing a linked list or another type of dynamically allocated structure. Types of trade-offs I would need to consider is difficulty of implementation, runtime, and memory usage for a dynamic allocated list compared to having a fixed-size array. 
