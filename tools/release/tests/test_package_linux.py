import hashlib
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from package_linux import package
from fixtures import installation, SHA


class LinuxPackageTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.stage = installation(self.root / 'stage', 'linux-x86_64')

    def test_archive_preserves_permissions_and_internal_symlinks(self):
        (self.stage / 'bin/ElaWidgetToolsExample').chmod(0o755)
        (self.stage / 'lib/alias.so').symlink_to('libElaWidgetTools.so')
        asset = package(self.stage, self.root / 'out', '2.0.0', SHA)
        with tarfile.open(asset) as archive:
            prefix = 'ElaWidgetTools-2.0.0-linux-x86_64/'
            self.assertTrue(archive.getmember(prefix + 'bin/ElaWidgetToolsExample').mode & 0o111)
            self.assertEqual(archive.getmember(prefix + 'lib/alias.so').linkname, 'libElaWidgetTools.so')
        info = json.loads((asset.parent / 'package-info-linux-x86_64.json').read_text())
        self.assertEqual(info['sha256'], hashlib.sha256(asset.read_bytes()).hexdigest())
        self.assertEqual(info['source_commit'], SHA)

    def test_missing_and_empty_library(self):
        lib = self.stage / 'lib/libElaWidgetTools.so'
        lib.write_bytes(b'')
        with self.assertRaises(ValueError):
            package(self.stage, self.root / 'out', '2.0.0', SHA)
        lib.unlink()
        with self.assertRaises(ValueError):
            package(self.stage, self.root / 'out', '2.0.0', SHA)

    def test_escape_symlink_rejected(self):
        (self.stage / 'escape').symlink_to(self.root)
        with self.assertRaises(ValueError):
            package(self.stage, self.root / 'out', '2.0.0', SHA)

    def test_output_inside_stage_rejected(self):
        with self.assertRaises(ValueError):
            package(self.stage, self.stage / 'out', '2.0.0', SHA)
