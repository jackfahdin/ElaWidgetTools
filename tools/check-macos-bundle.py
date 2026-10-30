"""在 macOS CI 中检查部署后的依赖，并脱离 Qt SDK 环境启动示例。"""

from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

bundle = Path(sys.argv[1]).resolve()
executable = bundle / "Contents/MacOS/ElaWidgetToolsExample"
if sys.platform != "darwin" or not executable.is_file():
    sys.exit("需要 macOS 和已经部署的 ElaWidgetToolsExample.app")

mach_o = {b"\xcf\xfa\xed\xfe", b"\xfe\xed\xfa\xcf", b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca"}
binaries = []
for path in bundle.rglob("*"):
    if path.is_file() and not path.is_symlink():
        with path.open("rb") as file:
            if file.read(4) in mach_o:
                binaries.append(path)
for binary in binaries:
    dependencies = subprocess.check_output(["otool", "-L", str(binary)], text=True)
    for line in dependencies.splitlines():
        if not line.startswith("\t"):
            continue
        dependency = line.strip().split(" (", 1)[0]
        if dependency.startswith("/") and not dependency.startswith(("/usr/lib/", "/System/Library/")):
            sys.exit(f"应用仍依赖外部库：{binary}: {dependency}")
    commands = subprocess.check_output(["otool", "-l", str(binary)], text=True)
    for rpath in re.findall(r"cmd LC_RPATH\s+cmdsize \d+\s+path (.*?) \(offset", commands):
        if rpath.startswith("/") and not rpath.startswith(("/usr/lib/", "/System/Library/")):
            sys.exit(f"应用仍含外部库搜索路径：{binary}: {rpath}")

env = dict(os.environ)
for name in list(env):
    if name.startswith(("QT_", "QML", "DYLD_")):
        env.pop(name)
env["DYLD_FALLBACK_LIBRARY_PATH"] = ""
log = Path("macos-smoke.log").resolve()
with tempfile.TemporaryDirectory(prefix="ela-macos-smoke-") as cwd, log.open("w") as output:
    # 隔离 .app，防止从原安装前缀的 lib/ 或构建目录找到未部署的组件库。
    relocated = Path(cwd) / bundle.name
    shutil.copytree(bundle, relocated, symlinks=True)
    executable = relocated / "Contents/MacOS/ElaWidgetToolsExample"
    try:
        result = subprocess.run(
            [str(executable), "--smoke-test"], cwd=cwd, env=env,
            stdout=output, stderr=subprocess.STDOUT, timeout=30, check=False,
        )
    except subprocess.TimeoutExpired:
        sys.exit("示例在 30 秒内没有正常退出，见 macos-smoke.log")
if result.returncode:
    sys.exit(f"示例启动或退出失败：{result.returncode}，见 macos-smoke.log")
print(f"检查了 {len(binaries)} 个 Mach-O 文件；部署后的示例正常启动并退出。")
