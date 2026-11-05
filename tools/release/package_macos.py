"""macOS ARM64 发布包：由 ditto 保存应用权限、资源和 framework 链接。"""
from pathlib import Path
import shutil
import subprocess
import tempfile
from common import finish_package, package_cli, prepare_package, require_files, validate_tree


def validate_stage(stage):
    validate_tree(stage)
    require_files(stage, ['lib/libElaWidgetTools.dylib',
                          'ElaWidgetToolsExample.app/Contents/Info.plist',
                          'ElaWidgetToolsExample.app/Contents/MacOS/ElaWidgetToolsExample'])


def package(stage, output, label, sha):
    validate_stage(stage)
    stage, asset, name, version = prepare_package(stage, output, label, sha, 'macos-arm64')
    # 临时目录提供清晰的归档根目录，不能用普通 ZIP 写入来展开 framework 链接。
    with tempfile.TemporaryDirectory(prefix='ela-macos-package-') as tmp:
        source = Path(tmp) / name
        shutil.copytree(stage, source, symlinks=True)
        subprocess.run(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent',
                        str(source), str(asset)], check=True)
    return finish_package(asset, 'macos-arm64', version, label, sha)


if __name__ == '__main__':
    package_cli(package)
