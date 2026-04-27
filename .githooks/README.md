# Project git hooks

Git hooks live here (instead of `.git/hooks/`) so they ship with the repo.
Activate them in your local clone with:

```sh
git config core.hooksPath .githooks
```

Run that once per clone. It's local to your `.git/config` (per safety
policy the project does not auto-mutate user git config on clone).

## Hooks

### `commit-msg`

Enforces the **no Co-Authored-By trailer** rule from
[CLAUDE.md](../CLAUDE.md#commit-policy). Rejects commits whose message
contains:

- `Co-Authored-By:` (any AI agent)
- `🤖 Generated with [Claude Code]` or `Generated with Claude Code`

Bypass for legitimate human co-author commits with `--no-verify`.

## Adding hooks

Drop a new `<hook-name>` file here, make it executable
(`chmod +x .githooks/<hook-name>`), and document it above. Anything in
this directory is automatically picked up once `core.hooksPath` is set.
