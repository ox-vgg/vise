"""Cold-start a new isolated VISE process and verify an external self-query."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import shutil
import subprocess
import tempfile
import time
import urllib.error
import urllib.request

from merge_vise_indexes import _project_rows


def verify(cli, www, project, query, expected_filename, cycles=2, timeout=300):
    _, rows, _ = _project_rows(project)
    expected_filenames = [row[0] for row in rows]
    if expected_filename not in expected_filenames:
        raise ValueError('Smoke query target is not indexed')
    snapshots = []
    query_bytes = query.read_bytes()
    deadline = time.monotonic() + timeout
    for _ in range(cycles):
        with tempfile.TemporaryDirectory(prefix='vise-native-verify-') as temporary:
            with socket.socket() as listener:
                listener.bind(('127.0.0.1', 0))
                port = listener.getsockname()[1]
            command = [str(cli), '--cmd=serve-project', '--http-address=127.0.0.1',
                       '--http-port=' + str(port), '--http-worker=2',
                       '--http-www-dir=' + str(www), '--vise-home-dir=' + temporary,
                       project.name + ':' + str(project / 'data/conf.txt')]
            with (Path(temporary) / 'server.log').open('wb') as log:
                process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT,
                                           creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
                origin = 'http://127.0.0.1:' + str(port)
                opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
                def request(path, data=None):
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        raise TimeoutError('Native verification exceeded the overall deadline')
                    req = urllib.request.Request(origin + '/' + project.name + '/' + path, data=data)
                    with opener.open(req, timeout=min(60, remaining)) as response:
                        return response.read()
                try:
                    startup_deadline = min(deadline, time.monotonic() + 30)
                    while True:
                        if process.poll() is not None:
                            raise RuntimeError('Isolated VISE process exited before startup')
                        try:
                            status = json.loads(request('index_status?response_format=json'))
                            break
                        except urllib.error.URLError:
                            if time.monotonic() >= startup_deadline:
                                raise
                            time.sleep(0.1)
                    if not status.get('INDEX_STATUS', status).get('index_is_done'):
                        raise RuntimeError('Native index is not ready: ' + json.dumps(status))
                    filenames = []
                    for start in range(0, len(expected_filenames), 1000):
                        end = min(start + 1000, len(expected_filenames))
                        page = json.loads(request('filelist?mode=all&start=' + str(start) + '&end=' + str(end) + '&response_format=json'))
                        if page.get('PNAME') != project.name or page.get('FLIST_SIZE') != len(expected_filenames):
                            raise RuntimeError('Native filelist returned another project or size')
                        names = page.get('FLIST_FILENAME', [])
                        if len(names) != end - start:
                            raise RuntimeError('Native filelist page was truncated')
                        filenames.extend(names)
                    if filenames != expected_filenames:
                        raise RuntimeError('Native filelist differs from the on-disk dataset')
                    listing = {'PNAME': project.name, 'FLIST_SIZE': len(filenames), 'FLIST_FILENAME': filenames}
                    features = request('_extract_image_features', query_bytes)
                    if not features:
                        raise RuntimeError('Native query has no features')
                    matches = json.loads(request('_search_using_features', features))
                    if matches.get('PNAME') != project.name or expected_filename not in [hit.get('filename') for hit in matches.get('RESULT', [])]:
                        raise RuntimeError('Native self-query did not retrieve its own indexed image')
                    snapshots.append({'feature_sha256': hashlib.sha256(features).hexdigest(),
                                      'feature_bytes': len(features), 'matches': matches['RESULT'],
                                      'filelist': listing})
                except BaseException:
                    log.flush()
                    # Keep failure evidence after the isolated temporary HOME exits.
                    shutil.copy2(Path(temporary) / 'server.log', project / 'native-verification-error.log')
                    raise
                finally:
                    if process.poll() is None:
                        process.terminate()
                        try:
                            process.wait(timeout=15)
                        except subprocess.TimeoutExpired:
                            process.kill()
                            process.wait(timeout=15)
    if any(item['feature_sha256'] != snapshots[0]['feature_sha256'] or item['matches'] != snapshots[0]['matches'] for item in snapshots):
        raise RuntimeError('Repeated native cold loads returned different results')
    return {'result': 'passed', 'cold_loads': cycles, 'project': project.name,
            **snapshots[0]}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--vise-cli', type=Path, required=True)
    parser.add_argument('--www', type=Path, required=True)
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--query', type=Path, required=True)
    parser.add_argument('--expected-filename', required=True)
    parser.add_argument('--timeout', type=int, default=300)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    result = verify(args.vise_cli.resolve(), args.www.resolve(), args.project.resolve(), args.query.resolve(), args.expected_filename, timeout=args.timeout)
    if args.output:
        args.output.write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps({key: value for key, value in result.items() if key not in ['matches', 'filelist']}))

if __name__ == '__main__':
    main()
