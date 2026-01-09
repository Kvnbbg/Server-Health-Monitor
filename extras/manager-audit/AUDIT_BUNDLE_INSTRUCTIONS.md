# UbuntuBoost — Audit Bundle Instructions (for technicians)

Goal: deliver a minimal, repeatable evidence pack so a manager can validate work quality.

> “A professional does not ask for trust; they provide evidence.”

## Deliverables (required)
- `ubuntuboost_output.log`
- `snapshot_after.txt`
- `CHANGE_STATEMENT.md`

## Deliverables (recommended)
- `snapshot_before.txt`

## 1) Create the bundle folder

Linux/macOS:
```bash
bash extras/manager-audit/make_audit_bundle.sh
```

Windows PowerShell:
```powershell
powershell -ExecutionPolicy Bypass -File extras/manager-audit/make_audit_bundle.ps1
```

This creates:
- `audit_bundle_YYYYMMDD-HHMMSS/`
- `snapshot.txt`
- `CHANGE_STATEMENT.md` (template)
- `ubuntuboost_output.log` (empty placeholder)
- `README_MANAGER.md`

## 2) Capture UbuntuBoost output

Run UbuntuBoost and capture output to a file, for example:

```bash
# example pattern; adjust to your actual entrypoint
python3 run.py | tee audit_bundle_*/ubuntuboost_output.log
```

If UbuntuBoost is interactive, do a run, then copy/paste terminal output into `ubuntuboost_output.log`.

## 3) Snapshot before/after (optional but strong)

If possible, take snapshots before and after changes.
Put them as:
- `snapshot_before.txt`
- `snapshot_after.txt`

Use consistent commands:
- `date`
- `whoami`
- `uname -a`
- OS release info
- `uptime`
- `df -h`
- `free -h`
- `systemctl --failed`
- `ufw status verbose` (if applicable)
- `journalctl -p 3 -xb`

## 4) Change statement (mandatory)

Fill the template in `CHANGE_STATEMENT.md`:
- intent
- actions performed
- actions explicitly NOT performed
- risks & mitigations
- errors
- rollback plan

## 5) Redaction rules
- Never include passwords, tokens, private keys.
- Redact secrets if found in logs before sending the bundle.
- Avoid exposing personally identifiable customer data.

## 6) Manager audit step

Manager pastes the evidence into:
- `extras/manager-audit/PROMPT_MANAGER_AUDIT.md`

Expected outcome:
- structured report
- risk flags
- compliance check
- SOP recommendation
- approve/conditional/reject decision
