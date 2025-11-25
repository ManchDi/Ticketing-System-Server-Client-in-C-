# Ticketing System (Server & Client) in C++

This project implements a simple **multi-threaded ticket booking system** in C++ using TCP sockets. The system consists of a server that manages seat availability and multiple clients that can book seats either manually or automatically.

---

## Features

* **Server**

  * Handles multiple clients concurrently using `pthread`.
  * Tracks seat availability in a configurable rows × seats grid.
  * Displays a real-time seating map on the server console.
  * Automatically terminates when all seats are sold.
  * Supports dynamic seat queries and booking requests.

* **Client**

  * Connects to the server over TCP.
  * Two modes:

    * **Manual:** User inputs row and seat number.
    * **Automatic:** Random seat selection for simulation/testing.
  * Supports optional INI configuration for IP, port, and timeout.
  * Gracefully handles server disconnections.

---

## Prerequisites

* Linux/macOS (Windows may require minor modifications)
* C++17 compiler (`g++`)
* POSIX threads support

---

## Compilation

```bash
# Compile the server
g++ -std=c++17 -pthread -o server server.cpp

# Compile the client
g++ -std=c++17 -o client client.cpp
```

---

## Usage

### **Server**

```bash
# Run server with default 10x10 seating
./server

# Or specify custom rows and seats (max 40 each)
./server 15 20
```

### **Client**

```bash
# Manual mode
./client manual

# Automatic mode
./client automatic

# Automatic mode with INI config
./client automatic config.ini
```

**INI File Example (`config.ini`)**:

```
IP = 127.0.0.1
Port = 5437
Timeout = 5
```

---

## Commands (Client)

* `row seat` — e.g., `3 5` to book row 3, seat 5.
* `disc` — disconnect from the server.
* `quit` / `exit` — exit the client program.

---

## Notes

* Maximum supported grid size is 40 rows × 40 seats.
* Server automatically locks seats to prevent double-booking.
* Automatic client mode simulates random bookings every 0.5 seconds.

---

## License

This project is open-source for educational and experimental use.

---

## Author

Dmitrii Manchurak
