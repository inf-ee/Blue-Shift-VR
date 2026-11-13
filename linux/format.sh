#!/bin/bash
# Formats the game code with the repository's .clang-format (needs clang-format 20 or newer)
# and strips trailing whitespace, which clang-format leaves in comments and macros.
# Formats the .cpp/.h files built by the Visual Studio projects, without third-party code.
# Usage: linux/format.sh          format in place
#        linux/format.sh --check  report files that are not formatted, change nothing
# Set CLANG_FORMAT to use a clang-format other than the one in PATH.
set -eo pipefail
cd "$(dirname "$0")/.."

CF=${CLANG_FORMAT:-clang-format}
version=$("$CF" --version | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
if [ "${version%%.*}" -lt 20 ]; then
	echo "clang-format $version is too old, the .clang-format options need 20 or newer" >&2
	exit 1
fi

mapfile -t files < <(grep -ohP 'Include="\.\.\\\.\.\\\K[^"]+\.(cpp|h|hpp)(?=")' projects/vs2019/hldll.vcxproj projects/vs2019/hl_cdll.vcxproj |
	tr '\\' '/' | grep -vE '^external/' | sort -u |
	while read -r f; do [ -f "$f" ] && echo "$f"; done)

if [ "$1" = "--check" ]; then
	status=0
	"$CF" --style=file --dry-run --Werror "${files[@]}" || status=1
	if grep -lP '[ \t]+\r?$' "${files[@]}" >&2; then
		echo "trailing whitespace in the files above" >&2
		status=1
	fi
	exit $status
else
	"$CF" -i --style=file "${files[@]}"
	perl -pi -e 's/[ \t]+(\r?)$/$1/' "${files[@]}"
fi
