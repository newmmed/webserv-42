# Project Name: Webserv

## Table of Contents

- [Project Overview](#project-overview)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Configuration](#configuration)
- [Development](#development)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [License](#license)

***

## Project Overview

A brief description of the project, its goals, and the technologies used.

> _Example:_
> Webserv is a lightweight HTTP/1.0 server implemented in C++98. It is designed to handle basic web requests, serve static files, and provide a foundation for learning about network programming and server architecture.

***

## Features

- [ ] HTTP/1.0 protocol support
- [ ] Static file serving
- [ ] Configurable server settings
- [ ] Logging and error handling
- [ ] Modular codebase for easy extension
- [ ] (Add more features as you implement them)

***

## Getting Started

### Prerequisites

- C++98 compatible compiler (e.g., g++)
- `make` utility


### Build Instructions

```sh
make
```


### Running the Server

```sh
./webserv [config_file]
```


***

## Usage

- Place your static files in the designated directory (see configuration).
- Access the server via your browser or tools like `curl`:

```sh
curl http://localhost:8080/
```

- (Add more usage examples as features are added)

***

## Configuration

- The server can be configured via a configuration file (describe the format and options here).
- _Example:_

```
port: 8080
root: ./www
log: ./logs/server.log
```

- (Expand this section as your configuration options grow)

***

## Development

### Code Style

- Follow the [CODING_STANDARDS.md](./README_DOCS/CODING_STANDARDS.md) for code style and commit requirements.
- Use the provided pre-commit hooks for consistency.


### Project Structure

```
webserv/
├── src/
├── include/
├── www/
├── tests/
├── Makefile
├── README.md
└── NORM.md
```


### Adding Features

- List new features in the [Features](#features) section.
- Document any new configuration options or usage changes.

***

## Changelog

- Use this section to track major changes, bug fixes, and new features.
- _Example:_
    - **2025-10-03:** Project initialized
    - **2025-10-10:** Added static file serving
    - (Continue updating as the project evolves)

***

## Contributing

- Fork the repository and create a feature branch.
- Follow the code style and commit message guidelines.
- Open a pull request and request review from teammates.
- All code must pass the pre-commit checks and be reviewed before merging.
