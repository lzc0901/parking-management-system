import hashlib
import io
import subprocess
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUTPUT = ROOT / 'juli18-review-candidate-v2.zip'
MANIFEST = ROOT / 'juli18-MANIFEST-v2.sha256'
COMMIT = 'df3a71477b3635c93c64305f2e941f0ccb5f88b2'
TREE = 'f805a58a4b143902fedd0b28d0586e59ad32ab1d'
handoff = {
    'README_提交與審查.md': ('build/juli15-package/README_提交與審查.md', '6e41c09578a01d69e716f5991c3457ba4a741c4a7de585e7759c5ec009f29034'),
    'docs/juli11_user_guide_zh.md': ('juli11_user_guide_zh.md', '1eeae766d5816c48c31a1a39a06eee5cf68fef9bb18ff6a8f4b7a1f5cb62f5c9'),
    'docs/juli12_development_report_zh.md': ('juli12_development_report_zh.md', 'e4fb6102ecff30565d7b7f69d47690b8cc7ead9646980d0618755248aceff802'),
    'docs/juli12_development_report_zh.pdf': ('juli12_development_report_zh.pdf', 'b89371016feed37d4a084e04831a40814104322755cac9ca2566745f9662aac7'),
    'docs/juli12_development_report_zh.html': ('juli12_development_report_zh.html', '1a34cf18b41e14831e50ea6e0694805b84870135f498ded74c762aee70cb0bf1'),
    'docs/juli12_flowchart_1_overall.svg': ('juli12_flowchart_1_overall.svg', '6788e4e21d20ff82f0b815f3de02572eae777e9806c68f318c065f8093f2ee65'),
    'docs/juli12_flowchart_2_crud.svg': ('juli12_flowchart_2_crud.svg', '60b39ca3a65227da9e5ee9a25860e1e69f6d5c21e94956d0addd01752f98dd97'),
    'docs/juli12_flowchart_3_fee.svg': ('juli12_flowchart_3_fee.svg', 'f47948213ede2c0b4184c381e1b444848ceebf2c0a6728d7a50477e91c3764ab'),
    'docs/juli12_flowchart_4_storage.svg': ('juli12_flowchart_4_storage.svg', '0ce1bb5eea9f3b29b24e671de4696d791ccfbf43426121b4e98de86db3090101'),
    'docs/juli12_flowcharts.mmd': ('juli12_flowcharts.mmd', 'ce1a1a5a86393f1b6b33a3af54c01139fa14db697cd2a6ae1fd79c57d4aa157f'),
    'docs/juli12_page_inspection_evidence.md': ('juli12_page_inspection_evidence.md', '09acb7dc69b171ddde20ff0b02076b16e4c2d3b289a80c9bd48b38901a05ea8e'),
    'docs/juli13_demo_script_and_checklist_zh.md': ('juli13_demo_script_and_checklist_zh.md', '88d9a9bb673c20b29125abf74283f885742af1c87da37dd55a9e77b366519829'),
    'docs/juli13_rehearsal_evidence.md': ('juli13_rehearsal_evidence.md', 'eb24aa531074a86a81a140209705b6c411812335f971829bd95d5a54f9c5d222'),
    'docs/juli14-coverage.md': ('juli14-coverage.md', '0dcb0b9e6a9ebe5da2a726d337fe1b23648ac49113a398d98a31fd7ddd5c9b7f'),
    'docs/juli14_viewing_guide_and_alignment_zh.md': ('juli14_viewing_guide_and_alignment_zh.md', '00194c5f3fb6fcceabc9ab6f3e8cfe958886dce426859d92a29250d7d7ac1106'),
    'evidence/juli11_walkthrough_evidence.md': ('juli11_walkthrough_evidence.md', 'd20cbcc3219edae74419d691ede478c03df65590972d1f823d4d3e63535e8c65'),
    'evidence/requirements_map.md': ('build/juli15-package/evidence/requirements_map.md', 'c670c377b4601b374bfb44a0c0a5c6a878d914988e732fa25c29b080bd1b3c48'),
    'evidence/review_and_tests.md': ('build/juli15-package/evidence/review_and_tests.md', 'c6ab553f85802199a03c79e445f0102479f6767eefe56f8b84a8ad51bb0b82a1'),
}

def sha(data):
    return hashlib.sha256(data).hexdigest()

if OUTPUT.exists() or MANIFEST.exists():
    raise SystemExit('Candidate or manifest already exists; refusing to overwrite')
assert subprocess.check_output(['git', 'rev-parse', COMMIT + '^{tree}'], text=True).strip() == TREE
files = {}
for dest, (src, expected) in handoff.items():
    data = (ROOT / src).read_bytes()
    if sha(data) != expected:
        raise ValueError(f'Handoff hash mismatch: {src}')
    files[dest] = data
paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', COMMIT], text=True).splitlines()
for path in paths:
    files['source/e06/' + path] = subprocess.check_output(['git', 'show', COMMIT + ':' + path])
files['source/LICENSE'] = files['source/e06/LICENSE']
with zipfile.ZipFile(ROOT / 'juli15-review-package.zip') as old:
    for name in ('source/e06-source.zip', 'video/juli14-demo.mp4'):
        files[name] = old.read(name)
assert sha(files['source/e06-source.zip']) == '844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e'
files['video/juli14-timestamps.txt'] = (ROOT / 'juli14-timestamps.txt').read_bytes()
files['check_report_assets.py'] = (ROOT / 'check_report_assets.py').read_bytes()
manifest = ''.join(f'{sha(data)}  {name}\n' for name, data in sorted(files.items())).encode('utf-8')
files['MANIFEST.sha256'] = manifest
with zipfile.ZipFile(OUTPUT, 'x', zipfile.ZIP_DEFLATED) as result:
    for name, data in sorted(files.items()):
        result.writestr(name, data)
MANIFEST.write_bytes(manifest)
print(f'candidate={OUTPUT.name} sha256={sha(OUTPUT.read_bytes())} files={len(files)}')
print(f'manifest={MANIFEST.name} sha256={sha(manifest)}')
