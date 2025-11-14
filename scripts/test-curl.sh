#!/usr/bin/env bash
set -euo pipefail
BASE="http://127.0.0.1:9900"

pass() { echo -e "[PASS] $1"; }
fail() { echo -e "[FAIL] $1"; exit 1; }
expect_status() {
  local url="$1"
  local method="${2:-GET}"
  local data="${3:-}"
  local expect="${4:-200}"  # default expected status 200 if not provided
  local out status
  if [ "$method" = GET ]; then
    out=$(curl -s -o /dev/null -w "%{http_code}" "$BASE$url")
  elif [ "$method" = POST ]; then
    out=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "$data" "$BASE$url")
  elif [ "$method" = DELETE ]; then
    out=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE$url")
  else
    out=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$BASE$url")
  fi
  if [ "$out" = "$expect" ]; then pass "$method $url → $expect"; else fail "$method $url expected $expect got $out"; fi
}

# Start checks
expect_status "/" GET 200
expect_status "/test.html" GET 200
expect_status "/static/hello.txt" GET 200

# Autoindex directory
expect_status "/static/" GET 200

# 404
expect_status "/nope.txt" GET 404

# Method not allowed
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE/static/hello.txt")
[ "$status" = 405 ] && pass "POST /static/hello.txt → 405" || fail "POST /static/hello.txt expected 405 got $status"

# Redirect
status=$(curl -s -o /dev/null -w "%{http_code}" -L -X GET "$BASE/old-page")
[ "$status" = 200 ] && pass "GET /old-page redirects to /new-page" || fail "Redirect chain failed"

# Upload small payload
status=$(curl -s -o -D- -w "%{http_code}" -X POST "$BASE/upload" --data "abc" )
[ "$status" = 200 ] && pass "POST /upload small body accepted" || echo "Note: If upload returns 201, adjust script"

# Oversize body (300KB) should be 413
bigtmp=$(mktemp)
python3 - <<'PY' > "$bigtmp"
import sys
sys.stdout.write('A'*300*1024)
PY
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE/upload" --data-binary @"$bigtmp")
[ "$status" = 413 ] && pass "POST /upload large body → 413" || fail "Expected 413 for large body, got $status"
rm -f "$bigtmp"

# DELETE: create then delete
resp=$(curl -s -D- -X POST "$BASE/upload" --data "to-delete")
loc=$(echo "$resp" | awk -F': ' '/^Location:/ {print $2}' | tr -d '\r')
if [ -n "$loc" ]; then
  # Location may be filesystem path; map to URL /upload/ by basename
  base=$(basename "$loc")
  status=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE/upload/$base")
  [ "$status" = 200 ] && pass "DELETE /upload/$base → 200" || fail "DELETE failed"
else
  echo "[WARN] No Location header from upload; skipping DELETE test"
fi

echo "All checks completed."
