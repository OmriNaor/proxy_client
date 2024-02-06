# C Project: CProxy

## Introduction

CProxy is a lightweight, command-line HTTP proxy tool written in C. It allows users to request web resources either directly from the web server or from a local cache if available.

## Features

CProxy offers a range of functionalities, including:

- Parsing and validating URLs.
- Handling HTTP GET requests.
- Connecting to web servers using TCP sockets.
- Receiving and parsing HTTP responses.
- Caching web resources locally.
- Serving cached resources when available to reduce network traffic.
- Optional flag to open the retrieved web resource in the default web browser.

## Components

- `cproxy.c`: The main program file that includes functions for parsing URLs, handling network communication, caching, and interacting with the filesystem.
- `cproxy.h`: The header file containing the declarations of the functions and structures used by `cproxy.c`.

## How It Works

The program operates as follows:

1. Parses the provided URL and validates its structure.
2. Checks if the requested resource is available in the local cache.
3. If not cached:
   - Establishes a TCP connection to the target web server.
   - Sends an HTTP GET request for the specified resource.
   - Receives the HTTP response and caches the resource locally.
4. If the `-s` flag is provided, opens the retrieved resource in the default web browser.

## Compilation and Execution

To compile and run the project, follow these steps:

1. Clone the repository or download the source code.
2. Navigate to the project directory.
3. Compile the project using a C compiler (e.g., `gcc` or `clang`): `gcc cproxy.c -o cproxy`
4. Run the compiled executable: `./cproxy <URL> [-s]`

## Remarks:

- CProxy handles only HTTP GET requests and is intended for educational purposes.
- The project demonstrates socket programming, HTTP protocol handling, and basic caching mechanisms in C.
- The `-s` flag is optional and used to open the fetched resource in the default web browser.

## Getting Started

1. Ensure you have a C compiler installed on your system.
2. Compile the project using the provided command.
3. Run the compiled program with a valid HTTP URL as an argument.

