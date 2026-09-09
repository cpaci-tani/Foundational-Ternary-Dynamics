"""CLI file-publication regressions; no physical recovery assertions."""
import json
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from phi_v2_lattice import native_codec as N, staged as P, state as S

ROOT = Path(__file__).resolve().parents[3]


def linux(path):
    value = str(Path(path).resolve()).replace('\\', '/')
    return '/mnt/' + value[0].lower() + value[2:] if os.name == 'nt' else value


@pytest.fixture(scope='module', params=('native', 'cuda'))
def backend(request):
    if request.param == 'native':
        configured = os.environ.get('FTD_STRICT_NATIVE_CLI')
        suffix = '.exe' if os.name == 'nt' else ''
        path = Path(configured or ROOT / 'engine/build_strict_native' / ('ftd_strict_cli' + suffix))
        required = configured is not None or os.environ.get('FTD_STRICT_NATIVE_REQUIRED') == '1'
        prefix = [str(path)]
    else:
        path = Path(os.environ.get('FTD_STRICT_CUDA_CLI', ROOT / 'engine/build_strict_cuda/ftd_strict_cuda_cli'))
        required = os.environ.get('FTD_STRICT_CUDA_REQUIRED') == '1'
        prefix = ['wsl', '-d', 'Ubuntu-22.04', '--', linux(path)] if os.name == 'nt' else [str(path)]
    if not path.is_file():
        if required: pytest.fail('required publication-test binary missing: ' + str(path))
        pytest.skip('optional publication-test binary missing: ' + str(path))
    if request.param == 'cuda':
        try:
            device = subprocess.run(prefix + ['--device'], capture_output=True, text=True, timeout=30)
        except (OSError, subprocess.TimeoutExpired) as error:
            if required: pytest.fail('required CUDA device check failed: ' + str(error))
            pytest.skip('optional CUDA device unavailable: ' + str(error))
        if device.returncode:
            if required: pytest.fail('required CUDA device unavailable: ' + device.stderr)
            pytest.skip('optional CUDA device unavailable: ' + device.stderr)
        assert json.loads(device.stdout)['backend'] == 'cuda_device_kernels'
    return request.param, prefix


def invoke(backend, source, output, events):
    name, prefix = backend
    paths = [linux(p) if name == 'cuda' else str(p) for p in (source, output, events)]
    return subprocess.run(prefix + [paths[0], paths[1], '0', paths[2]], capture_output=True, text=True, timeout=30)


@pytest.fixture
def files(tmp_path):
    source, output, events = (tmp_path / name for name in ('input.bin', 'output.bin', 'events.json'))
    source.write_bytes(N.encode(P.initialize(S.blank(3))))
    output.write_bytes(b'previous-checkpoint')
    events.write_bytes(b'previous-events')
    return source, output, events


def test_unopenable_events_keeps_previous_checkpoint(backend, files, tmp_path):
    source, output, events = files
    before = source.read_bytes(), output.read_bytes(), events.read_bytes()
    result = invoke(backend, source, output, tmp_path / 'missing-parent' / 'events.json')
    assert result.returncode != 0
    assert (source.read_bytes(), output.read_bytes(), events.read_bytes()) == before
    assert not list(tmp_path.glob('.ftd-publication-*'))


@pytest.mark.parametrize('alias', ('input-output', 'input-events', 'output-events', 'hardlink-input', 'hardlink-outputs'))
def test_alias_paths_rejected_without_changing_any_file(backend, files, tmp_path, alias):
    source, output, events = files
    if alias == 'input-output': output = source
    elif alias == 'input-events': events = source
    elif alias == 'output-events': events = output
    elif alias == 'hardlink-input':
        output = tmp_path / 'input-alias.bin'; os.link(source, output)
    else:
        events = tmp_path / 'output-alias.json'; os.link(output, events)
    before = {p: p.read_bytes() for p in tmp_path.iterdir() if p.is_file()}
    result = invoke(backend, source, output, events)
    assert result.returncode != 0
    assert {p: p.read_bytes() for p in before} == before
    assert not list(tmp_path.glob('.ftd-publication-*'))


@pytest.mark.parametrize('existing', (False, True))
def test_success_publishes_complete_pair_and_preserves_input(backend, files, tmp_path, existing):
    source, output, events = files
    if not existing:
        output.unlink(); events.unlink()
    before = source.read_bytes()
    result = invoke(backend, source, output, events)
    assert result.returncode == 0, result.stderr
    assert source.read_bytes() == output.read_bytes() == before
    assert json.loads(events.read_text()) == []
    assert not list(tmp_path.glob('.ftd-publication-*'))


HARNESS = r'''
#include "cli_publication.h"
#include <iostream>
int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    if (argc != 4) return 9;
    const fs::path folder = argv[1];
    const bool existing = std::string(argv[2]) == "1";
    const int failure = std::stoi(argv[3]);
    const auto input = folder / "input", output = folder / "output", events = folder / "events";
    auto write = [](const fs::path& p, const std::string& s) { std::ofstream f(p, std::ios::binary); f << s; };
    auto read = [](const fs::path& p) { std::ifstream f(p, std::ios::binary); return std::string(std::istreambuf_iterator<char>(f), {}); };
    write(input, "input-unchanged");
    if (existing) { write(output, "old-output"); write(events, "old-events"); }
    int calls = 0;
    bool rejected = false;
    try {
        ftd::strict::cli::detail::publish_pair(input, output, events, std::vector<std::uint8_t>{1,2,3}, "new-events",
          [&](const fs::path& from, const fs::path& to) {
              if (++calls == failure) throw std::runtime_error("injected publication rename failure");
              fs::rename(from, to);
          });
    } catch (const std::exception& error) {
        if (std::string(error.what()) != "injected publication rename failure") { std::cerr << error.what(); return 8; }
        rejected = true;
    }
    if (!rejected || calls != failure || read(input) != "input-unchanged") return 1;
    if (existing && (read(output) != "old-output" || read(events) != "old-events")) return 2;
    if (!existing && (fs::exists(output) || fs::exists(events))) return 3;
    for (const auto& entry : fs::directory_iterator(folder))
        if (entry.path().filename().string().find(".ftd-publication-") == 0) return 4;
    std::cout << "rollback PASS\n";
}
'''


@pytest.fixture(scope='module')
def rollback_harness(tmp_path_factory):
    compiler = shutil.which('g++')
    if not compiler:
        if os.environ.get('FTD_STRICT_PUBLICATION_HARNESS_REQUIRED') == '1':
            pytest.fail('g++ required for header-only publication exception harness')
        pytest.skip('optional g++ publication exception harness is unavailable')
    folder = tmp_path_factory.mktemp('publication-harness')
    source, binary = folder / 'harness.cpp', folder / ('harness.exe' if os.name == 'nt' else 'harness')
    source.write_text(HARNESS)
    built = subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT / 'engine/strict'),
                            str(source), '-o', str(binary)], capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return binary


@pytest.mark.parametrize('existing,failure', ((True, 1), (True, 2), (True, 3), (True, 4), (False, 1), (False, 2)))
def test_each_publication_rename_failure_rolls_back(rollback_harness, tmp_path, existing, failure):
    result = subprocess.run([str(rollback_harness), str(tmp_path), str(int(existing)), str(failure)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr
    assert result.stdout.strip() == 'rollback PASS'
