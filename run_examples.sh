#!/usr/bin/env bash

run_examples() {
    echo "Running examples..."
    docker build -f DockerfileExamples -t xml11 . && docker run --rm xml11
    return 0
}

main() {
    run_examples
    return 0
}

main $@