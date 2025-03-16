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

@test "Remote mode: file listing" {
  # Start server in background
  ./dsh -s -p 8891 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Create a test file
  echo "test content" > remote_test_file.txt
  
  # Run client
  timeout 5s ./dsh -c -p 8891 <<EOF > client_log.txt 2>&1
ls -la remote_test_file.txt
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Print logs for debugging
  echo "==== CLIENT LOG ===="
  cat client_log.txt
  echo "==== SERVER LOG ===="
  cat server_log.txt
  
  # Check results
  grep -q "remote_test_file.txt" client_log.txt
  
  # Cleanup
  rm -f remote_test_file.txt server_log.txt client_log.txt
}

@test "Remote mode: simple echo and cat pipeline" {
  # Start server in background
  ./dsh -s -p 8900 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Run client with a simple pipeline
  timeout 5s ./dsh -c -p 8900 <<EOF > client_log.txt 2>&1
echo "test" | cat
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Print logs for debugging
  echo "==== CLIENT LOG ===="
  cat client_log.txt
  
  # Check results
  grep -q "test" client_log.txt
  
  # Cleanup
  rm -f server_log.txt client_log.txt
}

@test "Remote mode: built-in command" {
  # Start server in background
  ./dsh -s -p 8895 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Create a test directory
  mkdir -p test_remote_dir
  
  # Run client with cd command
  timeout 5s ./dsh -c -p 8895 <<EOF > client_log.txt 2>&1
cd test_remote_dir
pwd
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Print logs for debugging
  echo "==== CLIENT LOG ===="
  cat client_log.txt
  
  # Check results - pwd should show the directory path containing test_remote_dir
  grep -q "test_remote_dir" client_log.txt
  
  # Cleanup
  rmdir test_remote_dir
  rm -f server_log.txt client_log.txt
}

@test "Remote mode: client connection timeout" {
  # Start server in background
  ./dsh -s -p 8896 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Try to connect to a different port (should fail)
  timeout 2s ./dsh -c -p 8897 <<EOF > client_log.txt 2>&1 || true
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Check client log for connection error
  grep -q "connect\|connection\|failed\|error\|refused" client_log.txt || [ ! -s client_log.txt ]
  
  # Cleanup
  rm -f server_log.txt client_log.txt
}

@test "Remote mode: multiple commands execution" {
  # Start server in background
  ./dsh -s -p 8892 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Run client with multiple commands
  timeout 5s ./dsh -c -p 8892 <<EOF > client_log.txt 2>&1
echo "first command"
echo "second command"
echo "third command"
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Print logs for debugging
  echo "==== CLIENT LOG ===="
  cat client_log.txt
  
  # Check results
  grep -q "first command" client_log.txt
  grep -q "second command" client_log.txt
  grep -q "third command" client_log.txt
  
  # Cleanup
  rm -f server_log.txt client_log.txt
}

@test "Remote mode: concurrent clients on threaded server" {
  # Start threaded server in background
  ./dsh -s -p 8902 -x > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Run first client in background
  ./dsh -c -p 8902 <<EOF > client1_log.txt 2>&1 &
sleep 3
echo "Client 1 output"
exit
EOF
  client1_pid=$!
  
  # Run second client while first is still running
  sleep 1
  ./dsh -c -p 8902 <<EOF > client2_log.txt 2>&1
echo "Client 2 output"
exit
EOF
  
  # Wait for first client to complete
  wait $client1_pid
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Check both clients got responses
  grep -q "Client 1 output" client1_log.txt
  grep -q "Client 2 output" client2_log.txt
  
  # Cleanup
  rm -f server_log.txt client1_log.txt client2_log.txt
}

@test "Remote mode: rc command returns status" {
  # Start server in background
  ./dsh -s -p 8903 > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Run client with successful and failing commands followed by rc
  timeout 5s ./dsh -c -p 8903 <<EOF > client_log.txt 2>&1
true
rc
false
rc
exit
EOF
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Print logs for debugging
  echo "==== CLIENT LOG ===="
  cat client_log.txt
  
  # Check for expected return codes (0 for true, 1 for false)
  grep -q "0" client_log.txt
  grep -q "1" client_log.txt
  
  # Cleanup
  rm -f server_log.txt client_log.txt
}

@test "Remote mode: threaded server under load" {
  # Start threaded server in background
  ./dsh -s -p 8904 -x > server_log.txt 2>&1 &
  server_pid=$!
  
  # Give server time to start
  sleep 2
  
  # Launch 5 simultaneous clients
  for i in {1..5}; do
    ./dsh -c -p 8904 <<EOF > client${i}_log.txt 2>&1 &
echo "Client $i connecting"
sleep 1
echo "Client $i completing"
exit
EOF
  done
  
  # Wait for clients to finish
  sleep 5
  
  # Cleanup server
  kill $server_pid 2>/dev/null || true
  wait $server_pid 2>/dev/null || true
  
  # Check all clients completed successfully
  for i in {1..5}; do
    grep -q "Client $i completing" client${i}_log.txt
  done
  
  # Cleanup
  rm -f server_log.txt client{1..5}_log.txt
}