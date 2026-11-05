"""Windows x64 安装树：MSVC 导入库、头文件和已部署的 Example。"""
import zipfile
from common import finish_package, package_cli, prepare_package, require_files, validate_tree


def package(stage, output, label, sha):
    validate_tree(stage, allow_symlinks=False)
    # Qt 的 CMake 部署使用根目录 plugins/，通过 bin/qt.conf 定位。
    require_files(stage, ['bin/ElaWidgetTools.dll', 'lib/ElaWidgetTools.lib',
                          'bin/ElaWidgetToolsExample.exe', 'bin/Qt6Core.dll',
                          'bin/qt.conf', 'plugins/platforms/qwindows.dll'])
    stage, asset, name, version = prepare_package(stage, output, label, sha, 'windows-x86_64')
    with zipfile.ZipFile(asset, 'w', zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(stage.rglob('*')):
            if path.is_file():
                archive.write(path, f'{name}/{path.relative_to(stage).as_posix()}')
    return finish_package(asset, 'windows-x86_64', version, label, sha)


if __name__ == '__main__':
    package_cli(package)
