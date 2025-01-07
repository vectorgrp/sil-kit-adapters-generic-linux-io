#!/bin/bash
set -e

echo "[info] Stopping processes"

stop_processes() {
    local processes=("$@")
    echo "[info] Processes to be stopped: ${processes[*]}"

    for process in "${processes[@]}"; do
        if pgrep -f "$process" > /dev/null; then
            (pkill -f "$process" && echo "[info] $process has been stopped") || echo "[error] Failed to stop $process"
        else
            echo "[info] $process is not running"
        fi
    done
}