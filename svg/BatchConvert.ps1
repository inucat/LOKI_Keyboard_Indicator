$data=((Get-ChildItem $pattern).BaseName)

$data.ForEach({
    magick -background transparent "$_.svg" "$_.ico"
})
