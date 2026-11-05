"""在启动平台构建前固定版本、源 SHA，并跳过没有变化的每日预览。"""
import argparse
import os
from pathlib import Path
import subprocess
from common import project_version, validate_identity
from publish import GitHub


def resolve_inputs(version, sha, mode, ref, force, github):
    if mode not in {'stable', 'nightly'}:
        raise ValueError('无效发布模式')
    nightly = mode == 'nightly'
    label = 'Continuous-Build' if nightly else version
    validate_identity(version, label, sha)
    tag = 'continuous-build' if nightly else f'v{version}'
    if not nightly and (ref != tag or github.tag(tag) != sha):
        raise ValueError('稳定标签必须与组件库版本和源提交完全一致')
    if nightly and ref != 'main':
        raise ValueError('Continuous Build 只允许从 main 构建')
    release = github.release(tag)
    if not nightly and release and not release['draft']:
        raise ValueError('稳定版已经公开，请创建新版本')
    unchanged = bool(nightly and release and not release['draft'] and release['prerelease']
                     and f'<!-- source-commit: {sha} -->' in (release.get('body') or '')
                     and github.tag(tag) == sha)
    return {'mode': mode, 'sha': sha, 'version': version, 'label': label,
            'tag': tag, 'changed': force or not unchanged}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=['stable', 'nightly'], required=True)
    parser.add_argument('--ref', required=True)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--force', action='store_true')
    args = parser.parse_args()
    sha = subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip()
    values = resolve_inputs(project_version('.'), sha, args.mode, args.ref, args.force, GitHub(args.repository))
    with Path(os.environ['GITHUB_OUTPUT']).open('a', encoding='utf-8') as output:
        for key, value in values.items():
            print(f'{key}={str(value).lower() if isinstance(value, bool) else value}', file=output)
    print('准备构建并发布' if values['changed'] else '源提交未变化，跳过本次构建和发布')
