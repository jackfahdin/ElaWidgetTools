import sys
from pathlib import Path
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import project_version, validate_identity
from fixtures import SHA


class VersionTest(unittest.TestCase):
    def test_current_project_version(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'ElaWidgetTools').mkdir()
            (root / 'ElaWidgetTools/CMakeLists.txt').write_text(
                'project(\n    ElaWidgetTools\n    VERSION 12.34.56\n    LANGUAGES CXX)')
            self.assertEqual(project_version(root), '12.34.56')

    def test_valid_identity(self):
        validate_identity('2.0.0', '2.0.0', SHA)
        validate_identity('2.0.0', 'Continuous-Build', SHA)

    def test_invalid_identity(self):
        for version, label, sha in [('02.0.0', '02.0.0', SHA), ('2.0.0', 'v2.0', SHA),
                                    ('2.0.0', '../bad', SHA), ('2.0.0', '2.1.0', SHA),
                                    ('2.0.0', '2.0.0', 'short')]:
            with self.subTest(label=label), self.assertRaises(ValueError):
                validate_identity(version, label, sha)
