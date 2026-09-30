"""Exercise HTTP framing on an isolated native server using only temporary data."""
import argparse
from contextlib import contextmanager
from pathlib import Path
import os
import socket
import subprocess
import tempfile
import time


def receive(connection, until_header=False):
    result = bytearray()
    while True:
        chunk = connection.recv(65536)
        if not chunk:
            return bytes(result)
        result.extend(chunk)
        if until_header and b'\r\n\r\n' in result:
            return bytes(result)


def exchange(port, chunks):
    with socket.create_connection(('127.0.0.1', port), timeout=10) as connection:
        connection.settimeout(10)
        for chunk in chunks:
            connection.sendall(chunk)
        return receive(connection)


def expect_status(response, expected):
    first = response.split(b'\r\n', 1)[0]
    if not first.startswith(('HTTP/1.1 ' + str(expected) + ' ').encode()):
        raise AssertionError('Unexpected response: ' + repr(response[:200]))


@contextmanager
def server(cli, www, limits=()):
    with tempfile.TemporaryDirectory(prefix='vise-http-limits-') as temporary:
        root = Path(temporary)
        projects = root / 'projects'
        assets = root / 'assets'
        projects.mkdir()
        assets.mkdir()
        with socket.socket() as reservation:
            reservation.bind(('127.0.0.1', 0))
            port = reservation.getsockname()[1]
        command = [str(cli), '--cmd=web-ui', '--http-address=127.0.0.1',
                   '--http-port=' + str(port), '--http-worker=2',
                   '--vise-home-dir=' + str(root), '--vise-project-dir=' + str(projects),
                   '--vise-asset-dir=' + str(assets), '--http-www-dir=' + str(www), *limits]
        with (root / 'server.log').open('wb') as log:
            process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT,
                                       creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
            try:
                deadline = time.monotonic() + 30
                while True:
                    if process.poll() is not None:
                        raise RuntimeError('Native server exited: ' + (root / 'server.log').read_text(errors='replace'))
                    try:
                        expect_status(exchange(port, [b'GET /settings HTTP/1.1\r\n\r\n']), 200)
                        break
                    except OSError:
                        if time.monotonic() >= deadline:
                            raise
                        time.sleep(0.05)
                yield port, projects
            finally:
                if process.poll() is None:
                    process.terminate()
                    process.wait(timeout=15)


def create(port, projects, name, fragmented=False, expect=False, coalesced=False):
    payload = ('pname=' + name).encode()
    head = (b'POST /_project_create HTTP/1.1\r\ncontent-length:\t' + str(len(payload)).encode() +
            b' \t\r\ncontent-type: application/x-www-form-urlencoded\r\n')
    if expect:
        head += b'expect:\t100-Continue \t\r\n'
    head += b'\r\n'
    if expect and not coalesced:
        with socket.create_connection(('127.0.0.1', port), timeout=10) as connection:
            connection.settimeout(10)
            connection.sendall(head)
            interim = receive(connection, until_header=True)
            if interim != b'HTTP/1.1 100 Continue\r\n\r\n':
                raise AssertionError('Invalid interim response: ' + repr(interim))
            connection.sendall(payload)
            response = receive(connection)
    elif fragmented:
        with socket.create_connection(('127.0.0.1', port), timeout=10) as connection:
            connection.settimeout(10)
            # Separate writes with a delay force delimiters to cross TCP reads.
            for chunk in [head[:-3], head[-3:-1], head[-1:], payload[:3], payload[3:]]:
                connection.sendall(chunk)
                time.sleep(0.01)
            response = receive(connection)
    else:
        response = exchange(port, [head + payload])
    expect_status(response, 303)
    if response.count(b'HTTP/1.1 ') != 1:
        raise AssertionError('Duplicate final/interim response: ' + repr(response[:300]))
    if not (projects / name / 'data' / 'conf.txt').is_file():
        raise AssertionError('Normal form POST did not create a project')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--vise-cli', type=Path, required=True)
    parser.add_argument('--www', type=Path, required=True)
    args = parser.parse_args()
    with server(args.vise_cli.resolve(), args.www.resolve()) as (port, projects):
        expect_status(exchange(port, [b'GET /settings HTTP/1.1\r\nX: ' + b'a' * 17000]), 413)
        expect_status(exchange(port, [b'POST /_project_create HTTP/1.1\r\nContent-Length: 104857601\r\n\r\n']), 413)
        for fields in [b'Content-Length: -1', b'Content-Length: 1junk',
                       b'Content-Length: 1\r\ncontent-length: 1', b'Transfer-Encoding: chunked']:
            expect_status(exchange(port, [b'POST /_project_create HTTP/1.1\r\n' + fields + b'\r\n\r\n']), 400)
        expect_status(exchange(port, [b'POST /_project_create HTTP/1.1\r\nContent-Length: 1\r\n\r\nab']), 400)
        with socket.create_connection(('127.0.0.1', port), timeout=10) as connection:
            connection.settimeout(10)
            connection.sendall(b'POST /_project_create HTTP/1.1\r\nContent-Length: 5\r\n\r\nab')
            time.sleep(0.01)
            connection.sendall(b'cdef')
            expect_status(receive(connection), 400)
        for fields in [b'', b'Content-Length: 0\r\n']:
            response = exchange(port, [b'POST /_project_create HTTP/1.1\r\n' + fields + b'\r\n'])
            if b'project name is missing' not in response:
                raise AssertionError('Zero-length POST did not reach the normal handler')
        create(port, projects, 'fragmented', fragmented=True)
        create(port, projects, 'continue_wait', expect=True)
        create(port, projects, 'continue_coalesced', expect=True, coalesced=True)
        create(port, projects, 'ordinary')
        # A fast client may send the body before the interim write finishes.
        for number in range(20):
            payload = ('pname=fast' + str(number)).encode()
            head = (b'POST /_project_create HTTP/1.1\r\nExpect: 100-continue\r\nContent-Length: ' +
                    str(len(payload)).encode() + b'\r\n\r\n')
            response = exchange(port, [head, payload])
            if response.startswith(b'HTTP/1.1 100 Continue\r\n\r\n'):
                response = response[len(b'HTTP/1.1 100 Continue\r\n\r\n'):]
            expect_status(response, 303)
            if response.count(b'HTTP/1.1 ') != 1:
                raise AssertionError('Fast-client response buffers were mixed')
        if {path.name for path in projects.iterdir()} != {
            'fragmented', 'continue_wait', 'continue_coalesced', 'ordinary'
        } | {'fast' + str(number) for number in range(20)}:
            raise AssertionError('Rejected requests changed the project store')
    with server(args.vise_cli.resolve(), args.www.resolve(),
                ['--http-max-header-bytes=512', '--http-max-body-bytes=128']) as (port, projects):
        expect_status(exchange(port, [b'GET /settings HTTP/1.1\r\nX: ' + b'a' * 513 + b'\r\n\r\n']), 413)
        expect_status(exchange(port, [b'POST /_project_create HTTP/1.1\r\nContent-Length: 129\r\n\r\n']), 413)
        payload = b'pname=body128&padding=' + b'a' * (128 - len(b'pname=body128&padding='))
        expect_status(exchange(port, [b'POST /_project_create HTTP/1.1\r\nContent-Length: 128\r\n\r\n' + payload]), 303)
        if not (projects / 'body128' / 'data' / 'conf.txt').is_file():
            raise AssertionError('Body at the configured limit was rejected')
    print('Native HTTP framing, fragmentation, Expect, and configurable limits passed')


if __name__ == '__main__':
    main()
