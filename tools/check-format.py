"""按仓库固定版本检查被 Git 跟踪的源码和 CMake 文件，不修改文件。"""

from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
files = subprocess.check_output(
    ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"], cwd=root
).decode().split("\0")
cpp = sorted({p for p in files if Path(p).suffix in {".h", ".cpp"}})
cmake = sorted({p for p in files if p.endswith(("CMakeLists.txt", ".cmake", ".cmake.in"))})
commands = [
    ["clang-format", "--dry-run", "--Werror", *cpp],
    ["cmake-format", "-c", ".cmake-format.yaml", "--check", *cmake],
]
failed = False
for command in commands:
    failed |= subprocess.run(command, cwd=root, check=False).returncode != 0
sys.exit(1 if failed else 0)
