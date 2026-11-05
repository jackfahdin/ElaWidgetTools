"""Linux x86_64 开发包；不把本机 Qt SDK 路径包装成通用运行环境。"""
import tarfile
from common import finish_package, package_cli, prepare_package, require_files, validate_tree


def package(stage, output, label, sha):
    validate_tree(stage)
    require_files(stage, ['lib/libElaWidgetTools.so', 'bin/ElaWidgetToolsExample'])
    stage, asset, name, version = prepare_package(stage, output, label, sha, 'linux-x86_64')
    # tar 保留执行权限和共享库符号链接，解压后保持 CMake 安装布局。
    with tarfile.open(asset, 'w:gz', dereference=False) as archive:
        archive.add(stage, arcname=name)
    return finish_package(asset, 'linux-x86_64', version, label, sha)


if __name__ == '__main__':
    package_cli(package)
