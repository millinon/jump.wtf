#!/bin/sh

echo "Pre-commit starting"

git diff --cached --name-only -- '*.hh' | while read ifname; do

if [ -f "${ifname}" ] ; then
    echo "Checking ${ifname}..."
    if hh_client check "${ifname}"; then
        echo 'pass'
    else
        echo 'Check failed'
        exit 1
    fi

    echo "Linting ${ifname}..."
    if hh_client check --lint "${ifname}"; then
        echo 'pass'
    else
        echo 'Lint died'
        exit 1
    fi

    echo "Formatting ${ifname}..."
    hh_format -i "${ifname}"
    git add "${ifname}"
fi
done

    if git diff --cached --name-only -- '*api*.hh' >/dev/null 2>&1; then
        echo "Validating API"
        if hhvm bin/validate-api.hh; then
            echo "API passed"
        else
            echo "API failed"
            exit 1
        fi
fi

echo "Pre-commit done"
