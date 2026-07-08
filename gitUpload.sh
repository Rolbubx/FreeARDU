#!/bin/bash

################################################################################
# Git Upload Script for Linux/macOS
# Purpose: Safe and user-friendly Git commit and push automation
# Version: 2.0 (Professional Edition)
################################################################################

# Color definitions for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Configuration
SCRIPT_VERSION="2.0"
SCRIPT_NAME="Git Upload Assistant"

################################################################################
# UTILITY FUNCTIONS
################################################################################

print_header() {
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC}                                                                            ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}                  ${BOLD}${SCRIPT_NAME} v${SCRIPT_VERSION}${NC}                            ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}              Safe and Professional Git Automation                        ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}                                                                            ${BLUE}║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_step() {
    echo -e "${BLUE}║${NC} [STEP $1] $2"
    echo -e "${BLUE}║${NC}"
}

print_success() {
    echo -e "${BLUE}║${NC} ${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${BLUE}║${NC} ${RED}✗${NC} ${BOLD}ERROR:${NC} $1"
}

print_warning() {
    echo -e "${BLUE}║${NC} ${YELLOW}⚠${NC}  $1"
}

print_info() {
    echo -e "${BLUE}║${NC} ℹ  $1"
}

print_divider() {
    echo -e "${BLUE}║${NC} ─────────────────────────────────────────────────────────────────────────"
}

print_section() {
    echo ""
    echo -e "${BLUE}╠════════════════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${BLUE}║${NC}  $1"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
}

error_exit() {
    echo ""
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    read -p "Press Enter to exit..."
    exit 1
}

################################################################################
# SCRIPT START
################################################################################

clear
print_header

################################################################################
# STEP 1: CHECK GIT INSTALLATION
################################################################################

print_step "1/5" "Checking Git Installation..."

if ! command -v git &> /dev/null; then
    print_error "Git is not installed or not found in PATH"
    echo ""
    print_info "Please install Git using one of the following commands:"
    echo -e "${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}   macOS:  brew install git"
    echo -e "${BLUE}║${NC}   Ubuntu: sudo apt-get install git"
    echo -e "${BLUE}║${NC}   Fedora: sudo dnf install git"
    echo -e "${BLUE}║${NC}   CentOS: sudo yum install git"
    echo ""
    error_exit
fi

GIT_VERSION=$(git --version)
print_success "Git found: ${CYAN}${GIT_VERSION}${NC}"
echo -e "${BLUE}║${NC}"

################################################################################
# STEP 2: CHECK REPOSITORY STATUS
################################################################################

print_step "2/5" "Checking Repository Status..."

if ! git rev-parse --is-inside-work-tree &> /dev/null; then
    print_error "Not a Git repository"
    echo ""
    print_info "Please navigate to a valid Git repository and run this script again."
    echo ""
    error_exit
fi

print_success "Valid Git repository detected"
echo -e "${BLUE}║${NC}"

################################################################################
# STEP 3: CHECK FOR MODIFICATIONS
################################################################################

print_step "3/5" "Scanning for Changes..."

# Check if there are any changes
if [[ -z $(git status --porcelain) ]]; then
    print_success "Status: Repository is clean"
    echo ""
    print_info "No changes detected to commit"
    echo ""
    print_info "This could mean:"
    echo -e "${BLUE}║${NC}   • All changes have already been committed"
    echo -e "${BLUE}║${NC}   • No files have been modified"
    echo -e "${BLUE}║${NC}   • All changes were staged but not tracked"
    echo ""
    print_info "If you have uncommitted changes, ensure they are in the working directory."
    echo ""
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    exit 0
fi

print_success "Changes detected - ready to commit"
echo -e "${BLUE}║${NC}"

################################################################################
# STEP 4: GET CURRENT BRANCH
################################################################################

print_step "4/5" "Retrieving Branch Information..."

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
print_success "Current branch: ${CYAN}${CURRENT_BRANCH}${NC}"
echo -e "${BLUE}║${NC}"

################################################################################
# STEP 5: GET COMMIT MESSAGE
################################################################################

echo -e "${BLUE}║${NC} Commit Message (required):"
echo -e "${BLUE}║${NC}"

read -p "$(echo -e "${BLUE}║${NC} Enter commit message: ")" COMMIT_MSG

if [[ -z "$COMMIT_MSG" ]]; then
    echo -e "${BLUE}║${NC}"
    print_error "Commit message cannot be empty"
    echo ""
    error_exit
fi

echo -e "${BLUE}║${NC}"
echo -e "${BLUE}║${NC} Commit message set: ${CYAN}\"${COMMIT_MSG}\"${NC}"
echo -e "${BLUE}║${NC}"

################################################################################
# DISPLAY SUMMARY & CONFIRMATION
################################################################################

print_step "5/5" "Review and Confirm"
echo -e "${BLUE}║${NC}"
print_divider
echo -e "${BLUE}║${NC}"
echo -e "${BLUE}║${NC}  Target Branch    : ${CYAN}${CURRENT_BRANCH}${NC}"
echo -e "${BLUE}║${NC}  Commit Message   : ${CYAN}${COMMIT_MSG}${NC}"
echo -e "${BLUE}║${NC}"
print_divider
echo -e "${BLUE}║${NC}"

read -p "$(echo -e "${BLUE}║${NC} Proceed with commit and push? [Y/N]: ")" CONFIRM

if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    echo -e "${BLUE}║${NC}"
    print_error "Operation cancelled by user"
    echo ""
    error_exit
fi

echo -e "${BLUE}║${NC}"

################################################################################
# EXECUTE GIT OPERATIONS
################################################################################

echo -e "${BLUE}║${NC} Executing Git operations..."
echo -e "${BLUE}║${NC}"

# Stage all changes
echo -e "${BLUE}║${NC} • Staging changes..."
if ! git add -A 2>/dev/null; then
    print_error "Failed to stage changes"
    echo ""
    error_exit
fi
print_success "Changes staged successfully"
echo -e "${BLUE}║${NC}"

# Commit changes
echo -e "${BLUE}║${NC} • Committing changes..."
if ! git commit -m "$COMMIT_MSG" &>/dev/null; then
    print_error "Failed to commit changes"
    echo ""
    print_info "This could indicate:"
    echo -e "${BLUE}║${NC}   • Commit message contains invalid characters"
    echo -e "${BLUE}║${NC}   • No changes were actually staged"
    echo -e "${BLUE}║${NC}   • Permission issues in the repository"
    echo ""
    error_exit
fi
print_success "Commit created successfully"
echo -e "${BLUE}║${NC}"

# Push changes
echo -e "${BLUE}║${NC} • Pushing to remote repository..."
if ! git push 2>&1; then
    echo -e "${BLUE}║${NC}"
    echo -e "${BLUE}╠════════════════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${BLUE}║${NC} ${YELLOW}⚠${NC}  ${BOLD}WARNING: Git push encountered an error!${NC}"
    echo -e "${BLUE}║${NC}"
    print_info "Possible causes:"
    echo -e "${BLUE}║${NC}   • No internet connection"
    echo -e "${BLUE}║${NC}   • Merge conflicts exist"
    echo -e "${BLUE}║${NC}   • Remote branch has diverged from local"
    echo -e "${BLUE}║${NC}   • Insufficient permissions"
    echo -e "${BLUE}║${NC}   • Remote repository is not accessible"
    echo -e "${BLUE}║${NC}"
    print_info "Please resolve the issue and try pushing manually using:"
    echo -e "${BLUE}║${NC}   ${CYAN}git push --force-with-lease${NC}"
    echo -e "${BLUE}║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

print_success "Changes pushed successfully"
echo -e "${BLUE}║${NC}"

################################################################################
# SUCCESS MESSAGE
################################################################################

echo ""
echo -e "${BLUE}╠════════════════════════════════════════════════════════════════════════════╣${NC}"
echo -e "${BLUE}║${NC}  ${GREEN}✓ SUCCESS!${NC}"
echo -e "${BLUE}║${NC}"
echo -e "${BLUE}║${NC}  Your changes have been successfully committed and pushed to:"
echo -e "${BLUE}║${NC}  Branch: ${CYAN}${CURRENT_BRANCH}${NC}"
echo -e "${BLUE}║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
echo ""

exit 0
