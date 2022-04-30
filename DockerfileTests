FROM silkeh/clang:latest

RUN apt-get update && apt-get install -y libgtest-dev libxml2 && rm -rf /var/lib/apt/lists/* && ln -s /usr/lib/x86_64-linux-gnu/libxml2.so.2 /usr/lib/x86_64-linux-gnu/libxml2.so

COPY . /usr/src/xml11
WORKDIR /usr/src/xml11

ENV CXX=clang++
ENV CC=clang

RUN make test

CMD ["./test"]