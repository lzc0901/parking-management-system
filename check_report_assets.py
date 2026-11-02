#!/usr/bin/env python3
"""Check local Markdown image references in a directory or ZIP archive."""

import argparse
import re
import sys
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path, PurePosixPath
from urllib.parse import unquote, urlsplit


IMAGE = re.compile(r'!\[[^\]]*\]\(\s*(?:<([^>]+)>|([^\s)]+))(?:\s+[^)]*)?\)')


def check(files):
    references = missing = invalid = 0
    for name in sorted(files):
        if not name.lower().endswith('.md'):
            continue
        content = files[name]().decode('utf-8-sig')
        for match in IMAGE.finditer(content):
            target = match.group(1) or match.group(2)
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            references += 1
            parts = list(PurePosixPath(name).parent.parts)
            for part in PurePosixPath(unquote(parsed.path)).parts:
                if part == '..':
                    if parts:
                        parts.pop()
                    else:
                        parts.append('..')
                elif part not in ('.', ''):
                    parts.append(part)
            resolved = '/'.join(parts)
            if resolved not in files:
                missing += 1
                print(f'MISSING {name}: {target} -> {resolved}')
            elif resolved.lower().endswith('.svg'):
                try:
                    ET.fromstring(files[resolved]())
                except ET.ParseError as exc:
                    invalid += 1
                    print(f'INVALID SVG {resolved}: {exc}')
    print(f'image_refs={references} missing={missing} invalid_svg={invalid}')
    return 1 if missing or invalid or references == 0 else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', type=Path, help='extracted package directory or ZIP')
    args = parser.parse_args()
    if args.path.is_dir():
        files = {p.relative_to(args.path).as_posix(): p.read_bytes
                 for p in args.path.rglob('*') if p.is_file()}
        return check(files)
    with zipfile.ZipFile(args.path) as archive:
        names = [entry.filename for entry in archive.infolist() if not entry.is_dir()]
        if len(names) != len(set(names)):
            raise ValueError('ZIP contains duplicate paths')
        return check({name: lambda name=name: archive.read(name) for name in names})


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, UnicodeError, ValueError, zipfile.BadZipFile) as exc:
        print(f'ERROR: {exc}', file=sys.stderr)
        sys.exit(2)
