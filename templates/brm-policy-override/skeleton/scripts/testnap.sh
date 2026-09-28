#!/usr/bin/env bash
# Calls an opcode on this service's Connection Manager in dev with testnap.
#
# Usage: scripts/testnap.sh <opcode> <flist-file> [flags]
#   opcode  a PCM_OP_* name or a number
#
# Example:
#   scripts/testnap.sh PCM_OP_CUST_POL_VALIDATE test/flists/cust_pol_validate/basic.flist
#
# testnap runs inside the CM pod and connects to the CM on localhost, so this
# only needs kubectl access to the <service>-dev namespace.
set -euo pipefail

cd "$(dirname "$0")/.."

if [ $# -lt 2 ]; then
  sed -n '2,11p' "$0"
  exit 2
fi

opcode="$1"
flist="$2"
flags="${3:-0}"

service=$(sed -n 's/^  name: //p' catalog-info.yaml | head -n 1)
namespace="${NAMESPACE:-${service}-dev}"

commands="r << XX 1
$(cat "$flist")
XX
xop ${opcode} ${flags} 1
"

# The nap pin.conf logs in with the CM's credentials from the mounted wallet.
kubectl -n "$namespace" exec -i "deploy/${service}" -c cm -- sh -c '
  set -e
  dir=$(mktemp -d "${CM_WORKDIR:-/var/run/cm}/testnap.XXXXXX")
  trap "rm -rf \"$dir\"" EXIT
  cd "$dir"
  cat > pin.conf <<EOF
- nap cm_ptr ip localhost 11960
- nap login_type 1
- nap login_name root.0.0.0.1
- - userid 0.0.0.1 /service/pcm_client 1
EOF
  "${PIN_HOME:-/oms}/bin/testnap"
' <<< "$commands"
