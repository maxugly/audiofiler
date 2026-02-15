---
description: Download the latest Google Action build (replaces local build)
---

This workflow downloads the latest Google Action build using the `fetcher` command. This is useful for testing without building locally.
It now waits for the build corresponding to the current commit to complete before fetching.

1. Wait for build and download
   // turbo
   .agent/scripts/fetch_latest_build.sh
