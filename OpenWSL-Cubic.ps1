# ╔══════════════════════════════════════════╗
# ║        Cubic WSL Launcher - Win 11       ║
# ╚══════════════════════════════════════════╝

[CmdletBinding()]
param(
	[Parameter(Mandatory = $false)]
	[string]$Distribution = $env:WSL_DISTRO_NAME
)

# Launch WSL in the current PowerShell working directory with diagnostics.
$workingDirectory = $PWD.Path
$targetDistribution = if ([string]::IsNullOrWhiteSpace($Distribution)) {
	'<default>'
}
else {
	$Distribution
}

$commandString = if ($targetDistribution -eq '<default>') {
	"wsl --cd $workingDirectory"
}
else {
	"wsl -d $targetDistribution --cd $workingDirectory"
}

Write-Verbose "Current working directory: $workingDirectory"
Write-Verbose "Target distribution: $targetDistribution"
Write-Verbose "WSL command: $commandString"

if ($targetDistribution -eq '<default>') {
	& wsl --cd "$workingDirectory"
}
else {
	& wsl -d "$targetDistribution" --cd "$workingDirectory"
}

$exitCode = $LASTEXITCODE
Write-Verbose "WSL exit code: $exitCode"

if ($exitCode -ne 0) {
	Write-Error "WSL command failed with exit code $exitCode."
}

exit $exitCode
