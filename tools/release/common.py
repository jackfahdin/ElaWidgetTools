"""发布公共约定：版本、输入校验和摘要；不包含平台命令或 GitHub 操作。"""
import argparse
import hashlib
import json
from pathlib import Path
import re

QT_VERSION = '6.12.0'
PLATFORMS = {'linux-x86_64': '.tar.gz', 'windows-x86_64': '.zip', 'macos-arm64': '.zip'}
VERSION_RE = r'(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)'


def project_version(source):
    content = (Path(source) / 'ElaWidgetTools/CMakeLists.txt').read_text(encoding='utf-8-sig')
    match = re.search(r'project\(\s*ElaWidgetTools\s+VERSION\s+(' + VERSION_RE + r')\s', content)
    if not match:
        raise ValueError('组件库 CMake 项目中没有有效的 X.Y.Z 版本')
    return match[1]


def validate_identity(version, label, sha):
    if not re.fullmatch(VERSION_RE, version):
        raise ValueError('版本必须为 X.Y.Z，数字不能含多余前导零')
    if label not in {version, 'Continuous-Build'}:
        raise ValueError('资产标签必须等于版本号或 Continuous-Build')
    if not re.fullmatch(r'[0-9a-f]{40}', sha):
        raise ValueError('源提交必须为完整的 40 位 SHA')


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def write_json(path, value):
    Path(path).write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def require_files(stage, names):
    for name in names:
        path = Path(stage) / name
        if not path.is_file() or path.stat().st_size == 0:
            raise ValueError(f'安装包缺少必需文件或文件为空：{name}')


def validate_tree(stage, allow_symlinks=True):
    stage = Path(stage).resolve()
    if not stage.is_dir():
        raise ValueError('安装目录不存在')
    for path in stage.rglob('*'):
        if path.is_symlink():
            # framework 和共享库允许相对链接；拒绝泄漏主机路径或打包外部文件。
            target = path.readlink()
            if (not allow_symlinks or target.is_absolute() or not path.exists()
                    or not path.resolve().is_relative_to(stage)):
                raise ValueError(f'安装树中存在无效或越界符号链接：{path}')
        elif not path.is_file() and not path.is_dir():
            raise ValueError(f'安装树中存在特殊文件：{path}')


def prepare_package(stage, output, label, sha, platform):
    stage, output = Path(stage).resolve(), Path(output).resolve()
    if output.is_relative_to(stage):
        raise ValueError('输出目录不能位于安装树内部')
    require_files(stage, ['include/ElaWidgetTools/ElaAppBar.h', 'LICENSE',
                          'lib/cmake/ElaWidgetTools/ElaWidgetToolsConfig.cmake',
                          'lib/cmake/ElaWidgetTools/ElaWidgetToolsTargets.cmake',
                          'lib/cmake/ElaWidgetTools/ElaWidgetToolsConfigVersion.cmake'])
    # 读取真正安装的 CMake 版本，防止源代码版本和所打包的旧安装树不一致。
    content = (stage / 'lib/cmake/ElaWidgetTools/ElaWidgetToolsConfigVersion.cmake').read_text()
    match = re.search(r'set\(PACKAGE_VERSION "(' + VERSION_RE + r')"\)', content)
    if not match:
        raise ValueError('安装树中没有有效的 CMake 包版本')
    version = match[1]
    validate_identity(version, label, sha)
    output.mkdir(parents=True, exist_ok=True)
    name = f'ElaWidgetTools-{label}-{platform}'
    asset = output / (name + PLATFORMS[platform])
    if asset.exists():
        raise ValueError(f'输出文件已存在，请使用空的输出目录：{asset}')
    return stage, asset, name, version


def finish_package(asset, platform, version, label, sha):
    if not asset.is_file() or not asset.stat().st_size:
        raise ValueError('没有生成有效归档')
    write_json(asset.parent / f'package-info-{platform}.json', {
        'schema': 1, 'platform': platform, 'version': version, 'label': label,
        'source_commit': sha, 'qt_version': QT_VERSION,
        'name': asset.name, 'size': asset.stat().st_size, 'sha256': digest(asset),
    })
    return asset


def package_cli(packager):
    parser = argparse.ArgumentParser(description='检查平台安装树并生成发布包和元数据')
    parser.add_argument('--stage', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--label', required=True)
    parser.add_argument('--sha', required=True)
    args = parser.parse_args()
    print(packager(args.stage, args.output, args.label, args.sha))
