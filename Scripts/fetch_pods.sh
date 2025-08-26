#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PATCH_MONKEY_GIT="$SCRIPT_DIR/patches/monkey_git.rb"
PATCH_KEEP_TMP="$SCRIPT_DIR/patches/keep_tmp_dirs.rb"
PATCHER_DL_GIT="$SCRIPT_DIR/patches/patch_downloader_git.rb"

PODFILE_DIR="$REPO_ROOT/Sample"
DEST="$REPO_ROOT/output"
MODE="rubypre"   # rubypre | patchgit
VERBOSE=0
UNSHALLOW=0
KEEP_ONLY_REPOS=1
FINAL_DIR_NAME="repos"

log(){ printf '[%s] %s\n' "$(date +%H:%M:%S)" "$*" >&2; }
die(){ printf 'ERROR: %s\n' "$*" >&2; exit 1; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    --podfile-dir) PODFILE_DIR="$2"; shift 2;;
    --dest) DEST="$2"; shift 2;;
    --verbose) VERBOSE=1; shift;;
    --unshallow) UNSHALLOW=1; shift;;
    --patch-downloader-git) MODE="patchgit"; shift;;
    --restore-downloader-git) ruby "$PATCHER_DL_GIT" restore --print-path; exit 0;;
    --keep-all) KEEP_ONLY_REPOS=0; shift;;
    --final-dir-name) FINAL_DIR_NAME="$2"; shift 2;;
    *) die "Unknown arg: $1";;
  esac
done

[[ -d "$PODFILE_DIR" ]] || die "Podfile dir not found: $PODFILE_DIR"
PODFILE_DIR="$(cd "$PODFILE_DIR" && pwd)"
mkdir -p "$DEST"
DEST="$(cd "$DEST" && pwd)"
mkdir -p "$DEST/tmp_clones/_tmp" "$DEST/$FINAL_DIR_NAME"

log "Mode: $MODE"
log "Using DEST: $DEST"
log "pod: $(command -v pod || echo 'not found') | $(pod --version || true)"

pushd "$PODFILE_DIR" >/dev/null

if [[ "$MODE" == "patchgit" ]]; then
  DETECTED="$(ruby "$PATCHER_DL_GIT" detect || true)"
  if [[ -n "$DETECTED" ]]; then
    log "Detected git.rb: $DETECTED"
  else
    log "Detector could not find git.rb automatically; consider setting CP_DL_GIT_RB"
  fi
  ruby "$PATCHER_DL_GIT" apply --print-path
  RUBYOPT_OVERRIDE=""
else
  RUBYOPT_OVERRIDE="-r$PATCH_MONKEY_GIT -r$PATCH_KEEP_TMP"
fi

CP_HOME_DIR="$DEST/_cp_home"
CP_GIT_TMP_ROOT="$DEST/tmp_clones/_tmp"

env \
  CP_HOME_DIR="$CP_HOME_DIR" \
  CP_GIT_TMP_ROOT="$CP_GIT_TMP_ROOT" \
  CP_GIT_SHALLOW="$([[ $UNSHALLOW -eq 1 ]] && echo 0 || echo 1)" \
  RUBYOPT="$RUBYOPT_OVERRIDE" \
  pod install $([[ $VERBOSE -eq 1 ]] && echo --verbose) 2>&1 | tee "$DEST/pod_install.log"
STATUS=${PIPESTATUS[0]}

popd >/dev/null

if [[ $STATUS -ne 0 ]]; then
  [[ "$MODE" == "patchgit" ]] && ruby "$PATCHER_DL_GIT" restore --print-path || true
  die "pod install failed. See $DEST/pod_install.log"
fi

# Move clones into final directory and cleanup
shopt -s nullglob dotglob
for cdir in "$CP_GIT_TMP_ROOT"/*; do
  [[ -d "$cdir/.git" ]] || continue
  base="$(basename "$cdir")"
  dest_path="$DEST/$FINAL_DIR_NAME/$base"
  echo "[INFO] Finalizing repo: $base -> $dest_path"
  rsync -a "$cdir/." "$dest_path/"
done

if [[ $KEEP_ONLY_REPOS -eq 1 ]]; then
  echo "[INFO] Cleaning up non-essential files..."
  rm -rf "$DEST/tmp_clones" "$DEST/_cp_home" "$DEST/Pods" "$DEST/workspace" "$DEST/cache-list.txt" "$DEST/pods_list.csv"
  rm -f "$DEST/pod_install.log"
fi

if [[ "$MODE" == "patchgit" ]]; then
  ruby "$PATCHER_DL_GIT" restore --print-path
  echo "[INFO] Restored downloader git.rb"
fi

echo "Done. Repos live in: $DEST/$FINAL_DIR_NAME"
