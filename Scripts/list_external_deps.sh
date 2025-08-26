#!/usr/bin/env bash
set -Eeuo pipefail

LOCKFILE="${1:-}"
if [[ -z "$LOCKFILE" ]]; then
  echo "Usage: $0 /path/to/Podfile.lock" >&2
  exit 1
fi

if [[ ! -f "$LOCKFILE" ]]; then
  echo "Lockfile not found: $LOCKFILE" >&2
  exit 1
fi

ruby -ryaml - "$LOCKFILE" <<'RUBY'
lock = YAML.load_file(ARGV[0])
external = lock["EXTERNAL SOURCES"] || {}
checkout = lock["CHECKOUT OPTIONS"] || {}
external.each do |name, info|
  git = info[:git] || info["git"]
  next unless git
  co = checkout[name] || {}
  tag = co[:tag] || co["tag"] || co[:commit] || co["commit"] || co[:branch] || co["branch"]
  puts "#{name},#{git},#{tag}"
end
RUBY
