# Powershell script to make a release

$README="..\README.md","..\README_ja.md","..\LICENSE"
$CLOG="..\Changelog.md"
$EXEC="LOKI.exe"

$RDIR="LOKI"

# New-Item -ItemType Directory -Force $RDIR *>$null
# Copy-Item $EXEC,$CLOG -Force -Destination $RDIR
# Copy-Item $README -Force -Destination $RDIR
# Rename-Item $RDIR\LICENSE $RDIR\LICENSE.txt
# Get-ChildItem .\$RDIR\*.md | Rename-Item -NewName { $_.name -Replace '\.md$','.txt' }