# Emotelib

## Overview

This server is designed to automate the processing of images via URL configurations and to serve them via a basic HTTP server. It handles image downloading, MD5-based naming for caching, format conversion, and basic HTTP responses.

## Features

- **Image Downloading**: Downloads images from URLs specified in a configuration file.
- **MD5 Hash Naming**: Uses MD5 hashing to uniquely identify and store images to avoid duplicates.
- **Image Conversion**: Converts downloaded images to a specified format and stores them for quick retrieval.
- **HTTP Server**: Serves images over HTTP on a specified port and provides a directory listing as an index page.

## Prerequisites

Ensure you have the following installed on your system:
- GCC Compiler or equivalent for compiling the project.
- cURL for handling URL downloads (`libcurl`).
- ImageMagick for image processing.
- OpenSSL for MD5 hashing (`libssl`).

## Installation

1. **Clone the repository**:

```sh
git clone [repository-url]
cd [project-directory]
```
2. **Compile the source code and run**

```
make
./main
```
## Usage

Prepare a configuration file (`/opt/config.txt`) with URLs of images to be processed:

```
http://example.com/image1.jpg
http://example.com/image2.jpg
```

Start the server and access it through a web browser or a client at http://localhost:8080.
Navigate to http://localhost:8080/ to view the index page listing all available images.
