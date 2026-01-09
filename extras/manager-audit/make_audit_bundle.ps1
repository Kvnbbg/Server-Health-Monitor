# “Excellence is not an opinion; it is what survives inspection.”
# L’excellence n’est pas une opinion : c’est ce qui résiste à l’inspection.

$ts = Get-Date -Format "yyyyMMdd-HHmmss"
$outDir = "audit_bundle_$ts"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

Write-Host "[+] Creating audit bundle: $outDir"

$snapshotPath = Join-Path $outDir "snapshot.txt"

@"
=== DATE ===
$(Get-Date)

=== USER ===
$env:USERNAME

=== HOST ===
$env:COMPUTERNAME

=== OS ===
$(Get-CimInstance Win32_OperatingSystem | Select-Object Caption, Version, BuildNumber | Format-List | Out-String)

=== UPTIME ===
$(Get-CimInstance Win32_OperatingSystem | Select-Object LastBootUpTime | Format-List | Out-String)

=== DISK ===
$(Get-PSDrive -PSProvider FileSystem | Select-Object Name, Used, Free | Format-Table -AutoSize | Out-String)

=== NETWORK ADAPTERS ===
$(Get-NetAdapter | Select-Object Name, Status, LinkSpeed | Format-Table -AutoSize | Out-String)

"@ | Out-File -FilePath $snapshotPath -Encoding UTF8

$changeStatement = Join-Path $outDir "CHANGE_STATEMENT.md"
@"
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
"@ | Out-File -FilePath $changeStatement -Encoding UTF8

$managerReadme = Join-Path $outDir "README_MANAGER.md"
@"
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
"@ | Out-File -FilePath $managerReadme -Encoding UTF8

New-Item -ItemType File -Force -Path (Join-Path $outDir "ubuntuboost_output.log") | Out-Null

Write-Host "[+] Done."
Write-Host "Bundle ready: $outDir"
Write-Host "Next: run UbuntuBoost and paste output into $outDir\ubuntuboost_output.log"
