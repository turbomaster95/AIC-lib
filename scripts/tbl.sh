#!/usr/bin/env bash
set -euo pipefail

if [ "${1:-}" = "" ]; then
  echo "Usage: $0 <syscall_64.tbl> [HEADER_NAME]" >&2
  exit 2
fi

TBL="$1"
HEADER_NAME="${2:-ARCH_SYSCALL_H}"

awk -v GUARD="$HEADER_NAME" '
BEGIN {
  print "/* Generated from table file - do not edit manually */"
  print "/* ALL EDITS WILL BE LOST!! */"
  print "#ifndef " GUARD
  print "#define " GUARD
  print ""
  print "/* Architecture syscall numbers generated from table file */"
  print ""
}
# skip comments and empty lines
/^[[:space:]]*#/ { next }
/^[[:space:]]*$/ { next }

# fields: number abi name entrypoint ...
{
  num = $1
  abi = $2
  name = $3

  # only accept numeric syscall numbers
  if (num !~ /^[0-9]+$/) next

  # avoid repeated names (first occurrence wins)
  if (seen[name]++) next

  # sanitize name: replace any non-alnum/_ with _
  gsub(/[^A-Za-z0-9_]/, "_", name)

  # print define
  printf("#define __NR_%s %s\n", name, num) > "/dev/stdout"

  # track x32 legacy range
  if (num >= 512 && num <= 547) {
    x32_count++
  }
}
END {
  if (x32_count > 0) {
    print ""
    print "/* Note: syscalls numbered 512-547 are the legacy x32 range. */"
  }
  print ""
  print "#endif"
}
' "$TBL"

