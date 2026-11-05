from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from package_macos import validate_stage, package
from fixtures import installation, SHA


class MacPackageTest(unittest.TestCase):
    def test_bundle_and_framework_links(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            stage = installation(root / 'stage', 'macos-arm64')
            framework = stage / 'ElaWidgetToolsExample.app/Contents/Frameworks/QtCore.framework'
            (framework / 'Versions/A').mkdir(parents=True)
            (framework / 'Versions/A/QtCore').write_bytes(b'framework')
            (framework / 'Versions/Current').symlink_to('A')
            validate_stage(stage)
            # Linux 只检查打包命令与结构；真实 ditto 归档及解压在 macOS CI 验证。
            with patch('package_macos.subprocess.run') as run:
                def archive(command, **kwargs):
                    self.assertIn('--keepParent', command)
                    self.assertTrue((Path(command[-2]) / 'ElaWidgetToolsExample.app').is_dir())
                    Path(command[-1]).write_bytes(b'zip')
                run.side_effect = archive
                package(stage, root / 'out', '2.0.0', SHA)
            (stage / 'ElaWidgetToolsExample.app/Contents/MacOS/ElaWidgetToolsExample').unlink()
            with self.assertRaises(ValueError):
                validate_stage(stage)
