# The Big Sort

This repo contains information, the source for utilities and an example for The Big Sort dev day.


## What you need to make

For The Big Sort dev portion you need to write a program that sorts a file. The file will
be plaintext with many many lines of text. You will need to produce a file where each
line of text is alphanumerically greater than the line before it. The sort must be
case-insensitive.

Furthermore, the program you write will need to be containerized with docker. (Remember that
a docker container runs like an executable). When your container is run, the paths for
the source and destination dirs will be mounted, and your program will be executed with
the name of the source and destination file.

For example, your container will be invoked (kind of) like so:
```sh
docker pull <your/image>
docker run -it -v /src:/src:ro -v /dst:/dst <your/image> /src/file.dat /dst/file.dat
```

The time it takes for your program to run will be recorded. Once the program is done, the
destination file will be checked. If the file is sorted, the sort will be considered
successful.

Note: Inside your container everything will be read only except the destination directory.

Note: to see the actual command used to run the container, see run-sort.sh.


## Utilities
To build and install the utilities, run this in the src directory:
```sh
make
sudo make install
```

### UTILITY: `generate`

Generates a file with random stuff.

    Usage: `generate <amount in gigabytes>`

The program will print to stdout so run it e.g. like this:
```sh
# will generate 3Gb of random data and store in /tmp/wut.dat
generate 3 > /tmp/wut.dat
```

### UTILITY: `check-sorted`

checks if a file is sorted.

    Usage: `check-sorted <sorted file>`

The program will print status to stderr and return 0 if the file is sorted.


## Example

In the directory example you'll find an example docker image. It uses sort(1) to sort the
file. You can use it as an example of how your dockerfile should be built. Your dockerfile
will be run much like this example when it is benchmarked.

How to build:
Execute this in the example directory:
```sh
docker build -t sortexample .
```

How to run:
For example, to sort 1 gb, run this:
```sh
mkdir -p /tmp/src /tmp/dst
generate 1 > /tmp/src/file.dat
time docker run -it \
    -v /tmp/src:/src:ro -v /tmp/dst:/dst \
    sortexample \
    /src/file.dat \
    /dst/file.dat
```

The output should be:
```sh
sort -f -o /dst/file.dat /src/file.dat
done

real    0m15.008s
user    0m0.008s
sys     0m0.008s
```

Here we see the file was sorted in 15 seconds.

Note: your results will be skewed if `/tmp` has `tmpfs` mounted
