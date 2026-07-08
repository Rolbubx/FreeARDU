#!/bin/bash

TARGET_BRANCH="updates-branch"

echo "[GIT] Preparing to upload changes to branch: $TARGET_BRANCH"

# Check if git repository
if ! git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    echo "[ERROR] Not a git repository."
    exit 1
fi

# Add all changes
git add .

# Commit changes
echo -n "Enter commit message (default: Update from gitUpload.sh): "
read commit_msg
if [ -z "$commit_msg" ]; then
    commit_msg="Update from gitUpload.sh"
fi
git commit -m "$commit_msg"

# Check if branch exists
if git show-ref --verify --quiet "refs/heads/$TARGET_BRANCH"; then
    echo "[GIT] Switching to branch: $TARGET_BRANCH"
    git checkout "$TARGET_BRANCH"
    git merge main
else
    echo "[GIT] Creating new branch: $TARGET_BRANCH"
    git checkout -b "$TARGET_BRANCH"
fi

# Push to remote
echo "[GIT] Pushing to origin $TARGET_BRANCH..."
git push origin "$TARGET_BRANCH"

if [ $? -eq 0 ]; then
    echo "[SUCCESS] Upload completed successfully."
else
    echo "[ERROR] Upload failed."
fi
