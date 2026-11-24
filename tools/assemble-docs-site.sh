#!/usr/bin/env bash
# Build a GitHub Pages tree: the firmware, manifests and installer currently live at
# https://skydoy.github.io/ESPHome_UFH/ plus a fresh MkDocs site under docs/.
# Does not rebuild firmware and does not copy static/index.html from git,
# because main can be ahead of the last Pages deploy.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
dest="${1:-$root/site-pages}"
base="https://skydoy.github.io/ESPHome_UFH"
rm -rf "$dest"
mkdir -p "$dest"

download() {
  local url="$1" out="$2"
  mkdir -p "$(dirname "$out")"
  curl -fsSL --retry 3 --retry-delay 2 -o "$out" "$url"
}

echo "Downloading the live installer files"
for name in $(find "$root/static" -maxdepth 1 -type f -printf '%f\n' | sort); do
  # A file added to static/ after the last firmware release is not live yet (404). Skip it:
  # this script keeps what is deployed, and the next release publishes the new file.
  code=$(curl -sSL -o /dev/null -w '%{http_code}' --retry 3 --retry-delay 2 "$base/$name")
  if [ "$code" = "404" ]; then
    echo "  $name: not live yet, skipped"
    continue
  fi
  download "$base/$name" "$dest/$name"
done

python3 - "$dest" "$base" << 'PY'
import hashlib, json, os, sys, urllib.request
dest, base = sys.argv[1], sys.argv[2].rstrip("/")
manifests = {
    "firmware/p4-poe/manifest.json": "esphome-ufh-esp32p4.ota.bin",
    "firmware/p4-poe-direct/manifest.json": "esphome-ufh-esp32p4.ota.bin",
}

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()

def fetch(rel):
    url = f"{base}/{rel}"
    out = os.path.join(dest, rel)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    print(f"GET {url}")
    with urllib.request.urlopen(url, timeout=120) as r:
        data = r.read()
    with open(out, "wb") as f:
        f.write(data)
    return out

paths = []
for rel, ota_name in manifests.items():
    path = fetch(rel)
    doc = json.load(open(path))
    blob = json.dumps(doc)
    if ota_name not in blob:
        raise SystemExit(f"{rel} does not name {ota_name}")
    files = []
    for build in doc["builds"]:
        files.append(build["ota"]["path"])
        for part in build.get("parts", []):
            files.append(part["path"])
    folder = os.path.dirname(rel)
    for name in files:
        bin_rel = f"{folder}/{name}" if folder else name
        bin_path = fetch(bin_rel)
        digest = sha256(bin_path)
        paths.append((bin_rel, digest, os.path.getsize(bin_path)))
        print(f"  {bin_rel} {digest} {os.path.getsize(bin_path)}")
index = open(os.path.join(dest, "index.html"), encoding="utf-8").read()
for needle in ("esp-web-install-button", "firmware/p4-poe/manifest.json", "firmware/p4-poe-direct/manifest.json"):
    if needle not in index:
        raise SystemExit(f"live installer is missing {needle}")
stamp = os.path.join(dest, ".live-firmware-sha256")
with open(stamp, "w") as f:
    for rel, digest, size in paths:
        f.write(f"{digest}  {rel}  {size}\n")
print(f"preserved {len(paths)} firmware files")
PY

echo "Building documentation into ${dest}/docs"
(cd "$root" && mkdocs build --strict -d "$dest/docs")

python3 - "$dest" << 'PY'
import hashlib, os, sys
dest = sys.argv[1]
stamp = os.path.join(dest, ".live-firmware-sha256")

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()

for line in open(stamp):
    digest, rel, size = line.split()
    path = os.path.join(dest, rel)
    got = sha256(path)
    if got != digest or str(os.path.getsize(path)) != size:
        raise SystemExit(f"firmware changed during docs build: {rel}")
os.remove(stamp)
for rel in (
    "docs/index.html",
    "docs/pt/index.html",
    "index.html",
    "firmware/p4-poe/manifest.json",
    "firmware/p4-poe-direct/manifest.json",
):
    if not os.path.isfile(os.path.join(dest, rel)):
        raise SystemExit(f"missing {rel}")
print("firmware bytes unchanged; docs and installer are present")
PY
echo "assembled $dest"
