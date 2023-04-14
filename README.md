# cSocket

Socket use examples for `select`, `poll` and `epoll`.

## Build and Run on Ubuntu 20.04

### Build
```shell
cmake -H. -Bbuild
cmake --build build -j
```

### Run

#### select test
```shell
./build/select # for socket select server
./build/client # for socket client
```

#### poll test
```shell
./build/poll # for socket poll server
./build/client # for socket client
```

#### select test
```shell
./build/epoll  # for socket epoll server
./build/client # for socket client
```

Reference document [here](https://www.jianshu.com/p/722819425dbd/).
