FROM ubuntu

COPY * /build

RUN apt update && apt install -y build-essential && \
    cd /build && make && cp sorter / && cd / && rm -rf /build && \
    apt purge -y --auto-remove build-essential && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/sorter"]
