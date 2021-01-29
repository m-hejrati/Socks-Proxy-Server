# Socks Proxy Server

Simple proxy server in C++ language using boost::asio library and socks protocol.

### Prerequisites

we use asio and some other library of boost in this project, for installing boost you can write this command:
```bash
sudo apt-get install libboost-all-dev
```

For logging, we use spdlog , you can clone it from below github repo:
```bash
git clone https://github.com/gabime/spdlog.git
```

### Compile and Run

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
