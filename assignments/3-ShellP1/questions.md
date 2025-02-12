1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  'fgets' is a good choice because it handles line-by-line input, matching how users will enter shell commands. It provides a built-in buffer overflow protection by taking in a maximum size parameter and preserves the newline character, which can be removed by creating our own functionality to help in command processsing.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using malloc() for cmd_buff was necessary because memory needed to be allocated on a heap instead of a stack. The reason why because, stack memory is limited and could overflow.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  It's necessary to trim leading and trailing spaces from each command because it can cause errors for the command calls since command with whitespace is not the same as one without. If we didn't trim the spaces, problems that can arise are failure in argument parsing or can be interpreted as addional arguments for a command. 

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  An example of redirect is to have a redirect STDOUT to a file. A second example is taking from a file instead of STDIN, and a third option for redirection is error redirection, which redirect STDERR to a file. Potential challenges we might encounter implementing these is check handling for file permissions, handling of large input files efficiently, or error redirection needs to handle both STDOUT and STDERR separately. 

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection allows us to redirect the output or errors to different desinations, such as output to a file or takes input from a file . Pipes connect two programs togethor, the output of one program becomes the input of another. 

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It's important to keep STDERR and STDOUT in separate shells is important for error handling, pipeline processing where error messages are not suppose to interfere with the data being processed, and programs can parse regular output while showing errors from STDERR. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**: How our shell should handle errors from commands that fail by using redirection and this can be achieved by creating individual directions with STDOUT goes to output and STDERR goes to error. After, we can merge them with STDOUT and STDERR going to the same file.