#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:?usage: check-windows-pe-baseline.sh BIN_DIR [MAX_MAJOR MAX_MINOR]}"
MAX_MAJOR="${2:-6}"
MAX_MINOR="${3:-2}"

command -v objdump >/dev/null 2>&1 || {
  echo "objdump is required for PE baseline inspection" >&2
  exit 1
}

count=0
while IFS= read -r -d '' file; do
  header="$(objdump -p "$file")"
  major="$(awk '/MajorSubsystemVersion/{print $2; exit}' <<<"$header")"
  minor="$(awk '/MinorSubsystemVersion/{print $2; exit}' <<<"$header")"
  format="$(objdump -f "$file" | awk -F': ' '/file format/{print $2; exit}')"

  if [[ -z "$major" || -z "$minor" || -z "$format" ]]; then
    echo "Unable to read PE metadata: $file" >&2
    exit 1
  fi
  if [[ "$format" != "pei-x86-64" ]]; then
    echo "Unexpected PE format (expected x64): $file ($format)" >&2
    exit 1
  fi
  if (( major > MAX_MAJOR || (major == MAX_MAJOR && minor > MAX_MINOR) )); then
    echo "PE subsystem baseline exceeds Windows 8.1 / Server 2012 (6.2): $file ($major.$minor)" >&2
    exit 1
  fi

  printf 'PE baseline OK: %s (%s.%s)\n' "$(basename "$file")" "$major" "$minor"
  count=$((count + 1))
done < <(find "$ROOT" -maxdepth 1 -type f \( -iname '*.exe' -o -iname '*.dll' \) -print0)

if (( count == 0 )); then
  echo "No PE executables or DLLs found in $ROOT" >&2
  exit 1
fi

echo "Inspected $count x64 PE files. This checks PE subsystem metadata only; it does not prove imported API availability or runtime compatibility on Windows Server."
