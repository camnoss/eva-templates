#!/bin/sh
# Writes this Connection Manager's pin.conf and starts the CM.
#
# pin.conf is built from two parts, in order:
#   1. The stock pin.conf of the CM image, if it has one: the standard FMs,
#      including the policy library this image replaces.
#   2. Overrides from Central Dogma (a ConfigMap): the DM, ports and logging for
#      this environment. Each key replaces the stock entries with the same key.
#      fm_module entries are added, never replaced.
#
# The root filesystem is read-only, so pin.conf and cm.pinlog go to CM_WORKDIR.
set -eu

pin_home="${PIN_HOME:-/oms}"
stock="${CM_STOCK_PIN_CONF:-${pin_home}/sys/cm/pin.conf}"
overrides="${CM_PIN_CONF_OVERRIDES:-/etc/nerv/cm/overrides/pin.conf}"
workdir="${CM_WORKDIR:-/var/run/cm}"

mkdir -p "$workdir"
cd "$workdir"

: > pin.conf

if [ -f "$stock" ]; then
  if [ -f "$overrides" ]; then
    awk 'NR == FNR { if ($1 == "-" && $3 != "fm_module") keys[$2 " " $3] = 1; next }
         !($1 == "-" && (($2 " " $3) in keys))' "$overrides" "$stock" >> pin.conf
  else
    cat "$stock" >> pin.conf
  fi
fi

if [ -f "$overrides" ]; then
  cat "$overrides" >> pin.conf
fi

exec "${CM_COMMAND:-${pin_home}/bin/cm}" "$@"
