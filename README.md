# mu-http

A minimalist HTTP server written in C99.

## Building

This project uses make for building. Run `make` for building the binary, `make
install` to install it to your `PATH` (under `~/.local/bin` by default, change
`PREFIX`), `make clean` to clean up the project and restore it to a pre-build
state. By default, builds don't include debug symbols; specify `debug` to
include them (e.g. `make debug`, `make debug install`).

## Limitations

Currently, the web server only supports HTTP version 1.1, and no advanced
functionalities such as chunked requests or responses.

## Configuration

Server configuration is split into two parts. Internal constants are specified
in `src/config.h` with reasonable default values. These are:
- `CLIENT_MAX_BODY_LENGTH`: the maximum length of a request body sent by a
  client, in bytes
- `CLIENT_MAX_HEADERS_LENGTH`: the maximum length of the request headers sent
  by a client, in bytes.
- `CLIENT_MAX_HEADERS`: the maximum number of request headers sent by a client.
- `CLIENT_MAX_URI_LENGTH`: the maximum length of a request URI, as included in
  the request target line, in bytes.
- `CLIENT_BUFFER_SIZE`: the buffer size for receiving client requests,
  excluding the body, in bytes. This value is set to a fixed value, broken down
  as follows:
    - 4 bytes for the method (GET) and whitespace
    - `CLIENT_MAX_URI_LENGTH` bytes for request target
    - 9 bytes for whitespace and HTTP version (`HTTP/1.1`)
    - 2 bytes for CRLF
    - `CLIENT_MAX_HEADERS_LENGTH` bytes for headers
    - 2 bytes for CRLF
- `LISTEN_PORT`: the port number on which to listen for connections
- `LISTEN_BACKLOG`: the size of the socket's backlog

Request handling configuration is done in `config`, which at build time
generates a `src/config.c` file defining the request handling logic.

Such a config file might look like this:
```
begin
    prefix(/files, file)
    exact(/echo, logreq, echo)
end
```

If the request target is of the form `/files/...` (i.e. `/files` is a *prefix*
of the target), use the `file` handler, which responds with the requested file
inside the `files/` directory. For example, if the request target is
`/files/index.html`, the server will respond with the contents of
`files/index.html`.

If the request target is *exactly* `/echo`, the request handlers `logreq`
(which logs details about the request) and `echo` (which echoes back the
request body) are used, in this order.

Note that the server executes all matching rules, e.g. `prefix(/files, file)`,
`exact(/files/foo, logreq)` would run *both* the `file` and the `logreq`
handler if the request target is `/files/foo`.

For a list of available request handlers, look for the `handler_...` functions
in `src/handlers.c`. The available rule filters currently are:
- `prefix`: for a prefix match
- `exact`: for an exact match
