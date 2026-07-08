@echo off
setlocal enabledelayedexpansion

set "target_branch=updates-branch"

echo [GIT] Preparing to upload changes to branch: %target_branch%

:: Check if there are changes
git status --short
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Not a git repository or git not found.
    pause
    exit /b 1
)

:: Add all changes
git add .

:: Commit changes
set /p "commit_msg=Enter commit message: "
if "!commit_msg!"=="" set "commit_msg=Update from gitUpload.bat"
git commit -m "!commit_msg!"

:: Check if branch exists, if not create it
git branch | findstr /C:"%target_branch%" >nul
if %ERRORLEVEL% neq 0 (
    echo [GIT] Creating new branch: %target_branch%
    git checkout -b %target_branch%
) else (
    echo [GIT] Switching to branch: %target_branch%
    git checkout %target_branch%
    git merge main
)

:: Push to remote
echo [GIT] Pushing to origin %target_branch%...
git push origin %target_branch%

if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Upload completed successfully.
) else (
    echo [ERROR] Upload failed.
)

pause
