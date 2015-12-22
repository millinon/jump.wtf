#!/bin/sh

echo "Pre-commit starting"

pass=1

git diff --cached --name-only -- '*.hh' | while read ifname; do

if [ -f "${ifname}" ]; then

    echo "Checking ${ifname}..."
    if hh_client check "${ifname}"; then
        echo 'pass'
    else
        echo 'Check failed'
        pass=0
    fi

    echo "Linting ${ifname}..."
    if hh_client check --lint "${ifname}"; then
        echo 'pass'
    else
        echo 'Lint died'
        pass=0
    fi

    echo "Formatting ${ifname}..."
    hh_format -i "${ifname}"
    git add "${ifname}"
fi
done

if [ "$pass" == "1" ]; then
    if git diff --cached --name-only -- '*api*.hh' >/dev/null 2>&1; then
        echo "Validating API"
        if hhvm bin/validate-api.hh; then
            echo "API passed"
            pass=1
        else
            echo "API failed"
            pass=0
        fi
    fi
fi

if [ "$pass" != "1" ]; then
    echo "Pre-commit aborting"
    exit 1
fi

echo "Pre-commit done"
