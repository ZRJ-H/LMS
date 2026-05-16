"""Decode a GBK-encoded C_LMS file to UTF-8 and save to a temp file for Read tool.
Usage: python .claude/tools/gbk_cat.py <filepath>
Output: writes decoded UTF-8 to .claude/tools/_c_lms_view.txt
"""
import sys
import os

OUTPUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '_c_lms_view.txt')

def read_gbk(filepath):
    for enc in ['gbk', 'gb18030', 'utf-8']:
        try:
            with open(filepath, 'r', encoding=enc) as f:
                return f.read(), enc
        except (UnicodeDecodeError, UnicodeError):
            continue
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        return f.read(), 'utf-8 (with errors)'

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python gbk_cat.py <filepath>")
        sys.exit(1)
    content, enc = read_gbk(sys.argv[1])
    with open(OUTPUT, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"[{enc}] -> {OUTPUT}  ({len(content)} chars)")
