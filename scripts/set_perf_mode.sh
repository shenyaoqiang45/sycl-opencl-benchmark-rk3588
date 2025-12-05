#!/usr/bin/env bash
# Toggle CPU/GPU performance mode on RK3588 (and similar Linux systems)
# Usage:
#   sudo ./scripts/set_perf_mode.sh performance   # enable performance mode
#   sudo ./scripts/set_perf_mode.sh powersave     # enable powersave mode
#   ./scripts/set_perf_mode.sh status             # show current governors/freqs

set -euo pipefail

MODE=${1:-}
if [ -z "$MODE" ]; then
  echo "Usage: $0 <performance|powersave|status>"
  exit 2
fi

if [ "$MODE" != "status" ] && [ "$EUID" -ne 0 ]; then
  echo "This script must be run as root to change governors/frequencies. Re-running with sudo..."
  exec sudo "$0" "$@"
fi

info() { printf "[INFO] %s\n" "$*"; }
warn() { printf "[WARN] %s\n" "$*"; }
err() { printf "[ERROR] %s\n" "$*"; }

set_cpu_mode() {
  local mode=$1
  local cpus
  cpus=$(ls -d /sys/devices/system/cpu/cpu[0-9]* 2>/dev/null || true)
  if [ -z "$cpus" ]; then
    warn "No CPU cpufreq sysfs entries found; system may not expose cpufreq"
    return
  fi

  for cpu in $cpus; do
    local govfile="$cpu/cpufreq/scaling_governor"
    local maxfile="$cpu/cpufreq/scaling_max_freq"
    local minfile="$cpu/cpufreq/scaling_min_freq"
    local info_max="$cpu/cpufreq/cpuinfo_max_freq"
    local info_min="$cpu/cpufreq/cpuinfo_min_freq"
    if [ ! -f "$govfile" ]; then
      continue
    fi

    if [ "$mode" = "status" ]; then
      printf "CPU: %s governor=%s" "$(basename $cpu)" "$(cat $govfile 2>/dev/null || echo unknown)"
      if [ -f "$maxfile" ]; then
        printf " max=%s" "$(cat $maxfile 2>/dev/null || echo unknown)"
      fi
      if [ -f "$minfile" ]; then
        printf " min=%s" "$(cat $minfile 2>/dev/null || echo unknown)"
      fi
      printf "\n"
      continue
    fi

    # Set governor if available
    local avail=""
    avail=$(cat "$cpu/cpufreq/scaling_available_governors" 2>/dev/null || true)
    if [ -n "$avail" ]; then
      if echo "$avail" | grep -qw "$mode"; then
        echo "$mode" > "$govfile" 2>/dev/null || warn "Could not write $mode to $govfile"
        info "Set $govfile -> $mode"
      else
        # Try reasonable fallbacks
        if [ "$mode" = "performance" ]; then
          if echo "$avail" | grep -qw "performance"; then
            echo performance > "$govfile" 2>/dev/null || warn "Could not write performance to $govfile"
            info "Set $govfile -> performance"
          else
            warn "Governor 'performance' not available for $(basename $cpu): $avail"
          fi
        else
          # powersave fallback to 'powersave' or 'ondemand' or 'schedutil'
          for g in powersave ondemand schedutil conservative; do
            if echo "$avail" | grep -qw "$g"; then
              echo "$g" > "$govfile" 2>/dev/null || warn "Could not write $g to $govfile"
              info "Set $govfile -> $g"
              break
            fi
          done
        fi
      fi
    else
      warn "No 'scaling_available_governors' for $(basename $cpu); skipping governor change"
    fi

    # Optionally set max/min frequencies from cpuinfo ranges (aggressive)
    if [ "$mode" = "performance" ]; then
      if [ -f "$info_max" ] && [ -f "$maxfile" ]; then
        cat "$info_max" > "$maxfile" 2>/dev/null || warn "Could not set $maxfile"
        info "Set $(basename $cpu) max freq -> $(cat $info_max 2>/dev/null)"
      fi
    else
      if [ -f "$info_min" ] && [ -f "$minfile" ]; then
        cat "$info_min" > "$minfile" 2>/dev/null || warn "Could not set $minfile"
        info "Set $(basename $cpu) min freq -> $(cat $info_min 2>/dev/null)"
      fi
    fi
  done
}

set_gpu_mode() {
  local mode=$1
  local devs
  devs=$(ls -d /sys/class/devfreq/* 2>/dev/null || true)
  if [ -z "$devs" ]; then
    warn "No devfreq devices found; GPU may not expose devfreq"
    return
  fi

  for d in $devs; do
    local name
    name=$(basename "$d")
    # Heuristic: operate on devices that look like GPU/Mali
    if ! echo "$name" | grep -Eqi "mali|gpu|g3d|drm|kgsl"; then
      # still allow if governor file exists
      :
    fi

    local govfile="$d/governor"
    local availfile="$d/available_governors"
    if [ ! -f "$govfile" ]; then
      continue
    fi

    if [ "$mode" = "status" ]; then
      printf "devfreq: %s governor=%s\n" "$name" "$(cat $govfile 2>/dev/null || echo unknown)"
      continue
    fi

    local avail=""
    avail=$(cat "$availfile" 2>/dev/null || true)
    if [ -n "$avail" ]; then
      if echo "$avail" | grep -qw "$mode"; then
        echo "$mode" > "$govfile" 2>/dev/null || warn "Could not write $mode to $govfile"
        info "Set $govfile -> $mode"
      else
        if [ "$mode" = "performance" ] && echo "$avail" | grep -qw "performance"; then
          echo performance > "$govfile" 2>/dev/null || warn "Could not write performance to $govfile"
          info "Set $govfile -> performance"
        elif [ "$mode" = "powersave" ] && echo "$avail" | grep -qw "powersave"; then
          echo powersave > "$govfile" 2>/dev/null || warn "Could not write powersave to $govfile"
          info "Set $govfile -> powersave"
        else
          warn "No matching governor for $name; available: $avail"
        fi
      fi
    else
      warn "No available_governors for $name; skipping"
    fi
  done
}

case "$MODE" in
  status)
    info "CPU status:"
    set_cpu_mode status
    info "GPU/devfreq status:"
    set_gpu_mode status
    ;;
  performance|powersave)
    info "Setting CPU -> $MODE"
    set_cpu_mode "$MODE"
    info "Setting GPU -> $MODE"
    set_gpu_mode "$MODE"
    ;;
  *)
    echo "Unknown mode: $MODE"
    echo "Usage: $0 <performance|powersave|status>"
    exit 2
    ;;
esac

info "Done."
