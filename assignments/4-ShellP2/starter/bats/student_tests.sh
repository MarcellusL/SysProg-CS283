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

@test "cd to invalid directory" {
    run "./dsh" <<EOF
cd /nonexistent_directory
pwd
EOF
    
    # Should stay in current directory and show error
    current_dir=$(pwd)
    stripped_output=$(echo "$output" | grep -v "cd: No such file or directory" | tr -d '[:space:]')
    expected_output="${current_dir}dsh2>dsh2>dsh2>cmdloopreturned0"

    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "cd with multiple arguments" {
    run "./dsh" <<EOF
cd /tmp multiple args
pwd
EOF
    
    # Should ignore extra arguments
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh2>dsh2>dsh2>cmdloopreturned0"
    
    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "quoted strings with multiple spaces" {
    run "./dsh" <<EOF
echo "   multiple    internal    spaces   "
EOF
    
    # Preserve spaces within quotes
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="   multiple    internal    spaces   dsh2> dsh2> cmd loop returned 0"
    
    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "mixed quoted and unquoted arguments" {
    run "./dsh" <<EOF
echo normal "quoted string" normal
EOF
    
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="normal quoted string normaldsh2> dsh2> cmd loop returned 0"
    
    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "empty command" {
    run "./dsh" <<EOF

EOF
    
    # Modify to match actual shell output
    stripped_output=$(echo "$output" | tr -d '\n\r\t' | sed 's/dsh2> //g')
    expected_output="warning: no commands provided"
    
    echo "Output: $output"
    echo "Stripped: $stripped_output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [[ "$stripped_output" == *"$expected_output"* ]]
    [ "$status" -eq 0 ]
}

@test "command with only spaces" {
    run "./dsh" <<EOF
     
EOF
    
    # Modify to match actual shell output
    stripped_output=$(echo "$output" | tr -d '\n\r\t' | sed 's/dsh2> //g')
    expected_output="warning: no commands provided"
    
    echo "Output: $output"
    echo "Stripped: $stripped_output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [[ "$stripped_output" == *"$expected_output"* ]]
    [ "$status" -eq 0 ]
}



@test "external command with path" {
    run "./dsh" <<EOF
/bin/echo test
EOF
    
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="testdsh2> dsh2> cmd loop returned 0"
    
    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "exit command" {
    run "./dsh" <<EOF
exit
EOF
    
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>cmdloopreturned-7"
    
    echo "Output: $output"
    echo "Expected: $expected_output"
    echo "Status: $status"
    
    [ "$stripped_output" = "$expected_output" ]
}
