#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Basic: ls command executes successfully" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Built-in: cd command changes directory" {
    run ./dsh <<EOF
mkdir -p test_dir
cd test_dir
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"test_dir"* ]]
}

@test "Pipeline: ls | grep works correctly" {
    run ./dsh <<EOF
ls -la | grep "."
EOF
    [ "$status" -eq 0 ]
    [[ "$output" != "" ]]
}

@test "Exit: exit command terminates shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Error handling: nonexistent command" {
    run ./dsh <<EOF
nonexistentcommand
EOF
    [[ "$output" == *"not found"* ]] || [[ "$output" == *"command not found"* ]] || [[ "$output" == *"No such file"* ]]
}

@test "Remote: client connects to server" {
    # Start server in background with output redirected to a file
    ./dsh -s -p 9876 > server_output.log 2>&1 &
    server_pid=$!
    
    # Give the server time to start up
    sleep 2
    
    # Check if server is running
    if ! kill -0 $server_pid 2>/dev/null; then
        echo "Server failed to start or crashed. Check server_output.log"
        cat server_output.log
        return 1
    fi
    
    # Run client with simple echo command
    ./dsh -c -p 9876 <<EOF > client_output.log 2>&1
echo "Hello from client"
exit
EOF
    client_status=$?
    
    # Kill server (with error suppression)
    kill $server_pid 2>/dev/null || true
    wait $server_pid 2>/dev/null || true
    
    # Check client results
    [ "$client_status" -eq 0 ]
    grep "Hello from client" client_output.log
    
    # Clean up
    rm -f server_output.log client_output.log
}

@test "Remote: stop-server command" {
    # Start server in background
    ./dsh -s -p 9877 &
    server_pid=$!
    sleep 1
    
    # Run client
    run ./dsh -c -p 9877 <<EOF
stop-server
EOF
    
    # Check if server is still running
    sleep 1
    kill -0 $server_pid 2>/dev/null || server_stopped=true
    
    [ "$server_stopped" = true ]
}

@test "Redirection: command output to file" {
    run ./dsh <<EOF
echo "test output" > test_output.txt
cat test_output.txt
rm test_output.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"test output"* ]]
}

@test "Multiple commands: sequential execution" {
    run ./dsh <<EOF
echo "first"
echo "second"
echo "third"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"first"*"second"*"third"* ]]
}

@test "Long pipeline: multiple pipe stages" {
    run ./dsh <<EOF
ls -la | grep "." | sort | head -n 3
EOF
    [ "$status" -eq 0 ]
    [[ "$output" != "" ]]
}

@test "Remote mode: basic connection" {
  # Start server in background
  ./dsh -s -p 8888 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 1
  
  # Run client
  timeout 5s ./dsh -c -p 8888 <<EOF > client_log.txt 2>&1
echo "Connection test successful"
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Check results
  grep -q "Connection test successful" client_log.txt
  
  # Cleanup
  rm -f server_log.txt client_log.txt
}
