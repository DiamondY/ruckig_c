# GitHub Operations Notes

This document records project-specific GitHub CLI handling notes for release,
workflow, tag, and CI evidence operations.

## GitHub CLI Authentication

On Windows, `gh` stores the active GitHub token in the user keyring. In Codex or
other managed/sandboxed command environments, a plain sandboxed command can
report an invalid token even when the user has successfully authenticated in the
normal desktop session:

```text
gh auth status -h github.com
X Failed to log in to github.com account DiamondY (default)
The token in default is invalid.
```

Do not treat that sandbox-only result as proof that the GitHub account lacks
permissions. First verify from a command environment that can read the Windows
keyring:

```powershell
gh auth status -h github.com
```

Expected valid state:

```text
github.com
  ✓ Logged in to github.com account DiamondY (keyring)
  - Active account: true
  - Git operations protocol: https
  - Token scopes: 'gist', 'read:org', 'repo', 'workflow'
```

For this repository, the relevant scopes are:

- `repo`: push, tag push, repository reads/writes, artifact access, and GitHub
  Release creation.
- `workflow`: manual GitHub Actions workflow dispatch, including release-random
  runs.
- `read:org`: organization/repository visibility checks where GitHub requires
  organization context.

`gist` is not required by this repository, but it can appear in the authenticated
token without affecting the release flow.

## Codex Command Policy

When a remote operation depends on `gh` authentication and the sandboxed command
reports an invalid token, retry the authentication check or the remote operation
from an environment that can access the keyring instead of assuming a permission
failure.

Use this approach for:

- `gh auth status -h github.com`
- `gh run view`
- `gh workflow run`
- `gh release create`
- `gh run download`

Do not print or record the raw token. It is acceptable to record the visible
scope summary from `gh auth status`, with the token value redacted by `gh`.

If keyring-aware `gh auth status` still fails, refresh authentication before
continuing release operations:

```powershell
gh auth logout -h github.com -u DiamondY
gh auth login -h github.com
gh auth status -h github.com
```

Stop before tag/release/workflow-dispatch steps if valid authentication cannot
be confirmed.

## Read-Only Fallbacks

Some release checks do not require authenticated `gh`:

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c ls-remote --tags ruckig_c "refs/tags/v*"
```

Public GitHub REST endpoints can also verify read-only state, such as published
releases, without relying on the local `gh` token:

```powershell
python -c "import json, urllib.request; req=urllib.request.Request('https://api.github.com/repos/DiamondY/ruckig_c/releases?per_page=30', headers={'User-Agent':'ruckig_c-release-check'}); data=json.load(urllib.request.urlopen(req, timeout=30)); print('\\n'.join(r['tag_name'] for r in data))"
```

These fallbacks are useful for confirming absence of an accidental tag or
release, but they do not replace authenticated checks for creating releases,
dispatching workflows, or downloading private artifacts.
