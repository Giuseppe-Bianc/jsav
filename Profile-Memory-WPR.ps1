#Requires -Version 5.1
<#
.SYNOPSIS
    Profila l'uso della memoria di un'applicazione usando WPR (Windows Performance Recorder)
    e apre automaticamente il report in WPA (Windows Performance Analyzer).

.DESCRIPTION
    Lo script:
      1. Verifica la presenza di WPR/WPA (Windows ADK)
      2. Avvia WPR con un profilo memoria personalizzato
      3. Avvia (o aggancia) il processo target
      4. Registra per la durata specificata (o finche' l'utente preme un tasto)
      5. Ferma WPR e salva il file .etl
      6. Apre automaticamente WPA con un layout preconfigurato

.PARAMETER AppPath
    Percorso completo dell'eseguibile da profilare.
    Es: -AppPath "C:\MyApp\myapp.exe"

.PARAMETER AppArgs
    Argomenti da passare all'applicazione (opzionale).

.PARAMETER PID
    PID di un processo gia' in esecuzione da agganciare (alternativa ad AppPath).

.PARAMETER OutputDir
    Directory dove salvare i file .etl e i report. Default: Desktop\WPR_Reports

.PARAMETER DurationSec
    Durata della registrazione in secondi. Se 0 (default), aspetta la pressione di un tasto.

.PARAMETER ProfileType
    Tipo di profilo WPR: Memory | Heap | Full. Default: Memory

.EXAMPLE
    # Avvia e profila una nuova istanza
    .\Profile-Memory-WPR.ps1 -AppPath "C:\MyApp\myapp.exe" -DurationSec 30

.EXAMPLE
    # Aggancia un processo gia' in esecuzione
    .\Profile-Memory-WPR.ps1 -PID 1234 -ProfileType Heap

.EXAMPLE
    # Profilo completo con output custom
    .\Profile-Memory-WPR.ps1 -AppPath "C:\MyApp\app.exe" -AppArgs "--config prod" `
        -OutputDir "D:\Profiling" -DurationSec 60 -ProfileType Full

.NOTES
    Requisiti: Windows ADK installato (wpr.exe e wpa.exe nel PATH o in %ProgramFiles(x86)%\Windows Kits\10\)
    Download ADK: https://learn.microsoft.com/windows-hardware/get-started/adk-install
    Eseguire come Amministratore per profili Heap e Full.
#>

[CmdletBinding(DefaultParameterSetName = 'ByPath')]
param(
    [Parameter(ParameterSetName = 'ByPath', Mandatory = $true)]
    [ValidateScript({ Test-Path $_ -PathType Leaf })]
    [string]$AppPath,

    [Parameter(ParameterSetName = 'ByPath')]
    [string]$AppArgs = "",

    [Parameter(ParameterSetName = 'ByPID', Mandatory = $true)]
    [int]$ProcessId,

    [string]$OutputDir = "$env:USERPROFILE\Desktop\WPR_Reports",

    [ValidateRange(0, 3600)]
    [int]$DurationSec = 0,

    [ValidateSet("Memory", "Heap", "Full")]
    [string]$ProfileType = "Memory"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ─────────────────────────────────────────────────────────────
#  COSTANTI E COLORI
# ─────────────────────────────────────────────────────────────
$SEPARATOR = "=" * 65
$COLOR_OK    = "Green"
$COLOR_WARN  = "Yellow"
$COLOR_ERR   = "Red"
$COLOR_INFO  = "Cyan"
$COLOR_HEAD  = "Magenta"

function Write-Header($msg) {
    Write-Host ""
    Write-Host $SEPARATOR -ForegroundColor $COLOR_HEAD
    Write-Host "  $msg" -ForegroundColor $COLOR_HEAD
    Write-Host $SEPARATOR -ForegroundColor $COLOR_HEAD
}

function Write-Step($n, $msg) {
    Write-Host ""
    Write-Host "  [$n] $msg" -ForegroundColor $COLOR_INFO
}

function Write-OK($msg)   { Write-Host "      [OK] $msg" -ForegroundColor $COLOR_OK }
function Write-Warn($msg) { Write-Host "      [!!] $msg" -ForegroundColor $COLOR_WARN }
function Write-Err($msg)  { Write-Host "      [ERR] $msg" -ForegroundColor $COLOR_ERR }

# ─────────────────────────────────────────────────────────────
#  BANNER
# ─────────────────────────────────────────────────────────────
Clear-Host
Write-Host @"

  ╔══════════════════════════════════════════════════════════╗
  ║       MEMORY PROFILER  —  WPR + WPA  (Windows ADK)      ║
  ║              Profilo: $($ProfileType.PadRight(10)) | PowerShell $($PSVersionTable.PSVersion.Major).$($PSVersionTable.PSVersion.Minor)         ║
  ╚══════════════════════════════════════════════════════════╝

"@ -ForegroundColor Cyan

# ─────────────────────────────────────────────────────────────
#  STEP 1 — Trova WPR e WPA
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 1 — Verifica Windows ADK"

$adkSearchPaths = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10\Windows Performance Toolkit",
    "${env:ProgramFiles}\Windows Kits\10\Windows Performance Toolkit",
    "C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit",
    "C:\Program Files\Windows Kits\10\Windows Performance Toolkit"
)

$wprExe = $null
$wpaExe = $null

# Prima cerca nel PATH
$fromPath = Get-Command wpr.exe -ErrorAction SilentlyContinue
if ($fromPath) {
    $wprExe = $fromPath.Source
    $wpaDir = Split-Path $wprExe
    $wpaExe = Join-Path $wpaDir "wpa.exe"
}

# Altrimenti cerca nelle directory ADK
if (-not $wprExe) {
    foreach ($p in $adkSearchPaths) {
        $candidate = Join-Path $p "wpr.exe"
        if (Test-Path $candidate) {
            $wprExe = $candidate
            $wpaExe = Join-Path $p "wpa.exe"
            break
        }
    }
}

if (-not $wprExe -or -not (Test-Path $wprExe)) {
    Write-Err "wpr.exe non trovato!"
    Write-Host ""
    Write-Host "  Installa Windows ADK da:" -ForegroundColor Yellow
    Write-Host "  https://learn.microsoft.com/windows-hardware/get-started/adk-install" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  Seleziona: 'Windows Performance Toolkit'" -ForegroundColor Yellow
    exit 1
}

Write-OK "WPR trovato: $wprExe"

if (Test-Path $wpaExe) {
    Write-OK "WPA trovato: $wpaExe"
} else {
    Write-Warn "WPA non trovato in $wpaExe — apriremo il file .etl manualmente"
    $wpaExe = $null
}

# Verifica privilegi Amministratore per profili avanzati
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator
)
if ($ProfileType -in @("Heap", "Full") -and -not $isAdmin) {
    Write-Warn "Il profilo '$ProfileType' richiede privilegi Amministratore."
    Write-Warn "Riavvia PowerShell come Amministratore per risultati completi."
}

# ─────────────────────────────────────────────────────────────
#  STEP 2 — Prepara directory output e nomi file
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 2 — Preparazione directory output"

$timestamp  = Get-Date -Format "yyyyMMdd_HHmmss"
$sessionDir = Join-Path $OutputDir $timestamp

New-Item -ItemType Directory -Path $sessionDir -Force | Out-Null
Write-OK "Directory creata: $sessionDir"

$etlFile     = Join-Path $sessionDir "memory_profile_$timestamp.etl"
$summaryFile = Join-Path $sessionDir "summary_$timestamp.txt"
$wpaLayout   = Join-Path $sessionDir "layout_$timestamp.wpaProfile"

Write-OK "File ETL destinazione: $etlFile"

# ─────────────────────────────────────────────────────────────
#  STEP 3 — Crea profilo WPR personalizzato (XML)
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 3 — Generazione profilo WPR ($ProfileType)"

# Mappa profilo → provider WPR builtin
$builtinProfiles = switch ($ProfileType) {
    "Memory" { @("MemoryDiagnostics", "VirtualAllocation") }
    "Heap"   { @("HeapSnapshot", "MemoryDiagnostics", "VirtualAllocation") }
    "Full"   { @("HeapSnapshot", "MemoryDiagnostics", "VirtualAllocation",
                  "CpuUsageCallstacks", "DiskIO", "FileIO") }
}

# Costruisci argomenti -start per WPR
$wprStartArgs = $builtinProfiles | ForEach-Object { "-start $_" }
$wprStartLine = $wprStartArgs -join " "

Write-OK "Provider attivati: $($builtinProfiles -join ', ')"

# Crea layout WPA minimalista per analisi memoria
$wpaLayoutXml = @"
<?xml version="1.0" encoding="utf-8"?>
<WpaProfileContainer xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                     Version="2" xmlns="http://tempuri.org/SerializableElement.xsd">
  <Content xsi:type="WpaProfile2">
    <Sessions>
      <Session Index="0">
        <FileReferences />
      </Session>
    </Sessions>
    <Views>
      <View Guid="mem-virt-alloc" IsVisible="true" Title="Virtual Memory Allocations">
        <Graphs>
          <Graph Guid="a669df53-d8b6-4f5c-b7a8-c88f3949ef22" LayoutStyle="DataTable"
                 Color="#FF0099BC" GraphHeight="200" IsActive="true">
            <Preset Name="Commit by Process" GraphColumnCount="4" />
          </Graph>
        </Graphs>
      </View>
      <View Guid="mem-heap" IsVisible="true" Title="Heap Allocations">
        <Graphs>
          <Graph Guid="93cd6a3c-7fd5-44ad-b6fc-f42eba6f8ae9" LayoutStyle="DataTable"
                 Color="#FF4CAF50" GraphHeight="200" IsActive="true">
            <Preset Name="Heap Allocations by Process" GraphColumnCount="4" />
          </Graph>
        </Graphs>
      </View>
      <View Guid="mem-resident" IsVisible="true" Title="Resident Set (Working Set)">
        <Graphs>
          <Graph Guid="3a8f2a58-78e7-4bf4-b5b2-5e4bbd9c69db" LayoutStyle="DataTable"
                 Color="#FFFF9800" GraphHeight="200" IsActive="true">
            <Preset Name="Working Set by Process" GraphColumnCount="4" />
          </Graph>
        </Graphs>
      </View>
    </Views>
  </Content>
</WpaProfileContainer>
"@

$wpaLayoutXml | Out-File -FilePath $wpaLayout -Encoding UTF8
Write-OK "Layout WPA salvato: $wpaLayout"

# ─────────────────────────────────────────────────────────────
#  STEP 4 — Avvia processo target (se ByPath)
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 4 — Processo target"

$targetProcess = $null

if ($PSCmdlet.ParameterSetName -eq 'ByPID') {
    $targetProcess = Get-Process -Id $ProcessId -ErrorAction SilentlyContinue
    if (-not $targetProcess) {
        Write-Err "Nessun processo con PID $ProcessId trovato."
        exit 1
    }
    Write-OK "Agganciato processo: $($targetProcess.Name) (PID $ProcessId)"
    $appLaunched = $false
} else {
    Write-Step "4a" "Avvio applicazione: $AppPath $AppArgs"
    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName  = $AppPath
    $startInfo.Arguments = $AppArgs
    $startInfo.UseShellExecute = $true
    $targetProcess = [System.Diagnostics.Process]::Start($startInfo)
    $appLaunched = $true

    Start-Sleep -Milliseconds 800  # lascia inizializzare l'app
    if ($targetProcess.HasExited) {
        Write-Err "Il processo e' terminato subito dopo l'avvio (exit code $($targetProcess.ExitCode))."
        exit 1
    }
    Write-OK "Processo avviato: $($targetProcess.ProcessName) (PID $($targetProcess.Id))"
}

$targetPID  = $targetProcess.Id
$targetName = $targetProcess.ProcessName

# ─────────────────────────────────────────────────────────────
#  STEP 5 — Avvia WPR
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 5 — Avvio registrazione WPR"

# Ferma eventuali sessioni WPR orfane
Write-Step "5a" "Pulizia sessioni WPR precedenti..."
& $wprExe -cancel 2>$null | Out-Null

# Avvia WPR con tutti i profili selezionati
$wprArgs = ($builtinProfiles | ForEach-Object { "-start $_" }) + @("-filemode")
Write-Step "5b" "Avvio WPR con: $($wprArgs -join ' ')"

$wprProc = Start-Process -FilePath $wprExe `
    -ArgumentList $wprArgs `
    -NoNewWindow -Wait -PassThru

if ($wprProc.ExitCode -ne 0) {
    Write-Err "WPR -start fallito (exit code $($wprProc.ExitCode))."
    Write-Warn "Suggerimenti:"
    Write-Warn "  - Esegui come Amministratore"
    Write-Warn "  - Verifica che non ci siano sessioni WPR attive: wpr -status"
    if ($appLaunched) { Stop-Process -Id $targetPID -Force -ErrorAction SilentlyContinue }
    exit 1
}

$recordStart = Get-Date
Write-OK "Registrazione avviata alle $($recordStart.ToString('HH:mm:ss'))"

# ─────────────────────────────────────────────────────────────
#  STEP 6 — Monitoraggio live mentre WPR registra
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 6 — Monitoraggio in corso (PID $targetPID — $targetName)"

function Get-ProcessMemoryInfo($pid) {
    $proc = Get-Process -Id $pid -ErrorAction SilentlyContinue
    if (-not $proc) { return $null }
    return [PSCustomObject]@{
        WorkingSetMB   = [math]::Round($proc.WorkingSet64   / 1MB, 1)
        PrivateBytesMB = [math]::Round($proc.PrivateMemorySize64 / 1MB, 1)
        VirtualMB      = [math]::Round($proc.VirtualMemorySize64 / 1MB, 1)
        HandleCount    = $proc.HandleCount
        ThreadCount    = $proc.Threads.Count
        CPU            = [math]::Round($proc.CPU, 1)
    }
}

$samples = [System.Collections.Generic.List[PSObject]]::new()
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

if ($DurationSec -gt 0) {
    Write-Host ""
    Write-Host "  Registrazione per $DurationSec secondi..." -ForegroundColor Yellow
    Write-Host "  (premi Ctrl+C per interrompere anticipatamente)" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host ("  {0,-8} {1,12} {2,14} {3,12} {4,8}" -f "Sec", "Working Set", "Private Bytes", "Virtual", "Handles") -ForegroundColor DarkCyan
    Write-Host ("  " + "-" * 60) -ForegroundColor DarkGray

    while ($stopwatch.Elapsed.TotalSeconds -lt $DurationSec) {
        $mem = Get-ProcessMemoryInfo $targetPID
        if (-not $mem) {
            Write-Warn "Processo $targetPID terminato durante la registrazione."
            break
        }

        $elapsed = [math]::Round($stopwatch.Elapsed.TotalSeconds, 0)
        $samples.Add([PSCustomObject]@{ Sec=$elapsed; WS=$mem.WorkingSetMB; PB=$mem.PrivateBytesMB; VM=$mem.VirtualMB; H=$mem.HandleCount })

        Write-Host ("  {0,-8} {1,10} MB {2,12} MB {3,10} MB {4,8}" -f $elapsed, $mem.WorkingSetMB, $mem.PrivateBytesMB, $mem.VirtualMB, $mem.HandleCount)

        Start-Sleep -Seconds 2
    }
} else {
    Write-Host ""
    Write-Host "  Registrazione attiva. Premi [INVIO] per fermare..." -ForegroundColor Yellow
    Write-Host ""
    Write-Host ("  {0,-8} {1,12} {2,14} {3,12} {4,8}" -f "Sec", "Working Set", "Private Bytes", "Virtual", "Handles") -ForegroundColor DarkCyan
    Write-Host ("  " + "-" * 60) -ForegroundColor DarkGray

    # Monitoraggio in background finche' l'utente preme invio
    $job = Start-Job -ScriptBlock {
        param($pid2)
        while ($true) {
            $p = Get-Process -Id $pid2 -ErrorAction SilentlyContinue
            if (-not $p) { break }
            [PSCustomObject]@{
                WS = [math]::Round($p.WorkingSet64/1MB,1)
                PB = [math]::Round($p.PrivateMemorySize64/1MB,1)
                VM = [math]::Round($p.VirtualMemorySize64/1MB,1)
                H  = $p.HandleCount
                T  = (Get-Date)
            }
            Start-Sleep -Seconds 2
        }
    } -ArgumentList $targetPID

    $lastCount = 0
    while (-not [Console]::KeyAvailable) {
        $newResults = Receive-Job $job
        foreach ($r in $newResults) {
            $elapsed = [math]::Round($stopwatch.Elapsed.TotalSeconds, 0)
            $samples.Add([PSCustomObject]@{ Sec=$elapsed; WS=$r.WS; PB=$r.PB; VM=$r.VM; H=$r.H })
            Write-Host ("  {0,-8} {1,10} MB {2,12} MB {3,10} MB {4,8}" -f $elapsed, $r.WS, $r.PB, $r.VM, $r.H)
        }
        Start-Sleep -Milliseconds 500
    }
    [void][Console]::ReadKey($true)
    Stop-Job  $job | Out-Null
    Remove-Job $job | Out-Null
}

$recordEnd      = Get-Date
$totalDurationSec = [math]::Round($stopwatch.Elapsed.TotalSeconds, 1)
$stopwatch.Stop()

Write-Host ""
Write-OK "Registrazione terminata alle $($recordEnd.ToString('HH:mm:ss')) (durata: ${totalDurationSec}s)"

# ─────────────────────────────────────────────────────────────
#  STEP 7 — Ferma WPR e salva ETL
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 7 — Salvataggio traccia ETL"

Write-Step "7a" "Arresto WPR e scrittura: $etlFile"
Write-Host "  (operazione lenta per file grandi — attendere...)" -ForegroundColor DarkGray

$stopArgs = @("-stop", $etlFile)
$wprStop  = Start-Process -FilePath $wprExe -ArgumentList $stopArgs -NoNewWindow -Wait -PassThru

if ($wprStop.ExitCode -ne 0) {
    Write-Err "WPR -stop fallito (exit code $($wprStop.ExitCode))."
    Write-Warn "La traccia potrebbe essere parzialmente salvata."
} else {
    $etlSize = if (Test-Path $etlFile) { [math]::Round((Get-Item $etlFile).Length / 1MB, 1) } else { "?" }
    Write-OK "File ETL salvato: $etlFile ($etlSize MB)"
}

# ─────────────────────────────────────────────────────────────
#  STEP 8 — Statistiche campionate e file di riepilogo
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 8 — Statistiche sessione"

if ($samples.Count -gt 0) {
    $maxWS  = ($samples | Measure-Object -Property WS -Maximum).Maximum
    $minWS  = ($samples | Measure-Object -Property WS -Minimum).Minimum
    $avgWS  = [math]::Round(($samples | Measure-Object -Property WS -Average).Average, 1)
    $maxPB  = ($samples | Measure-Object -Property PB -Maximum).Maximum
    $avgPB  = [math]::Round(($samples | Measure-Object -Property PB -Average).Average, 1)
    $maxH   = ($samples | Measure-Object -Property H  -Maximum).Maximum
    $delta  = [math]::Round($maxWS - $minWS, 1)

    $statsTable = @"

  Processo     : $targetName (PID $targetPID)
  Durata       : ${totalDurationSec}s  |  Campioni: $($samples.Count)

  ┌──────────────────────────────────────────────┐
  │  WORKING SET (RAM fisica)                    │
  │    Min : $($minWS.ToString().PadLeft(8)) MB                         │
  │    Max : $($maxWS.ToString().PadLeft(8)) MB                         │
  │    Avg : $($avgWS.ToString().PadLeft(8)) MB                         │
  │    ΔMax: $($delta.ToString().PadLeft(8)) MB  ← crescita sospetta?   │
  ├──────────────────────────────────────────────┤
  │  PRIVATE BYTES (heap + stack)                │
  │    Max : $($maxPB.ToString().PadLeft(8)) MB                         │
  │    Avg : $($avgPB.ToString().PadLeft(8)) MB                         │
  ├──────────────────────────────────────────────┤
  │  HANDLE COUNT                                │
  │    Max : $($maxH.ToString().PadLeft(8))                             │
  └──────────────────────────────────────────────┘
"@
    Write-Host $statsTable -ForegroundColor Cyan

    # Segnali di possibile memory leak
    Write-Host "  Analisi rapida:" -ForegroundColor Yellow
    if ($delta -gt 100) {
        Write-Host "  ⚠  Working Set cresciuto di ${delta} MB — possibile memory leak!" -ForegroundColor Red
    } elseif ($delta -gt 30) {
        Write-Host "  ⚡ Working Set cresciuto di ${delta} MB — monitorare nel tempo" -ForegroundColor Yellow
    } else {
        Write-Host "  ✓  Working Set stabile (delta ${delta} MB)" -ForegroundColor Green
    }

    if ($maxH -gt 10000) {
        Write-Host "  ⚠  Handle count molto alto ($maxH) — possibile handle leak!" -ForegroundColor Red
    }

    # Salva riepilogo + CSV campioni
    $summaryContent = @"
====================================================
  MEMORY PROFILING REPORT — $($recordStart.ToString('yyyy-MM-dd HH:mm:ss'))
====================================================
Applicazione : $targetName
PID          : $targetPID
Profilo WPR  : $ProfileType
Durata       : ${totalDurationSec}s
File ETL     : $etlFile
====================================================
STATISTICHE WORKING SET
  Min  : $minWS MB
  Max  : $maxWS MB
  Avg  : $avgWS MB
  Delta: $delta MB

STATISTICHE PRIVATE BYTES
  Max  : $maxPB MB
  Avg  : $avgPB MB

HANDLE COUNT MAX : $maxH
====================================================
CAMPIONI (ogni 2s)
$(($samples | Format-Table -AutoSize | Out-String))
====================================================
"@
    $summaryContent | Out-File -FilePath $summaryFile -Encoding UTF8
    Write-OK "Riepilogo salvato: $summaryFile"

    # CSV per grafici Excel/Python
    $csvFile = Join-Path $sessionDir "samples_$timestamp.csv"
    $samples | Export-Csv -Path $csvFile -NoTypeInformation -Encoding UTF8
    Write-OK "Campioni CSV salvati: $csvFile"
}

# ─────────────────────────────────────────────────────────────
#  STEP 9 — Apri WPA
# ─────────────────────────────────────────────────────────────
Write-Header "STEP 9 — Apertura Windows Performance Analyzer"

if ($wpaExe -and (Test-Path $etlFile)) {
    Write-Step "9a" "Apertura WPA con traccia ETL..."

    $wpaArgs = @("`"$etlFile`"")
    if (Test-Path $wpaLayout) {
        $wpaArgs += @("-profile", "`"$wpaLayout`"")
        Write-OK "Layout preconfigurato applicato: $wpaLayout"
    }

    Start-Process -FilePath $wpaExe -ArgumentList $wpaArgs
    Write-OK "WPA avviato. In WPA vai su:"
    Write-Host ""
    Write-Host "    Graph Explorer → Memory → " -ForegroundColor DarkCyan -NoNewline
    Write-Host "Virtual Memory Snapshots" -ForegroundColor White
    Write-Host "    Graph Explorer → Memory → " -ForegroundColor DarkCyan -NoNewline
    Write-Host "Heap Allocations (se profilo Heap/Full)" -ForegroundColor White
    Write-Host "    Graph Explorer → Memory → " -ForegroundColor DarkCyan -NoNewline
    Write-Host "Outstanding Allocations" -ForegroundColor White
    Write-Host ""
    Write-Host "    Filtra per processo: " -ForegroundColor DarkCyan -NoNewline
    Write-Host $targetName -ForegroundColor White
} elseif (Test-Path $etlFile) {
    Write-Warn "WPA non trovato. Apri manualmente il file:"
    Write-Host "    $etlFile" -ForegroundColor Yellow
    Write-Host "  con Windows Performance Analyzer (wpa.exe)" -ForegroundColor DarkGray
} else {
    Write-Warn "File ETL non trovato — la registrazione potrebbe non aver avuto successo."
}

# ─────────────────────────────────────────────────────────────
#  STEP 10 — Riepilogo finale e prossimi passi
# ─────────────────────────────────────────────────────────────
Write-Header "COMPLETATO — File nella sessione"

Write-Host ""
$files = Get-ChildItem $sessionDir | ForEach-Object {
    $size = if ($_.Length -gt 1MB) { "$([math]::Round($_.Length/1MB,1)) MB" }
            elseif ($_.Length -gt 1KB) { "$([math]::Round($_.Length/1KB,1)) KB" }
            else { "$($_.Length) B" }
    "  $($_.Name.PadRight(45)) $size"
}
$files | ForEach-Object { Write-Host $_ -ForegroundColor Gray }

Write-Host ""
Write-Host "  Directory: " -ForegroundColor DarkGray -NoNewline
Write-Host $sessionDir -ForegroundColor White
Write-Host ""

# Apri la directory
$openDir = Read-Host "  Vuoi aprire la directory dei risultati? [S/n]"
if ($openDir -ne 'n' -and $openDir -ne 'N') {
    Start-Process explorer.exe $sessionDir
}

Write-Host ""
Write-Host "  PROSSIMI PASSI IN WPA:" -ForegroundColor Magenta
Write-Host "  1. Trascina 'Virtual Memory Snapshots' nel pannello di analisi" -ForegroundColor Gray
Write-Host "  2. Raggruppa per: Process → Allocation Stack" -ForegroundColor Gray
Write-Host "  3. Ordina per 'Size (MB)' decrescente" -ForegroundColor Gray
Write-Host "  4. Usa 'Commit Charge by Process' per vedere chi consuma piu' memoria" -ForegroundColor Gray
Write-Host "  5. Per leak: abilita 'Outstanding Allocations' e filtra per il tuo PID" -ForegroundColor Gray
Write-Host ""
Write-Host $SEPARATOR -ForegroundColor DarkGray
Write-Host ""