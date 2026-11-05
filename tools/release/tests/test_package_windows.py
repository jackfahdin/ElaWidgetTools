from pathlib import Path
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from package_windows import package
from fixtures import installation, SHA


class WindowsPackageTest(unittest.TestCase):
    def test_archive_and_missing_runtime(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            stage = installation(root / 'stage', 'windows-x86_64')
            asset = package(stage, root / 'out', 'Continuous-Build', SHA)
            with zipfile.ZipFile(asset) as archive:
                self.assertIn('ElaWidgetTools-Continuous-Build-windows-x86_64/bin/ElaWidgetTools.dll', archive.namelist())
            (stage / 'bin/platforms/qwindows.dll').unlink()
            with self.assertRaises(ValueError):
                package(stage, root / 'other', 'Continuous-Build', SHA)
