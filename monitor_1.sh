#!/bin/bash

# Configuration
PROJECT_NAME="MyProject"
PROJECT_PATH="/www/wwwroot/backend.mzingamaji.co.tz/build/MyProject"
LOG_FILE="/www/wwwroot/backend.mzingamaji.co.tz/myproject_monitor_1.log"
CHECK_INTERVAL=600  # 10 minutes in seconds
MAX_LOG_SIZE=$((50*1024*1024))  # 50MB in bytes

# Function to log messages
log_message() {
    local message="$(date '+%Y-%m-%d %H:%M:%S'): $1"
    echo "$message" | tee -a "$LOG_FILE"
    
    # Rotate log if it exceeds max size
    if [[ $(stat -c%s "$LOG_FILE") -gt $MAX_LOG_SIZE ]]; then
        mv "$LOG_FILE" "${LOG_FILE}.old"
        touch "$LOG_FILE"
        echo "$(date '+%Y-%m-%d %H:%M:%S'): Log file rotated." | tee -a "$LOG_FILE"
    fi
}

# Function to start MyProject
start_myproject() {
    log_message "Starting $PROJECT_NAME..."
    "$PROJECT_PATH" &
    sleep 2  # Give it a moment to start
}

# Function to check if MyProject is running
check_myproject() {
    pgrep -f "$PROJECT_NAME" > /dev/null 2>&1
    return $?
}

# Function to stop the script gracefully
cleanup() {
    log_message "Stopping monitor script."
    exit 0
}

# Set up trap for graceful shutdown
trap cleanup SIGINT SIGTERM

# Main monitoring loop
log_message "Starting monitor script for $PROJECT_NAME"
while true; do
    if ! check_myproject; then
        log_message "$PROJECT_NAME is not running. Attempting to restart..."
        start_myproject
        if check_myproject; then
            log_message "$PROJECT_NAME successfully restarted."
        else
            log_message "Failed to restart $PROJECT_NAME. Manual intervention may be required."
        fi
    fi
    
    sleep "$CHECK_INTERVAL"
done
