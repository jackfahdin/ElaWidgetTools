"""汇总本次工作流的三个平台包，拒绝缺包、混合提交和摘要不一致。"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import shutil
from common import PLATFORMS, QT_VERSION, digest, validate_identity, write_json


def assemble(artifacts, output, version, label, sha, mode, run_url):
    validate_identity(version, label, sha)
    if mode not in {'stable', 'nightly'} or (mode == 'nightly') != (label == 'Continuous-Build'):
        raise ValueError('发布模式与资产标签不一致')
    artifacts, output = Path(artifacts), Path(output)
    if output.exists() and any(output.iterdir()):
        raise ValueError('汇总目录必须为空')
    output.mkdir(parents=True, exist_ok=True)
    expected = set()
    entries = []
    for platform, extension in PLATFORMS.items():
        matches = list(artifacts.rglob(f'package-info-{platform}.json'))
        if len(matches) != 1:
            raise ValueError(f'平台元数据缺失或重复：{platform}')
        metadata = matches[0]
        info = json.loads(metadata.read_text(encoding='utf-8'))
        name = f'ElaWidgetTools-{label}-{platform}{extension}'
        for key, value in {'schema': 1, 'platform': platform, 'version': version, 'label': label,
                           'source_commit': sha, 'qt_version': QT_VERSION, 'name': name}.items():
            if info.get(key) != value:
                raise ValueError(f'平台元数据不匹配：{platform}/{key}')
        asset = metadata.parent / name
        if (not asset.is_file() or not asset.stat().st_size or asset.is_symlink()
                or info['size'] != asset.stat().st_size or info['sha256'] != digest(asset)):
            raise ValueError(f'平台资产缺失或摘要不匹配：{name}')
        expected.update({asset, metadata})
        entries.append({key: info[key] for key in ('platform', 'name', 'size', 'sha256')})
    if {p for p in artifacts.rglob('*') if p.is_file()} != expected:
        raise ValueError('发现不属于本次三个平台包的额外文件')
    for path in expected:
        if not path.name.startswith('package-info-'):
            shutil.copyfile(path, output / path.name)
    result = {
        'schema': 1, 'version': version, 'label': label, 'mode': mode,
        'tag': 'continuous-build' if mode == 'nightly' else f'v{version}',
        'source_commit': sha, 'qt_version': QT_VERSION, 'workflow_run': run_url,
        'built_at': datetime.now(timezone.utc).isoformat(), 'assets': entries,
    }
    write_json(output / f'release-info-{label}.json', result)
    checksums = ''.join(f'{digest(p)}  {p.name}\n' for p in sorted(output.iterdir()))
    (output / f'SHA256SUMS-{label}.txt').write_text(checksums, encoding='utf-8')
    return result


def verified_files(directory, info):
    directory = Path(directory)
    version, label, sha = info['version'], info['label'], info['source_commit']
    validate_identity(version, label, sha)
    if info.get('schema') != 1 or info.get('qt_version') != QT_VERSION:
        raise ValueError('不支持的元数据或 Qt 版本')
    if info.get('mode') not in {'nightly', 'stable'}:
        raise ValueError('无效发布模式')
    nightly = info['mode'] == 'nightly'
    if nightly != (label == 'Continuous-Build') or info['tag'] != ('continuous-build' if nightly else f'v{version}'):
        raise ValueError('发布模式、标签和版本不一致')
    entries = info['assets']
    if len(entries) != 3 or {e['platform'] for e in entries} != set(PLATFORMS):
        raise ValueError('必须包含三个不同平台')
    for entry in entries:
        name = f'ElaWidgetTools-{label}-{entry["platform"]}{PLATFORMS[entry["platform"]]}'
        if entry['name'] != name:
            raise ValueError('资产名称不符合平台约定')
        path = directory / name
        if (not path.is_file() or not entry['size'] or path.stat().st_size != entry['size']
                or digest(path) != entry['sha256']):
            raise ValueError(f'发布资产已变化：{name}')
    metadata_name = f'release-info-{label}.json'
    if json.loads((directory / metadata_name).read_text(encoding='utf-8')) != info:
        raise ValueError('发布元数据不一致')
    checksum_name = f'SHA256SUMS-{label}.txt'
    names = {e['name'] for e in entries} | {metadata_name}
    checksums = ''.join(f'{digest(directory / n)}  {n}\n' for n in sorted(names))
    if (directory / checksum_name).read_text(encoding='utf-8') != checksums:
        raise ValueError('校验和清单已变化')
    names.add(checksum_name)
    files = sorted(directory.iterdir())
    if {p.name for p in files} != names or any(p.is_symlink() or not p.is_file() for p in files):
        raise ValueError('发布目录含额外文件或符号链接')
    return files


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('artifacts', 'output', 'version', 'label', 'sha', 'mode', 'run-url'):
        parser.add_argument('--' + option, required=True)
    args = parser.parse_args()
    assemble(args.artifacts, args.output, args.version, args.label, args.sha, args.mode, args.run_url)
