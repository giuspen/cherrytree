#!/bin/bash
set -e

git rev-list --count $(git describe --tags --abbrev=0)..HEAD
