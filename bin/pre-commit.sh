#!/bin/sh

GIT_DIR=`git rev-parse --show-toplevel`

cd "$GIT_DIR"

git stash -q --keep-index

echo "Linting..."

touch "$GIT_DIR/.hhconfig"

if hh_client check --lint "${GIT_DIR}/www/"
then
    echo "Formatting..."
    hh_format --root "${GIT_DIR}/www/"
    git stash pop -q
else
    git stash pop -q
    exit 1
fi

