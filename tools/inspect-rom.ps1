[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$LiteralPath
)

$ErrorActionPreference = 'Stop'
$item = Get-Item -LiteralPath $LiteralPath
$files = if ($item.PSIsContainer) {
    Get-ChildItem -LiteralPath $item.FullName -File -Filter '*.gba' | Sort-Object Name
} else {
    $item
}

foreach ($file in $files) {
    $stream = [IO.File]::OpenRead($file.FullName)
    try {
        $header = New-Object byte[] 192
        if ($stream.Read($header, 0, $header.Length) -ne $header.Length) {
            throw "File is too short for a GBA header: $($file.Name)"
        }
        $code = [Text.Encoding]::ASCII.GetString($header, 172, 4)
        $title = [Text.Encoding]::ASCII.GetString($header, 160, 12).TrimEnd([char]0)
        $family = switch ($code) {
            'BE8E' { 'FE8U candidate; runtime validation required' }
            'BE8J' { 'FE8J; compare exact hash with the profile registry' }
            'BE8P' { 'FE8 European base; exact layout validation required' }
            { $_ -in 'AFEJ', 'AFEE' } { 'FE6; outside FE8 extensions' }
            { $_ -in 'AE7J', 'AE7E', 'AE7P' } { 'FE7; outside FE8 extensions' }
            default { 'Unknown; header alone cannot establish compatibility' }
        }
        $stream.Position = 0
        $hasher = [Security.Cryptography.SHA1]::Create()
        try {
            $digest = [BitConverter]::ToString($hasher.ComputeHash($stream)).Replace('-', '').ToLowerInvariant()
        } finally {
            $hasher.Dispose()
        }
        [PSCustomObject]@{
            Name = $file.Name
            Size = $stream.Length
            Title = $title
            GameCode = $code
            SHA1 = $digest
            ProfileStatus = $family
        }
    } finally {
        $stream.Dispose()
    }
}
