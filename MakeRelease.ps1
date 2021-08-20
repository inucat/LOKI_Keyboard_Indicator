$README="README.md","README_ja.md","LICENSE"
$CLOG="Changelog.md"
$RDIR="LOKI"
$EXEC="loki.exe"
# $FILES=($README)

Copy-Item $EXEC,$CLOG -Force -Destination $RDIR
Copy-Item $README -Force -Destination $RDIR
