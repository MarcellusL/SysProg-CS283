#!/bin/bash

# Debug info
echo "Current directory: $(pwd)"
echo "dsh exists: $(ls -la ./dsh)"

# Start server with ALL output captured
./dsh -s -p 8889 > server_log.txt 2>&1 || { echo "Server failed to start" > server_error.txt; exit 1; }
server_pid=$!
echo "Server started with PID: $server_pid"

# Make sure the server is running
if ! ps -p $server_pid > /dev/null; then
  echo "ERROR: Server process not running!"
  exit 1
fi

# Give server time to start
sleep 3

#Check if port is open.
if ! timeout 2s nc -z localhost 8889; then
    echo "ERROR: Server port 8889 is not open. Server may have failed to start correctly."
    kill $server_pid 2>/dev/null || true
    wait $server_pid 2>/dev/null || true
    exit 1
fi

# Run client with ALL output captured with a 10 second timeout.
timeout 10s echo 'echo "test" | cat\nexit' | ./dsh -c -p 8889 > client_log.txt 2>&1
client_exit=$?
echo "Client exited with status: $client_exit"

# Cleanup server
kill $server_pid 2>/dev/null || true
wait $server_pid 2>/dev/null || true

# Show logs and process information
echo "==== CLIENT LOG ===="
cat client_log.txt
echo "==== SERVER LOG ===="
cat server_log.txt
echo "==== ACTIVE PROCESSES ===="
ps aux | grep dsh