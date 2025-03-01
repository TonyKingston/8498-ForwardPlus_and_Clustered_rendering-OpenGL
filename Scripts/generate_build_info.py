import subprocess
import datetime
import json
from typing import Optional

# Constants
BUILD_HEADER_FILE = "build.h"
BUILD_JSON_FILE = "build.json"

# Helper function to run shell commands
def run_command(command: list[str]) -> Optional[str]:
    """Executes a shell command and returns its output."""
    try:
        return subprocess.check_output(command, text=True).strip()
    except subprocess.CalledProcessError:
        return None

# Fetch the latest Git commit hash
def get_git_commit() -> str:
    return run_command(["git", "rev-parse", "--short", "HEAD"]) or "unknown"

# Get current branch name
def get_git_branch() -> str:
    return run_command(["git", "rev-parse", "--abbrev-ref", "HEAD"]) or "unknown"

# Check if working directory is dirty
def is_git_dirty() -> str:
    return "dirty" if run_command(["git", "status", "--porcelain"]) else "clean"

# Get latest git tag (used for versioning)
# Will be the same as the commit hash if no tags are available
def get_git_version() -> str:
    return run_command(["git", "describe", "--tags", "--always"]) or "v0.0.0"

# Get current date and time
def get_build_timestamp() -> str:
    return datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")

# Generate build header
def generate_build_header() -> None:
    commit_hash = get_git_commit()
    branch_name = get_git_branch()
    git_status = is_git_dirty()
    git_version = get_git_version()
    build_timestamp = get_build_timestamp()

    header_content = f"""#pragma once

// Automatically generated build information
#define NCL_NAME "NCL"
#define NCL_VERSION "{git_version}"
#define NCL_BUILD_COMMIT "{commit_hash}"
#define NCL_BUILD_BRANCH "{branch_name}"
#define NCL_BUILD_STATUS "{git_status}"
#define NCL_BUILD_TIMESTAMP "{build_timestamp}"
"""
    
    with open(BUILD_HEADER_FILE, "w") as f:
        f.write(header_content)

    # Generate JSON build metadata
    build_metadata = {
        "name": "NCL",
        "version": git_version,
        "commit": commit_hash,
        "branch": branch_name,
        "status": git_status,
        "timestamp": build_timestamp
    }
    with open(BUILD_JSON_FILE, "w") as f:
        json.dump(build_metadata, f, indent=4)

    print(f"{BUILD_HEADER_FILE} and {BUILD_JSON_FILE} updated successfully!")

if __name__ == "__main__":
    generate_build_header()