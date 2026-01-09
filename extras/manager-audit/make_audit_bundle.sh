#!/usr/bin/env bash
set -euo pipefail

# “Excellence is not an opinion; it is what survives inspection.”
# L’excellence n’est pas une opinion : c’est ce qui résiste à l’inspection.

TS="$(date +%Y%m%d-%H%M%S)"
OUT_DIR="audit_bundle_${TS}"
mkdir -p "$OUT_DIR"

echo "[+] Creating audit bundle: $OUT_DIR"

# Snapshot (manager-friendly, evidence-oriented)
{
  echo "=== DATE ==="; date
  echo
  echo "=== USER ==="; whoami
  echo
  echo "=== HOST ==="; (hostname 2>/dev/null || true)
  echo
  echo "=== OS ==="
  (lsb_release -a 2>/dev/null || cat /etc/os-release 2>/dev/null || true)
  echo
  echo "=== KERNEL ==="; uname -a
  echo
  echo "=== UPTIME ==="; (uptime 2>/dev/null || true)
  echo
  echo "=== DISK ==="; (df -h 2>/dev/null || true)
  echo
  echo "=== MEMORY ==="; (free -h 2>/dev/null || true)
  echo
  echo "=== FAILED SERVICES ==="
  (sudo systemctl --failed --no-pager 2>/dev/null || systemctl --failed --no-pager 2>/dev/null || true)
  echo
  echo "=== UFW (if present) ==="
  (sudo ufw status verbose 2>/dev/null || ufw status verbose 2>/dev/null || echo "UFW not available")
  echo
  echo "=== CRITICAL ERRORS (journalctl) ==="
  (sudo journalctl -p 3 -xb --no-pager 2>/dev/null | tail -n 200 || true)
} > "${OUT_DIR}/snapshot.txt"

# Change statement template
cat > "${OUT_DIR}/CHANGE_STATEMENT.md" <<'EOS'
# Change Statement (Mandatory)

## 1) Intent (what I wanted to achieve)
- 

## 2) Environment
- Host / Asset ID:
- Environment: Dev / Staging / Prod
- Maintenance window:
- Constraints (what I was NOT allowed to do):

## 3) Actions performed (what I actually did)
- 

## 4) Actions NOT performed (explicitly)
- 

## 5) Risks assessed (and mitigations)
- Risk:
  - Mitigation:

## 6) Evidence provided
- UbuntuBoost log file: ubuntuboost_output.log
- Snapshot after: snapshot_after.txt
- Snapshot before (if provided): snapshot_before.txt

## 7) Errors encountered (if any)
- 

## 8) Rollback plan
- 

## 9) Validation checks performed
- Service status checks:
- Connectivity checks:
- Disk/memory checks:
- Security checks (if applicable):
EOS

# Manager readme
cat > "${OUT_DIR}/README_MANAGER.md" <<'EOS'
# Manager Evidence Pack

Purpose: manager-friendly audit evidence for work performed.

## Contents
- snapshot.txt
- CHANGE_STATEMENT.md
- ubuntuboost_output.log (paste UbuntuBoost run output here)
- snapshot_before.txt (optional)
- snapshot_after.txt (optional)

## Next step (Manager)
Paste the evidence into:
extras/manager-audit/PROMPT_MANAGER_AUDIT.md
EOS

# Placeholders
touch "${OUT_DIR}/ubuntuboost_output.log"

echo "[+] Done."
echo "Bundle ready: ${OUT_DIR}/"
echo "Next: run UbuntuBoost and paste output into ${OUT_DIR}/ubuntuboost_output.log"
