"""GitHub Release 发布状态机：先验证、组装草稿，再公开；稳定标签永不移动。"""
import argparse
import json
from pathlib import Path
import re
import subprocess
from assemble import verified_files
from common import digest


class GitHub:
    def __init__(self, repository):
        if not re.fullmatch(r'[\w.-]+/[\w.-]+', repository):
            raise ValueError('无效 GitHub 仓库名称')
        self.repository = repository

    def api(self, path, method='GET', data=None, optional=False):
        command = ['gh', 'api', f'repos/{self.repository}/{path}', '--method', method]
        if data is not None:
            command += ['--input', '-']
        result = subprocess.run(command, input=json.dumps(data) if data is not None else None,
                                text=True, capture_output=True)
        if result.returncode:
            if optional and 'HTTP 404' in result.stderr:
                return None
            raise RuntimeError(result.stderr.strip())
        return json.loads(result.stdout) if result.stdout.strip() else None

    def pages(self, path):
        result = []
        page = 1
        while True:
            batch = self.api(f'{path}?per_page=100&page={page}')
            result.extend(batch)
            if len(batch) < 100:
                return result
            page += 1

    def tag(self, tag):
        ref = self.api(f'git/ref/tags/{tag}', optional=True)
        if ref is None:
            return None
        obj = ref['object']
        # 同时支持轻量标签和 annotated tag；限制解引用深度。
        for _ in range(10):
            if obj['type'] == 'commit':
                return obj['sha']
            if obj['type'] != 'tag':
                break
            obj = self.api(f'git/tags/{obj["sha"]}')['object']
        raise ValueError('标签没有指向可识别的提交')

    def release(self, tag):
        release = self.api(f'releases/tags/{tag}', optional=True)
        if release is not None:
            return release
        # tags 接口看不到草稿，通过认证列表恢复上一次中断的草稿。
        return next((r for r in self.pages('releases') if r['tag_name'] == tag), None)

    def create(self, tag, sha, nightly):
        return self.api('releases', 'POST', {
            'tag_name': tag, 'target_commitish': sha, 'draft': True,
            'prerelease': nightly, 'name': 'Continuous Build' if nightly else tag,
            'make_latest': 'false',
        })

    def edit(self, release_id, data):
        return self.api(f'releases/{release_id}', 'PATCH', data)

    def assets(self, release_id):
        return self.pages(f'releases/{release_id}/assets')

    def delete_asset(self, asset_id):
        self.api(f'releases/assets/{asset_id}', 'DELETE')

    def upload(self, tag, files):
        subprocess.run(['gh', 'release', 'upload', tag, *map(str, files), '--clobber',
                        '--repo', self.repository], check=True)

    def can_advance(self, previous_sha, sha):
        comparison = self.api(f'compare/{previous_sha}...{sha}')
        return comparison['status'] in {'ahead', 'identical'}

    def move_tag(self, tag, sha):
        if tag != 'continuous-build':
            raise ValueError('只允许移动 continuous-build 标签')
        if self.tag(tag) is None:
            self.api('git/refs', 'POST', {'ref': f'refs/tags/{tag}', 'sha': sha})
        else:
            self.api(f'git/refs/tags/{tag}', 'PATCH', {'sha': sha, 'force': True})


def release_notes(info):
    preview = info['mode'] == 'nightly'
    return '\n'.join([
        '# Continuous Build' if preview else f'# ElaWidgetTools {info["version"]}', '',
        '这是跟随 main 更新的开发预览包。' if preview else '这是版本标签对应的稳定发布包。', '',
        f'- 组件库版本：`{info["version"]}`', f'- 源提交：`{info["source_commit"]}`',
        f'- Qt 构建基线：`{info["qt_version"]}`；使用其他受支持的 Qt 版本时请重新编译源码。',
        f'- 构建记录：{info["workflow_run"]}', f'- 构建时间（UTC）：{info["built_at"]}', '',
        '| 平台 | 下载包说明 |', '| --- | --- |',
        '| Linux x86_64 | 开发安装包及 Example，需要匹配的 Qt SDK；不保证跨发行版直接运行 |',
        '| Windows x64 | MSVC 开发包与已部署 Qt 依赖的 Example |',
        '| macOS ARM64 | 开发包和已部署的 .app；实验性支持，未做 Developer ID 签名及公证 |', '',
        '开发包包含动态库、公共头文件、CMake 配置和示例。校验下载请使用附带的 SHA256SUMS 文件。',
        '源码支持 Qt 6.12.0 及以上；预编译库使用 Qt 私有接口，不能任意替换 Qt 运行库。', '',
        f'<!-- source-commit: {info["source_commit"]} -->',
    ])


def publish(github, directory, info):
    files = verified_files(directory, info)
    tag, sha = info['tag'], info['source_commit']
    nightly = info['mode'] == 'nightly'
    previous_sha = github.tag(tag)
    if nightly and previous_sha and not github.can_advance(previous_sha, sha):
        raise ValueError('已有更新的预览或历史已分叉，拒绝旧任务回退；请从 main 重新运行')
    if not nightly and previous_sha != sha:
        raise ValueError('稳定标签不存在或已移动，拒绝发布')
    release = github.release(tag)
    if release:
        if not nightly and not release['draft']:
            raise ValueError('稳定版已公开，必须使用新版本')
        if not nightly and release['target_commitish'] != sha:
            raise ValueError('现有草稿指向其他提交')
        if nightly and not release['prerelease']:
            raise ValueError('不能用预览包覆盖稳定 Release')
    else:
        release = github.create(tag, sha, nightly)
    rid = release['id']
    # 固定下载名称替换期间暂时隐藏预览；失败保留草稿，重新运行可以恢复。
    if not release['draft']:
        github.edit(rid, {'draft': True})
    github.upload(tag, files)
    remote = github.assets(rid)
    by_name = {asset['name']: asset for asset in remote}
    for path in files:
        asset = by_name.get(path.name)
        if (not asset or asset['size'] != path.stat().st_size
                or asset.get('digest') != 'sha256:' + digest(path)):
            raise ValueError(f'远端资产不完整或摘要不一致：{path.name}')
    # 删除草稿中上一次失败运行遗留的旧资产，避免混入不同版本。
    keep = {p.name for p in files}
    for asset in remote:
        if asset['name'] not in keep:
            github.delete_asset(asset['id'])
    if nightly:
        if github.tag(tag) != previous_sha:
            raise ValueError('上传期间预览标签被其他操作修改，保持草稿')
        github.move_tag(tag, sha)
    elif github.tag(tag) != sha:
        raise ValueError('上传期间稳定标签发生变化，保持草稿')
    github.edit(rid, {'draft': False, 'prerelease': nightly,
                      'name': 'Continuous Build' if nightly else tag,
                      'target_commitish': sha, 'body': release_notes(info),
                      'make_latest': 'false' if nightly else 'true'})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--directory', type=Path, required=True)
    args = parser.parse_args()
    metadata = list(args.directory.glob('release-info-*.json'))
    if len(metadata) != 1:
        parser.error('发布目录必须恰好包含一份 release-info 元数据')
    publish(GitHub(args.repository), args.directory, json.loads(metadata[0].read_text(encoding='utf-8')))
