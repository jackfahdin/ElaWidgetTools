"""用内存 GitHub 替身验证发布状态，资产摘要使用真实文件。"""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from assemble import assemble, verified_files
from common import PLATFORMS, digest, write_json
from prepare import resolve_inputs
from publish import publish
from fixtures import SHA


class FakeGitHub:
    def __init__(self):
        self.tags = {'v2.0.0': SHA}
        self.current = None
        self.uploads = {}
        self.fail_upload = False
        self.mutations = []

    def tag(self, tag):
        return self.tags.get(tag)

    def release(self, tag):
        return copy.deepcopy(self.current)

    def create(self, tag, sha, nightly):
        self.current = {'id': 1, 'tag_name': tag, 'target_commitish': sha,
                        'draft': True, 'prerelease': nightly, 'body': ''}
        self.mutations.append('create')
        return self.release(tag)

    def edit(self, release_id, data):
        self.current.update(data)
        self.mutations.append('edit')

    def assets(self, release_id):
        return list(self.uploads.values())

    def upload(self, tag, files):
        if self.fail_upload:
            raise RuntimeError('模拟网络中断')
        for path in files:
            self.uploads[path.name] = {'id': path.name, 'name': path.name,
                                       'size': path.stat().st_size, 'digest': 'sha256:' + digest(path)}

    def delete_asset(self, asset_id):
        del self.uploads[asset_id]

    def can_advance(self, previous_sha, sha):
        return True

    def move_tag(self, tag, sha):
        self.tags[tag] = sha
        self.mutations.append('move')


class ReleaseTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.api = FakeGitHub()

    def assets(self, label='2.0.0', sha=SHA):
        artifacts = self.root / 'artifacts'
        artifacts.mkdir()
        for platform, ext in PLATFORMS.items():
            asset = artifacts / f'ElaWidgetTools-{label}-{platform}{ext}'
            asset.write_bytes(platform.encode())
            write_json(artifacts / f'package-info-{platform}.json', {
                'schema': 1, 'platform': platform, 'version': '2.0.0', 'qt_version': '6.12.0',
                'label': label, 'source_commit': sha, 'name': asset.name,
                'size': asset.stat().st_size, 'sha256': digest(asset)})
        return artifacts

    def bundle(self, nightly=False):
        label = 'Continuous-Build' if nightly else '2.0.0'
        return assemble(self.assets(label), self.root / 'release', '2.0.0', label, SHA,
                        'nightly' if nightly else 'stable', 'https://github.com/example/repo/actions/runs/1')

    def test_all_platforms_and_checksum_required(self):
        info = self.bundle()
        output = self.root / 'release'
        self.assertEqual(len(verified_files(output, info)), 5)
        (output / info['assets'][0]['name']).write_bytes(b'changed')
        with self.assertRaises(ValueError):
            verified_files(output, info)

    def test_incorrect_source_and_missing_platform_rejected(self):
        artifacts = self.assets(sha='b' * 40)
        with self.assertRaises(ValueError):
            assemble(artifacts, self.root / 'release', '2.0.0', '2.0.0', SHA, 'stable', 'url')
        (artifacts / 'package-info-macos-arm64.json').unlink()
        with self.assertRaises(ValueError):
            assemble(artifacts, self.root / 'release', '2.0.0', '2.0.0', SHA, 'stable', 'url')

    def test_stable_promotion_and_immutable_republish(self):
        info = self.bundle()
        publish(self.api, self.root / 'release', info)
        self.assertFalse(self.api.current['draft'])
        self.assertFalse(self.api.current['prerelease'])
        before = len(self.api.mutations)
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertEqual(len(self.api.mutations), before)

    def test_moved_stable_tag_rejected_before_write(self):
        info = self.bundle()
        self.api.tags['v2.0.0'] = 'b' * 40
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertFalse(self.api.mutations)

    def test_upload_failure_leaves_recoverable_draft(self):
        info = self.bundle(nightly=True)
        self.api.fail_upload = True
        with self.assertRaises(RuntimeError):
            publish(self.api, self.root / 'release', info)
        self.assertTrue(self.api.current['draft'])
        self.assertNotIn('continuous-build', self.api.tags)
        self.api.fail_upload = False
        publish(self.api, self.root / 'release', info)
        self.assertEqual(self.api.tags['continuous-build'], SHA)
        self.assertTrue(self.api.current['prerelease'])
        self.assertFalse(self.api.current['draft'])

    def test_preview_skip_and_manual_force(self):
        info = self.bundle(nightly=True)
        publish(self.api, self.root / 'release', info)
        result = resolve_inputs('2.0.0', SHA, 'nightly', 'main', False, self.api)
        self.assertFalse(result['changed'])
        self.assertTrue(resolve_inputs('2.0.0', SHA, 'nightly', 'main', True, self.api)['changed'])
        self.api.current['draft'] = True
        self.assertTrue(resolve_inputs('2.0.0', SHA, 'nightly', 'main', False, self.api)['changed'])

    def test_wrong_branch_or_version_rejected(self):
        with self.assertRaises(ValueError):
            resolve_inputs('2.0.0', SHA, 'nightly', 'feature', False, self.api)
        with self.assertRaises(ValueError):
            resolve_inputs('2.0.0', SHA, 'stable', 'v2.1.0', False, self.api)

    def test_remote_digest_mismatch_keeps_draft(self):
        info = self.bundle(nightly=True)
        real_upload = self.api.upload
        def corrupt_upload(tag, files):
            real_upload(tag, files)
            next(iter(self.api.uploads.values()))['digest'] = 'sha256:' + '0' * 64
        self.api.upload = corrupt_upload
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertTrue(self.api.current['draft'])
        self.assertNotIn('continuous-build', self.api.tags)

    def test_tag_moved_during_upload_keeps_stable_draft(self):
        info = self.bundle()
        real_upload = self.api.upload
        def move_during_upload(tag, files):
            real_upload(tag, files)
            self.api.tags[tag] = 'b' * 40
        self.api.upload = move_during_upload
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertTrue(self.api.current['draft'])

    def test_preview_replaces_old_assets_without_becoming_latest(self):
        info = self.bundle(nightly=True)
        self.api.tags['continuous-build'] = 'b' * 40
        self.api.current = {'id': 1, 'draft': False, 'prerelease': True, 'body': '旧版',
                            'target_commitish': 'b' * 40, 'tag_name': 'continuous-build'}
        self.api.uploads['obsolete.zip'] = {'id': 'obsolete.zip', 'name': 'obsolete.zip'}
        publish(self.api, self.root / 'release', info)
        self.assertNotIn('obsolete.zip', self.api.uploads)
        self.assertEqual(self.api.tags['continuous-build'], SHA)
        self.assertEqual(self.api.current['make_latest'], 'false')

    def test_checksum_file_tampering_rejected(self):
        info = self.bundle()
        (self.root / 'release/SHA256SUMS-2.0.0.txt').write_text('wrong checksum')
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertFalse(self.api.mutations)

    def test_old_run_cannot_roll_back_preview(self):
        info = self.bundle(nightly=True)
        self.api.tags['continuous-build'] = 'b' * 40
        self.api.can_advance = lambda previous, current: False
        with self.assertRaises(ValueError):
            publish(self.api, self.root / 'release', info)
        self.assertFalse(self.api.mutations)
