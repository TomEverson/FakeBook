# FakeBook 🚀

FakeBook is a Facebook clone written in C and HTMX. It's built from scratch with a focus on simplicity and learning, using only standard libraries and sockets for communication.

---

## Features ✨

- **Concurreny**: Handle Multiple Web Request Using Posix Thread
- **HTTP 1.1**: Implemented HTTP 1.1 From Scratch Using TCP
- **Template Serving**: Can Serve HTML Template From Router
- **User Authentication**: Sign Up, and Log Out From Scratch
- **Built In Database**: Sqlite Database is built in for storing data
- **Parsing Request**: Parsing Request Header in C
- **Encryption Layer**: Encryption from scratch using XOR Encrypion ( Not Secure )
- **Request Routing**: Request Routing
- **Cookie Parsing**: Cookie Parsing Algorithm From Scratch

---

## Getting Started 🏁

### Prerequisites

- A C compiler (like GCC, and check test.sh for more information)

### Installation and Running

1.  **Clone the repository:**

    ```bash
    git clone [https://github.com/TomEverson/FakeBook.git](https://github.com/TomEverson/FakeBook.git)
    cd FakeBook
    ```

2.  **Run the server:**

    ```bash
    chmod +x test.sh
    ./test.sh
    ```

    The server will start on port 4221.

3.  **Open your browser and navigate to:**
    ```
    http://localhost:4221
    ```

---

## Disclaimer ⚠️

**Note:** This project is for educational purposes only. It is **Not Secure** and should not be used in a production environment. The primary goal is to demonstrate how to build a web application with a C socket without relying on external dependencies.
