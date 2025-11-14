#!/usr/bin/env bash

set -euo pipefail

# Install or update the local Git pre-commit hook.

repo_root=$(git rev-parse --show-toplevel 2>/dev/null || true)
if [[ -z "${repo_root}" ]]; then
  echo "Not a git repository; cannot install pre-commit hook." >&2
  exit 1
fi

hook_dir="${repo_root}/.git/hooks"
hook_path="${hook_dir}/pre-commit"

mkdir -p "${hook_dir}"

cat > "${hook_path}" <<'HOOK'
#!/usr/bin/env bash

# Pre-commit hook: builds the project (make all). Regardless of success/failure, runs make fclean.

set -euo pipefail

repo_root=$(git rev-parse --show-toplevel)
cd "${repo_root}"

# Run style checker (non-blocking): collect staged code files and log output
mkdir -p "${repo_root}/logs"

# Collect staged files (Added, Copied, Modified, Renamed)
mapfile -t staged < <(git diff --cached --name-only --diff-filter=ACMR)

code_files=()
for f in "${staged[@]}"; do
  case "$f" in
    srcs/*|includes/*)
      case "$f" in
        *.c|*.cc|*.cxx|*.cpp|*.h|*.hpp)
          # Only include files that still exist in the index (skip deletions)
          if git cat-file -e ":${f}" 2>/dev/null; then
            code_files+=("$f")
          fi
          ;;
      esac
      ;;
  esac
done

# Prefer python3 if available
py=python3
command -v python3 >/dev/null 2>&1 || py=python

if [[ ${#code_files[@]} -gt 0 ]]; then
  echo "Running scripts/norm.py on staged files (logging to logs/norm_errors.txt)..."
  # Overwrite log on each run; do not fail commit regardless of exit code
  "$py" "${repo_root}/scripts/norm.py" "${code_files[@]}" > "${repo_root}/logs/norm_errors.txt" 2>&1 || true
else
  # If nothing to check, clear the log to avoid stale results
  : > "${repo_root}/logs/norm_errors.txt"
fi

# Build the project to ensure no errors
echo "Building (make all)..."
build_ok=1
if make -C "${repo_root}" all; then
  build_ok=0
else
  build_ok=2
fi

# Always clean after attempting to build
echo "Cleaning (make fclean)..."
make -C "${repo_root}" fclean || true

if [[ $build_ok -ne 0 ]]; then
  echo "Pre-commit: build failed. Fix errors before committing." >&2
  exit 1
fi

exit 0
HOOK

chmod +x "${hook_path}"
echo "Installed pre-commit hook at ${hook_path}"

# Configure commit message template if present
if [[ -f "${repo_root}/.gitmessage" ]]; then
  git config commit.template "${repo_root}/.gitmessage"
  echo "Configured git commit.template to ${repo_root}/.gitmessage"
fi
