"""平台打包测试使用最小安装树，不需要跨平台编译。"""
from pathlib import Path

SHA = 'a' * 40


def installation(root, platform):
    root = Path(root)
    files = {
        'include/ElaWidgetTools/ElaAppBar.h': b'header',
        'lib/cmake/ElaWidgetTools/ElaWidgetToolsConfig.cmake': b'config',
        'lib/cmake/ElaWidgetTools/ElaWidgetToolsTargets.cmake': b'targets',
        'lib/cmake/ElaWidgetTools/ElaWidgetToolsConfigVersion.cmake': b'set(PACKAGE_VERSION "2.0.0")',
        'LICENSE': b'MIT License',
    }
    files.update({
        'linux-x86_64': {'lib/libElaWidgetTools.so': b'library', 'bin/ElaWidgetToolsExample': b'app'},
        'windows-x86_64': {'bin/ElaWidgetTools.dll': b'library', 'lib/ElaWidgetTools.lib': b'import',
                           'bin/ElaWidgetToolsExample.exe': b'app', 'bin/Qt6Core.dll': b'qt',
                           'plugins/platforms/qwindows.dll': b'plugin',
                           'bin/qt.conf': b'[Paths]\nPrefix=..\nPlugins=plugins\n'},
        'macos-arm64': {'lib/libElaWidgetTools.dylib': b'library',
                        'ElaWidgetToolsExample.app/Contents/MacOS/ElaWidgetToolsExample': b'app',
                        'ElaWidgetToolsExample.app/Contents/Info.plist': b'plist'},
    }[platform])
    for name, content in files.items():
        path = root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    return root
