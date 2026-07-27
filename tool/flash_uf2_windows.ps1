param(
    [Parameter(Mandatory = $true)]
    [string] $Uf2Path,

    [string] $BootloaderUrl = "http://10.63.27.1/api/rebootToUf2",
    [string] $MountLabel = "RPI-RP2",
    [string] $WindowsDrive = "",
    [int] $TimeoutSeconds = 60,
    [switch] $SkipReboot
)

$ErrorActionPreference = "Stop"

function Write-FlashLog {
    param([string] $Message)
    Write-Host "[flash] $Message"
}

function Fail {
    param([string] $Message)
    Write-Error "[flash] ERROR: $Message"
    exit 1
}

function Normalize-DrivePath {
    param([string] $Drive)

    if ([string]::IsNullOrWhiteSpace($Drive)) {
        return ""
    }

    $Drive = $Drive.Trim()
    if ($Drive -match "^[A-Za-z]:$") {
        return "$Drive\"
    }
    if (-not $Drive.EndsWith("\")) {
        return "$Drive\"
    }
    return $Drive
}

function Test-Uf2Drive {
    param([string] $Drive)

    if ([string]::IsNullOrWhiteSpace($Drive)) {
        return $false
    }

    try {
        return Test-Path -LiteralPath (Join-Path $Drive "INFO_UF2.TXT")
    } catch {
        return $false
    }
}

function Find-Uf2Drive {
    param([string] $Label)

    $volume = Get-Volume -FileSystemLabel $Label -ErrorAction SilentlyContinue |
        Where-Object { $_.DriveLetter } |
        Select-Object -First 1

    if ($null -eq $volume) {
        return ""
    }

    return "$($volume.DriveLetter):\"
}

function Copy-Uf2 {
    param(
        [string] $Source,
        [string] $Drive
    )

    $destination = Join-Path $Drive (Split-Path -Leaf $Source)
    Write-FlashLog "copying $(Split-Path -Leaf $Source) to Windows drive $Drive"
    Copy-Item -LiteralPath $Source -Destination $destination -Force
    Start-Sleep -Milliseconds 500
}

if (-not (Test-Path -LiteralPath $Uf2Path)) {
    Fail "UF2 not found: $Uf2Path"
}

$WindowsDrive = Normalize-DrivePath $WindowsDrive

if ($SkipReboot) {
    Write-FlashLog "skip HTTP reboot request"
} else {
    Write-FlashLog "requesting UF2 bootloader: $BootloaderUrl"
    try {
        Invoke-WebRequest -Uri $BootloaderUrl -Method Post -TimeoutSec 2 -UseBasicParsing | Out-Null
    } catch {
        Write-FlashLog "bootloader request failed; waiting for an already-mounted $MountLabel drive"
    }
}

Write-FlashLog "waiting for $MountLabel drive"
$deadline = (Get-Date).AddSeconds($TimeoutSeconds)

while ((Get-Date) -lt $deadline) {
    if (-not [string]::IsNullOrWhiteSpace($WindowsDrive)) {
        if (Test-Uf2Drive $WindowsDrive) {
            Copy-Uf2 -Source $Uf2Path -Drive $WindowsDrive
            Write-FlashLog "done"
            exit 0
        }
    }

    $drive = Find-Uf2Drive $MountLabel
    if (-not [string]::IsNullOrWhiteSpace($drive)) {
        Copy-Uf2 -Source $Uf2Path -Drive $drive
        Write-FlashLog "done"
        exit 0
    }

    Start-Sleep -Seconds 1
}

Fail "timed out waiting for $MountLabel. Put the device in BOOTSEL mode or increase TIMEOUT_SECONDS."
