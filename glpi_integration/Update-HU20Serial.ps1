# ==============================================================================
# HU20 Reader Serial Collection and Integration Script for GLPI
# ==============================================================================

# 1. Path to the C++ executable
$ExePath = "C:\Program Files\LeitoraHU20\LeitoraHU20.exe"

# 2. Windows Registry path definition (HKLM)
$RegistryPath = "HKLM:\SOFTWARE\CustomInventory\HU20"
$ValueName    = "SerialNumber"

try {
    # Check if the executable exists
    if (-not (Test-Path -Path $ExePath)) {
        Write-Warning "Executable not found at: $ExePath"
        exit 1
    }

    # Execute the C++ program and capture all output lines (stdout and stderr)
    $Output = & $ExePath 2>&1

    # Use Regular Expression (Regex) to ignore logs/errors and capture the value after "Serial:"
    $SerialClean = $null
    foreach ($line in $Output) {
        if ($line -match 'Serial:\s*([A-Za-z0-9]+)') {
            $SerialClean = $Matches[1].Trim()
            break
        }
    }

    # Fallback: If not found in "Serial:" pattern, try "Serial decoded:" pattern
    if (-not $SerialClean) {
        foreach ($line in $Output) {
            if ($line -match 'Serial decoded:\s*([A-Za-z0-9]+)') {
                $SerialClean = $Matches[1].Trim()
                break
            }
        }
    }

    # Validate if the serial number was extracted correctly
    if ([string]::IsNullOrWhiteSpace($SerialClean)) {
        Write-Warning "Unable to extract the serial number from the executable output."
        exit 1
    }

    # Create the Registry key if it does not exist yet
    if (-not (Test-Path -Path $RegistryPath)) {
        New-Item -Path $RegistryPath -Force | Out-Null
    }

    # Write the exact serial number to the Registry (REG_SZ)
    Set-ItemProperty -Path $RegistryPath -Name $ValueName -Value $SerialClean -Type String -Force

    Write-Host "Success: Serial '$SerialClean' captured and written to $RegistryPath\$ValueName"
}
catch {
    Write-Error "Error processing HU20 reader: $_"
    exit 1
}
