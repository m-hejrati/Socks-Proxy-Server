# Socks Proxy Server

Simple proxy server in C++ language using boost::asio library and socks protocol,
based on [boost_socks5 project](https://github.com/philave/boost_socks5)

## Prerequisites

we use asio and some other library of boost in this project, for installing boost you can write this command:
```bash
sudo apt-get install libboost-all-dev
```

For logging, we use spdlog , you can clone it from below github repo:
```bash
git clone https://github.com/gabime/spdlog.git
```

## Installation

compile project with CMake:
```bash
mkdir build
cd build
cmake ..
make
````

and run:
```bash
./main
````

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.
