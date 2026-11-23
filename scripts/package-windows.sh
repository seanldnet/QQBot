#!/usr/bin/env bash
set -euo pipefail

DEST="${1:?usage: package-windows.sh DEST_DIR BINARY...}"
shift
mkdir -p "$DEST"
MINGW_BIN="${MINGW_PREFIX:-/mingw64}/bin"
declare -A seen
queue=("$@")

is_windows_system_dll() {
  local n="${1,,}"
  case "$n" in
    api-ms-win-*|ext-ms-win-*|kernel32.dll|user32.dll|advapi32.dll|shell32.dll|ole32.dll|oleaut32.dll|gdi32.dll|ws2_32.dll|bcrypt.dll|crypt32.dll|secur32.dll|ntdll.dll|rpcrt4.dll|comdlg32.dll|comctl32.dll|winmm.dll|version.dll|shlwapi.dll|iphlpapi.dll|normaliz.dll|dnsapi.dll|psapi.dll|setupapi.dll|cfgmgr32.dll|imm32.dll|msvcrt.dll) return 0 ;;
    *) return 1 ;;
  esac
}

while (("${#queue[@]}" > 0)); do
  current="${queue[0]}"
  queue=("${queue[@]:1}")
  [[ -f "$current" ]] || continue
  while IFS= read -r dll; do
    dll="${dll,,}"
    [[ -n "$dll" ]] || continue
    [[ -n "${seen[$dll]:-}" ]] && continue
    seen[$dll]=1
    is_windows_system_dll "$dll" && continue
    source_path="$(find "$MINGW_BIN" -maxdepth 1 -type f -iname "$dll" -print -quit)"
    if [[ -z "$source_path" ]]; then
      echo "Unresolved non-system runtime DLL: $dll (required by $current)" >&2
      exit 1
    fi
    cp -f "$source_path" "$DEST/"
    queue+=("$source_path")
  done < <(objdump -p "$current" | awk '/DLL Name:/{print $3}')
done

echo "Bundled ${#seen[@]} non-system DLL dependencies into $DEST"
