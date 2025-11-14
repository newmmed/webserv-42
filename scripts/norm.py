import re
import sys
import os

# Capture whether any output (violations) was produced, so we can exit non-zero.
class _FlagStream(object):
    def __init__(self, stream):
        self._stream = stream
        self.had_output = False
    def write(self, s):
        # Consider non-whitespace as a signal of a violation message
        try:
            if s and s.strip():
                self.had_output = True
        except Exception:
            pass
        return self._stream.write(s)
    def flush(self):
        return self._stream.flush()
    def isatty(self):
        try:
            return self._stream.isatty()
        except Exception:
            return False
    def writelines(self, lines):
        for line in lines:
            self.write(line)

# Wrap stdout early so any print triggers the flag.
_orig_stdout = sys.stdout
sys.stdout = _FlagStream(sys.stdout)

MAX_FUNC_LINES = 30
MAX_FILE_LINES = 500
INDENT = 4

def _is_comment_line(s):
    t = s.lstrip()
    return t.startswith('//') or t.startswith('/*') or t.startswith('*') or t.startswith('*/')

def _is_preprocessor_line(s):
    return s.lstrip().startswith('#')

def _prev_nonempty_index(lines, i):
    j = i - 1
    while j >= 0 and not lines[j].strip():
        j -= 1
    return j

def check_header_guard(filename, lines):
    if not filename.endswith('.hpp'):
        return
    guard = os.path.basename(filename).replace('.', '_').upper()
    ifndef_guard = None
    define_guard = None
    ifndef_re = re.compile(r'^\s*#\s*ifndef\s+(\S+)')
    define_re = re.compile(r'^\s*#\s*define\s+(\S+)')
    endif_re = re.compile(r'^\s*#\s*endif\b')

    # Find first #ifndef
    ifndef_idx = None
    for idx, line in enumerate(lines):
        if _is_comment_line(line) or not line.strip():
            continue
        m = ifndef_re.match(line)
        if m:
            ifndef_guard = m.group(1)
            ifndef_idx = idx
            break
    # Find first #define after #ifndef
    if ifndef_idx is not None:
        for line in lines[ifndef_idx+1:]:
            if _is_comment_line(line) or not line.strip():
                continue
            m = define_re.match(line)
            if m:
                define_guard = m.group(1)
                break

    # Find last non-empty, non-comment line and ensure it's #endif
    last_idx = len(lines) - 1
    while last_idx >= 0 and (not lines[last_idx].strip() or _is_comment_line(lines[last_idx])):
        last_idx -= 1
    has_endif = last_idx >= 0 and endif_re.match(lines[last_idx]) is not None

    if not (ifndef_guard and define_guard and ifndef_guard == define_guard and has_endif and guard in ifndef_guard):
        print(f"[Header Guard] {filename}: Missing or incorrect header guard.")

def check_class_naming(line, filename):
    match = re.match(r'\s*class\s+([A-Za-z_][A-Za-z0-9_]*)', line)
    if match:
        class_name = match.group(1)
        if not class_name[0].isupper():
            print(f"[Class Naming] {filename}: Class '{class_name}' should start with uppercase.")

def _is_signature_line(line):
    # Heuristic: function definition signature (not control statements), may have '{' on same line
    sig = re.match(r'^\s*([\w:<>~*&\s]+)\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{?\s*$', line)
    if not sig:
        return None
    tokens = sig.group(1).strip().split()
    if tokens:
        first = tokens[0]
        if first in ('if', 'for', 'while', 'switch', 'catch', 'return', 'throw', 'sizeof', 'new', 'delete'):
            return None
    return sig.group(2)

def _next_nonempty_noncomment(lines, i):
    j = i + 1
    while j < len(lines) and (not lines[j].strip() or _is_comment_line(lines[j])):
        j += 1
    return j if j < len(lines) else None

def check_function_naming(lines, filename):
    i = 0
    while i < len(lines):
        name = _is_signature_line(lines[i])
        if name is None:
            i += 1
            continue
        # Skip constructors/destructors qualified with '::Name(' or '::~Name('
        line = lines[i]
        if '::' in line and re.search(r'::\s*~?[A-Z][A-Za-z0-9_]*\s*\(', line):
            i += 1
            continue
        if name and not (name[0].islower() and re.match(r'^[a-zA-Z0-9_]+$', name)):
            print(f"[Function Naming] {filename}: Function '{name}' should be snake_case or camelCase, start with lowercase.")
        i += 1

def check_function_length(lines, filename):
    i = 0
    while i < len(lines):
        name = _is_signature_line(lines[i])
        if name is None:
            i += 1
            continue
        brace_line = i if '{' in lines[i] else _next_nonempty_noncomment(lines, i)
        if brace_line is None or '{' not in lines[brace_line]:
            i += 1
            continue
        brace_count = 0
        j = brace_line
        while j < len(lines):
            brace_count += lines[j].count('{')
            brace_count -= lines[j].count('}')
            if brace_count == 0:
                func_len = j - i + 1
                if func_len > MAX_FUNC_LINES:
                    print(f"[Function Length] {filename}: Function '{name}' is {func_len} lines (max {MAX_FUNC_LINES}).")
                break
            j += 1
        i = j + 1 if j and j > i else i + 1

def check_file_length(lines, filename):
    if len(lines) > MAX_FILE_LINES:
        print(f"[File Length] {filename}: {len(lines)} lines (max {MAX_FILE_LINES}).")

def check_indentation(lines, filename):
    for i, line in enumerate(lines):
        if not line.strip():
            continue
        if _is_preprocessor_line(line) or _is_comment_line(line):
            continue
        if line.startswith('\t'):
            continue
        leading_spaces = len(line) - len(line.lstrip(' '))
        if leading_spaces > 0 and (leading_spaces % INDENT) != 0:
            print(f"[Indentation] {filename}: Line {i+1} not indented in multiples of {INDENT} spaces.")

def check_function_comments(lines, filename):
    i = 0
    while i < len(lines):
        name = _is_signature_line(lines[i])
        if name is None:
            i += 1
            continue
        prev = _prev_nonempty_index(lines, i)
        if prev < 0 or (not lines[prev].lstrip().startswith('//') and not lines[prev].rstrip().endswith('*/')):
            print(f"[Comment] {filename}: Function at line {i+1} missing comment.")
        i += 1

def check_file(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    check_header_guard(filename, lines)
    check_file_length(lines, filename)
    check_indentation(lines, filename)
    check_function_length(lines, filename)
    for line in lines:
        check_class_naming(line, filename)
    check_function_naming(lines, filename)
    check_function_comments(lines, filename)

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        check_file(filename)

    # If any output was produced (violations), exit non-zero so hooks/CI can fail.
    try:
        had_output = getattr(sys.stdout, 'had_output', False)
    except Exception:
        had_output = False
    # Restore original stdout before exiting
    sys.stdout = _orig_stdout
    if had_output:
        sys.exit(1)
    sys.exit(0)