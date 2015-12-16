#!/bin/sh

echo "Pre-commit starting"

git diff --cached --name-only -- '*.hh'|while read ifname; do
    echo "Checking ${ifname}..."
    if hh_client check "${ifname}"; then
        echo 'Check failed, aborting commit'
        git stash pop -q
        exit 1
    fi

    echo "Linting ${ifname}..."
    if hh_client check --lint "${ifname}"; then
        echo 'Lint died, aborting commit'
        git stash pop -q
        exit 1
    fi
    
    echo "Formatting ${ifname}..."
    hh_format "${ifname}"
    git add "${ifname}"
done

echo "Pre-commit done"
