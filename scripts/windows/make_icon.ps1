# Makes a Windows .ico from a PNG (the icon0.png inside the user's own game dump). The icon is
# made on the user's PC and is never part of the repository or of a release.
# Usage:  powershell -ExecutionPolicy Bypass -File make_icon.ps1 -Png <icon0.png> -Out <file.ico>
# Exit code 0 when the .ico was written.
param(
    [Parameter(Mandatory = $true)][string]$Png,
    [Parameter(Mandatory = $true)][string]$Out
)
$ErrorActionPreference = 'Stop'
try {
    Add-Type -AssemblyName System.Drawing
    $sizes = 256, 64, 48, 32, 16
    $source = [System.Drawing.Image]::FromFile((Resolve-Path $Png))
    $images = foreach ($size in $sizes) {
        $bitmap = New-Object System.Drawing.Bitmap $size, $size
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        $graphics.InterpolationMode = 'HighQualityBicubic'
        $graphics.DrawImage($source, 0, 0, $size, $size)
        $graphics.Dispose()
        $stream = New-Object System.IO.MemoryStream
        $bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
        $bitmap.Dispose()
        , $stream.ToArray()
    }
    $source.Dispose()
    # ICO container: header, one 16-byte entry per size, then the PNG images.
    $ico = New-Object System.IO.MemoryStream
    $writer = New-Object System.IO.BinaryWriter $ico
    $writer.Write([uint16]0); $writer.Write([uint16]1); $writer.Write([uint16]$images.Count)
    $offset = 6 + 16 * $images.Count
    for ($i = 0; $i -lt $images.Count; $i++) {
        $writer.Write([byte]($sizes[$i] % 256)); $writer.Write([byte]($sizes[$i] % 256))
        $writer.Write([byte]0); $writer.Write([byte]0)
        $writer.Write([uint16]1); $writer.Write([uint16]32)
        $writer.Write([uint32]$images[$i].Length); $writer.Write([uint32]$offset)
        $offset += $images[$i].Length
    }
    foreach ($image in $images) { $writer.Write($image) }
    $writer.Flush()
    $folder = Split-Path -Parent $Out
    if ($folder -and -not (Test-Path $folder)) { New-Item -ItemType Directory -Force $folder | Out-Null }
    [System.IO.File]::WriteAllBytes($Out, $ico.ToArray())
    exit 0
} catch {
    Write-Host "make_icon: $($_.Exception.Message)"
    exit 1
}
