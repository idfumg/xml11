#!/usr/bin/env bash

run_tests() {
    echo "Running tests..."
    docker build -f DockerfileTests -t xml11 . && docker run --rm xml11
}

main() {
    run_tests
    return 0
}

main $@
