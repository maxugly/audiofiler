#!/bin/bash

# Configuration
WORKFLOW_FILENAME="build.yml" # Adjust if your workflow file has a different name
SLEEP_INTERVAL=10

echo "🔍 Checking for latest build for current commit..."

# Get current commit hash
CURRENT_COMMIT=$(git rev-parse HEAD)
echo "   Commit: $CURRENT_COMMIT"

# Function to get run status
get_run_status() {
  gh run list --workflow "$WORKFLOW_FILENAME" --commit "$CURRENT_COMMIT" --limit 1 --json status,conclusion --jq '.[0]'
}

echo "⏳ Waiting for build to complete..."

while true; do
  RUN_INFO=$(get_run_status)
  
  # Check if a run exists
  if [ -z "$RUN_INFO" ] || [ "$RUN_INFO" == "null" ]; then
    echo "   No build found for this commit yet. Waiting..."
    sleep "$SLEEP_INTERVAL"
    continue
  fi

  STATUS=$(echo "$RUN_INFO" | jq -r .status)
  CONCLUSION=$(echo "$RUN_INFO" | jq -r .conclusion)

  if [ "$STATUS" == "completed" ]; then
    if [ "$CONCLUSION" == "success" ]; then
      echo "✅ Build success! Proceeding to fetch..."
      fetcher
      exit 0
    else
      echo "❌ Build failed with conclusion: $CONCLUSION"
      exit 1
    fi
  else
    echo "   Status: $STATUS. Waiting..."
    sleep "$SLEEP_INTERVAL"
  fi
done
