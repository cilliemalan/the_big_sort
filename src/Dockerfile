FROM ubuntu

COPY * /build/

RUN apt-get update && apt-get install -y build-essential && \
    cd /build && make && cp sorter / && cd / && rm -rf /build && \
    apt-get purge -y --auto-remove build-essential && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/sorter"]
