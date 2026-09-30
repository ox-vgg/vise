# HTTP request limits and framing

The native HTTP server limits each request before dispatching it to a project:

| VISE setting | Default | Meaning |
| --- | --- | --- |
| `http-max-header-bytes` | `16384` | Maximum request line and header bytes, including the final CRLF delimiter |
| `http-max-body-bytes` | `104857600` | Maximum declared body bytes (100 MiB) |

Set these positive decimal byte counts in `vise_settings.txt`, or pass them to
`vise-cli`, for example `--http-max-body-bytes=209715200`. Older settings files
without these keys use the defaults. Invalid, zero and overflowing values fail
server initialization. Project endpoints can impose their own smaller image or
payload limits; these server settings do not raise endpoint limits.

Headers may arrive over multiple TCP reads. The server waits for the complete
header, checks the limits and validates framing before passing a body to the
existing request parser. Oversized headers or declared bodies receive 413;
malformed/duplicate `Content-Length` and unsupported `Transfer-Encoding` receive
400. A POST/PUT without `Content-Length` is treated as an empty body. Bodies for
other methods are not supported. Content-Length, Content-Type and Expect field
names are matched without regard to case, and surrounding optional whitespace
is normalized for the existing parser.

The server continues to handle one request per connection and closes after the
final response. It does not implement chunked request bodies or pipelining.
Bytes in a read beyond the declared body length are rejected rather than stored.
These are byte limits, not idle timeouts: stalled clients can still hold a
connection open. Applications exposed to slow/untrusted clients should configure
timeouts in their HTTP proxy.

`Expect: 100-continue` receives a complete interim response when the body is
still outstanding. A client that already supplied its full body receives the
final response directly. Interim and final asynchronous writes use separate
buffers and are serialized.

CTest runs a native header/parser regression and, when a Python 3 interpreter
and the `vise-cli` target are available, a socket integration test. The latter
starts only its own loopback server, creates projects in a temporary store, and
stops only that child process. It covers fragmented headers/bodies, binary parser
payloads, lower-case fields, default/custom limits, malformed framing, empty
POSTs and waiting/fast `100-continue` clients.
