# Quick Wave 2 overhead gate (not a substitute for BenchmarkDotNet).
# Builds a tiny throwaway runner against Sift.Core and compares ProcessSampler vs +PDH.
param([int]$Iterations = 20)

$ErrorActionPreference = 'Stop'
$appsRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$coreProj = Join-Path $appsRoot 'Sift.Core\Sift.Core.csproj'
$work = Join-Path $env:TEMP ("SiftPdhMeasure-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $work | Out-Null
try {
  dotnet build $coreProj -c Release | Out-Host
  if ($LASTEXITCODE -ne 0) { throw 'Sift.Core build failed.' }

  $csproj = @"
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net8.0-windows10.0.19041.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
  </PropertyGroup>
  <ItemGroup>
    <ProjectReference Include="$($coreProj.Replace('\','\\'))" />
  </ItemGroup>
</Project>
"@
  Set-Content -Path (Join-Path $work 'Measure.csproj') -Value $csproj -Encoding UTF8
  $program = @"
using System.Diagnostics;
using Sift.Services;
var n = $Iterations;
var processes = new ProcessSampler();
using var pdh = new PdhSystemSampler();
pdh.TryOpen();
_ = processes.Sample();
_ = pdh.Sample();
Thread.Sleep(150);
_ = pdh.Sample();
static double Avg(Action action, int n)
{
    var sw = Stopwatch.StartNew();
    for (var i = 0; i < n; i++) action();
    sw.Stop();
    return sw.Elapsed.TotalMilliseconds / n;
}
var baseline = Avg(() => processes.Sample(), n);
var combined = Avg(() => { processes.Sample(); pdh.Sample(); }, n);
var delta = baseline <= 0 ? 0 : (combined - baseline) / baseline * 100;
Console.WriteLine($"Baseline ProcessSampler: {baseline:0.00} ms/sample");
Console.WriteLine($"ProcessSampler + PDH:     {combined:0.00} ms/sample");
Console.WriteLine($"Delta:                    {delta:0.0}% (budget <= 15% median)");
Environment.Exit(delta > 15 ? 2 : 0);
"@
  Set-Content -Path (Join-Path $work 'Program.cs') -Value $program -Encoding UTF8
  Push-Location $work
  try {
    dotnet run -c Release
    if ($LASTEXITCODE -eq 2) {
      Write-Warning 'Overhead exceeded the Wave 2 guidance budget on this machine.'
      exit 2
    }
    if ($LASTEXITCODE -ne 0) { throw 'Overhead measurement failed.' }
    Write-Host 'Overhead within Wave 2 guidance budget.'
  }
  finally { Pop-Location }
}
finally {
  Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
}
