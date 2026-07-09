@echo off
setlocal enabledelayedexpansion

::==============================================================================
:: Git Upload Script for Windows
:: Purpose: Safe and user-friendly Git commit and push automation
:: Version: 2.0 (Professional Edition)
::==============================================================================

chcp 65001 >nul
cls

::==============================================================================
:: CONFIGURATION
::==============================================================================

set "SCRIPT_VERSION=2.0"
set "SCRIPT_NAME=Git Upload Assistant"

::==============================================================================
:: BANNER & HEADER
::==============================================================================

echo.
echo ╔══════════════════════════════════════════════════════════════════════════╗
echo ║                                                                          ║
echo ║                  %SCRIPT_NAME% v%SCRIPT_VERSION%                         ║
echo ║              Safe and Professional Git Automation                        ║
echo ║                                                                          ║
echo ╚══════════════════════════════════════════════════════════════════════════╝
echo.

::==============================================================================
:: CHECK GIT INSTALLATION
::==============================================================================

echo ║ [STEP 1/6] Checking Git Installation...
echo ║

git --version >nul 2>&1

if errorlevel 1 (
    echo  ══════════════════════════════════════════════════════════════════════════
    echo ║ ✗ ERROR: Git is not installed or not found in PATH
    echo ║
    echo ║ Please install Git from: https://git-scm.com/download/win
    echo ║ After installation, restart this script.
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

for /f "tokens=*" %%i in ('git --version') do set "GIT_VERSION=%%i"
echo ║ ✓ Git found: %GIT_VERSION%
echo ║

::==============================================================================
:: CHECK REPOSITORY STATUS
::==============================================================================

echo ║ [STEP 2/6] Checking Repository Status...
echo ║

git rev-parse --is-inside-work-tree >nul 2>&1

if errorlevel 1 (
    echo ║ ✗ ERROR: Not a Git repository
    echo ║
    echo ║ Please navigate to a valid Git repository and run this script again.
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║ ✓ Valid Git repository detected
echo ║

::==============================================================================
:: CHECK FOR MODIFICATIONS
::==============================================================================

echo ║ [STEP 3/6] Scanning for Changes...
echo ║

for /f "tokens=*" %%i in ('git status --porcelain') do set "HAS_CHANGES=true"

if not defined HAS_CHANGES (
    echo ║ ✓ Status: Repository is clean
    echo ║
    echo ║ ℹ  No changes detected to commit
    echo ║
    echo ║ This could mean:
    echo ║   • All changes have already been committed
    echo ║   • No files have been modified
    echo ║   • All changes were staged but not tracked
    echo ║
    echo ║ If you have uncommitted changes, ensure they are in the working directory.
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 0
)

echo ║ ✓ Changes detected - ready to commit
echo ║

::==============================================================================
:: GET CURRENT BRANCH
::==============================================================================

echo ║ [STEP 4/6] Retrieving Branch Information...
echo ║

for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD') do set "CURRENT_BRANCH=%%i"

echo ║ ✓ Current branch: %CURRENT_BRANCH%
echo ║

::==============================================================================
:: BRANCH SELECTION
::==============================================================================

echo ║ Branch Selection:
echo ║

set /p BRANCH_CHOICE="║ Push to current branch [%CURRENT_BRANCH%]? [Y/N/C]: "

set "TARGET_BRANCH=%CURRENT_BRANCH%"

if /i "!BRANCH_CHOICE!"=="N" (
    echo ║
    echo ║ Enter new branch name:
    echo ║
    
    set "NEW_BRANCH="
    set /p NEW_BRANCH=║ Branch name: 
    
    if "!NEW_BRANCH!"=="" (
        echo ║
        echo ║ ✗ ERROR: Branch name cannot be empty
        echo ║
        echo ╚══════════════════════════════════════════════════════════════════════════╝
        echo.
        pause
        exit /b 1
    )
    
    set "TARGET_BRANCH=!NEW_BRANCH!"
    echo ║
    echo ║ ✓ Target branch set to: !TARGET_BRANCH!
    echo ║
) else if /i "!BRANCH_CHOICE!"=="C" (
    echo ║
    echo ║ ✗ Operation cancelled by user
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
) else (
    echo ║
)

::==============================================================================
:: GET COMMIT MESSAGE
::==============================================================================

echo ║ Commit Message (required):
echo ║

set "COMMIT_MSG="
set /p COMMIT_MSG=║ Enter commit message: 

if "!COMMIT_MSG!"=="" (
    echo ║
    echo ║ ✗ ERROR: Commit message cannot be empty
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║
echo ║ Commit message set: "%COMMIT_MSG%"
echo ║

::==============================================================================
:: DISPLAY SUMMARY & CONFIRMATION
::==============================================================================

echo ║ [STEP 5/6] Review and Confirm
echo ║
echo ║ ─────────────────────────────────────────────────────────────────────────
echo ║
echo ║  Target Branch    : %TARGET_BRANCH%
echo ║  Commit Message   : %COMMIT_MSG%
echo ║
echo ║ ─────────────────────────────────────────────────────────────────────────
echo ║

set /p CONFIRM="║ Proceed with commit and push? [Y/N]: "

if /i not "!CONFIRM!"=="Y" (
    echo ║
    echo ║ ✗ Operation cancelled by user
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║

::==============================================================================
:: EXECUTE GIT OPERATIONS
::==============================================================================

echo ║ Executing Git operations...
echo ║

:: Stage all changes
echo ║ • Staging changes...
git add -A >nul 2>&1

if errorlevel 1 (
    echo ║ ✗ ERROR: Failed to stage changes
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║ ✓ Changes staged successfully
echo ║

:: Commit changes
echo ║ • Committing changes...
git commit -m "!COMMIT_MSG!" >nul 2>&1

if errorlevel 1 (
    echo ║ ✗ ERROR: Failed to commit changes
    echo ║
    echo ║ This could indicate:
    echo ║   • Commit message contains invalid characters
    echo ║   • No changes were actually staged
    echo ║   • Permission issues in the repository
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║ ✓ Commit created successfully
echo ║

:: Push changes
echo ║ • Pushing to remote repository...

:: Check if target branch is different from current branch
if not "!TARGET_BRANCH!"=="!CURRENT_BRANCH!" (
    echo ║ • Creating and checking out new branch: !TARGET_BRANCH!
    git checkout -b !TARGET_BRANCH! >nul 2>&1
    
    if errorlevel 1 (
        echo ║ ✗ ERROR: Failed to create/switch to branch
        echo ║
        echo ╚══════════════════════════════════════════════════════════════════════════╝
        echo.
        pause
        exit /b 1
    )
    
    echo ║ ✓ Branch created/switched successfully
    echo ║
)

git push -u origin !TARGET_BRANCH! 2>&1

if errorlevel 1 (
    echo ║
    echo ╠══════════════════════════════════════════════════════════════════════════╣
    echo ║ ⚠  WARNING: Git push encountered an error!
    echo ║
    echo ║ Possible causes:
    echo ║   • No internet connection
    echo ║   • Merge conflicts exist
    echo ║   • Remote branch has diverged from local
    echo ║   • Insufficient permissions
    echo ║   • Remote repository is not accessible
    echo ║
    echo ║ Please resolve the issue and try pushing manually using:
    echo ║   git push --force-with-lease
    echo ║
    echo ╚══════════════════════════════════════════════════════════════════════════╝
    echo.
    pause
    exit /b 1
)

echo ║ ✓ Changes pushed successfully
echo ║

::==============================================================================
:: SUCCESS MESSAGE
::==============================================================================

echo ║
echo ╠══════════════════════════════════════════════════════════════════════════╣
echo ║  ✓ SUCCESS!
echo ║
echo ║  Your changes have been successfully committed and pushed to:
echo ║  Branch: %CURRENT_BRANCH%
echo ║
echo ╚══════════════════════════════════════════════════════════════════════════╝
echo.

pause
exit /b 0
