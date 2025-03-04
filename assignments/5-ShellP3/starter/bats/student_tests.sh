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

@test "Pipe with wc - count lines" {
    run "./dsh" <<EOF                
echo -e "Line 1\nLine 2\nLine 3" | wc -l
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="3dsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Multiple pipe segments" {
    run "./dsh" <<EOF                
echo "one two three four five" | tr " " "\n" | sort | head -3
EOF

    
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    expected_output="fivefouronedsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]

    [ "$status" -eq 0 ]
}

@test "cat and grep using pipe" {
    # First create a temporary file
    echo -e "apple\nbanana\norange\npear" > test_fruits.txt
    
    run "./dsh" <<EOF                
cat test_fruits.txt | grep an
EOF

    # Clean up temp file
    rm -f test_fruits.txt

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="bananaorangedsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]

    [ "$status" -eq 0 ]
}

@test "Empty pipe segment handling" {
    run "./dsh" <<EOF                
echo "test" | | grep test
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Should contain warning message about empty commands
    [[ "$stripped_output" == *"warning"* ]] || [[ "$stripped_output" == *"error"* ]]

    [ "$status" -eq 0 ]
}

@test "Too many pipe segments" {
    # Create a really long pipeline that exceeds CMD_MAX
    long_pipe="echo test"
    for i in {1..20}; do
        long_pipe="$long_pipe | grep test"
    done
    
    run "./dsh" <<EOF                
$long_pipe
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Should contain error about too many commands
    [[ "$stripped_output" == *"error"* && "$stripped_output" == *"pipe"* ]]

    [ "$status" -eq 0 ]
}

@test "Built-in command with pipe" {
    run "./dsh" <<EOF                
cd / | ls
EOF

    # Output should contain an error message about built-in commands in pipelines
    # or should handle it gracefully in some way
    [[ "$output" == *"error"* ]] || [[ "$output" == *"failed"* ]] || [[ "$output" == *"warning"* ]]

    [ "$status" -eq 0 ]
}

@test "Exit command after pipe" {
    run "./dsh" <<EOF                
echo "testing" | grep test
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="testingdsh3>dsh3>cmdloopreturned-7"  # OK_EXIT is -7

    # These echo commands will help with debugging
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [[ "$stripped_output" == *"testing"* ]]

    [ "$status" -eq 0 ]
}


# New Test Cases 
@test "Pipe with text containing special characters" {
    # Create a test file with the special characters
    echo 'Hello | World' > special_chars.txt
    
    run "./dsh" <<EOF                
cat special_chars.txt | grep Hello
EOF

    # Clean up
    rm -f special_chars.txt

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Check for just Hello in the output
    [[ "$stripped_output" == *"Hello"* ]]

    # Assertions
    [ "$status" -eq 0 ]
}


@test "Large data through pipe" {
    # Use a more reliable approach - create our own test file
    # Create a temporary file with 50 lines
    for i in {1..50}; do
        echo "Line $i" >> test_lines.txt
    done
    
    run "./dsh" <<EOF                
cat test_lines.txt | wc -l
EOF

    # Clean up
    rm -f test_lines.txt

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # The result should be "50"
    [[ "$stripped_output" == *"50"* ]]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Error propagation through pipes" {
    run "./dsh" <<EOF                
ls /nonexistent_directory | grep test
EOF

    # This should show an error for the first command but not crash the shell
    [[ "$output" == *"No such file"* || "$output" == *"cannot access"* || "$output" == *"not found"* ]]

    # Assertions - shell should still exit successfully
    [ "$status" -eq 0 ]
}

@test "Command not found in pipe" {
    run "./dsh" <<EOF                
echo "test" | nonexistentcommand
EOF

    # This should show an error for the second command but not crash the shell
    [[ "$output" == *"not found"* || "$output" == *"command not found"* || "$output" == *"No such file"* ]]

    # Assertions - shell should still exit successfully
    [ "$status" -eq 0 ]
}

@test "Redirecting stderr in pipe" {
    run "./dsh" <<EOF                
ls /nonexistent_directory 2>&1 | grep "No such file"
EOF

    # We should see the error message from ls
    [[ "$output" == *"No such file"* || "$output" == *"cannot access"* || "$output" == *"not found"* ]]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Three-command pipe works" {
    # Create a test file with specific content
    echo -e "line1\nline2\nline3\nline4\nline5" > test_lines2.txt
    
    run "./dsh" <<EOF                
cat test_lines2.txt | grep line | wc -l
EOF

    # Clean up
    rm -f test_lines2.txt

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # The result should contain "5"
    [[ "$stripped_output" == *"5"* ]]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Consecutive execution of pipes" {
    run "./dsh" <<EOF
ls | grep dsh
echo "second pipe" | wc -w
EOF

    # Output should contain "dsh" from first command
    [[ "$output" == *"dsh"* ]]
    
    # Output should contain "2" from second command
    [[ "$output" == *"2"* ]]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Pipe with process substitution-like behavior" {
    # Create a temporary file
    echo -e "word1\nword2\nword3" > test1.txt
    echo -e "word2\nword4\nword5" > test2.txt
    
    run "./dsh" <<EOF
cat test1.txt | grep -f test2.txt
EOF

    # Clean up temp files
    rm -f test1.txt test2.txt

    # Result should contain only the common word
    [[ "$output" == *"word2"* ]]
    
    # Assertions
    [ "$status" -eq 0 ]
}

@test "Complex filter pipe chain" {
    run "./dsh" <<EOF
cat /etc/passwd | grep r | grep a | grep o | wc -l
EOF

    # We don't check the exact number here because it could vary by system
    # Just making sure it runs without error
    
    # Assertions
    [ "$status" -eq 0 ]
}

@test "Pipeline with sort by key" {
    # Create a temporary file
    echo -e "3:apple\n1:banana\n2:orange" > test_sort.txt
    
    run "./dsh" <<EOF
cat test_sort.txt | sort -t: -k1,1n | cut -d: -f2
EOF

    # Clean up temp file
    rm -f test_sort.txt

    # Should output the fruits in numeric order
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    [[ "$stripped_output" == *"bananaorangeapple"* ]]
    
    # Assertions
    [ "$status" -eq 0 ]
}


