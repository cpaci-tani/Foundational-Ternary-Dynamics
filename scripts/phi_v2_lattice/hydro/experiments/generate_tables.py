"""Generate, verify, and write the collision table; print its SHA256 to pin in channels.py."""
import json
import sys
import time

from .. import tables as T


def main():
    started = time.time()
    table = T.build_table()
    report = T.verify_table(table)
    if not (report["involution"] and report["mass_conserved"] and report["momentum_conserved"] and report["equivariant"]):
        print(json.dumps(report), file=sys.stderr)
        raise SystemExit("table verification failed; do not pin")
    path = T.write_table(table)
    report["path"] = str(path)
    report["elapsed_seconds"] = time.time() - started
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
