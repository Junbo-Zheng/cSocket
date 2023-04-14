# cSocket

Socket examples for `select`, `poll`, `epoll`.

## Build and Run on Ubuntu 20.04

### Build
```shell
cmake -H. -Bbuild
cmake --build build -j
```

### Run
```shell
./build/select # for socket select server
./build/client # for socket client
```
