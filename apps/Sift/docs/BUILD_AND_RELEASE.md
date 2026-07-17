# Sift build and release

Run commands from the repository root unless noted otherwise.

## Requirements

- Windows 11 or a supported Windows 10 build
- .NET 8 SDK with the Windows workload required by WinUI 3
- PowerShell
- Node.js only when rebuilding Script Studio's bundled web assets
- Windows SDK packaging and signing tools for MSIX work

## Build and validation

```powershell
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate.ps1
```

The complete validation entry point runs the feature-audit reference checks, deterministic unit tests, Core integration checks, the Release build, native UI traversal, screenshot capture, and repository diff checks. Generated evidence is written below `apps/Sift/artifacts/` and is not a substitute for signed-release acceptance.

## Folder release

```powershell
apps\Sift\build-release.ps1
apps\Sift\build-release.ps1 -Versioned
```

The default output is `apps/Sift/dist/Sift`; `-Versioned` uses `Sift-<version>`. Release publishing replaces the owned output directory. A non-empty custom destination is cleaned only when it contains Sift's ownership marker. The folder must contain `ElevationHost/Sift.ElevationHost.exe` and `MonitorHost/Sift.MonitorHost.exe`; neither helper may leak framework-dependent artifacts into the release root.

`BUILD-LATEST.bat` starts a local release build. `REBUILD.bat`, copied beside the published executable, rebuilds that release in place after Sift is closed.

## MSIX layout

Build and verify an unsigned package layout without installing it:

```powershell
apps\Sift\build-msix.ps1 -Unsigned
```

This checks package structure only. Do not distribute or describe the result as a signed release.

The package includes the main app, one-shot elevation helper, and as-invoker monitor host. Its `SiftMonitor` startup task is declared with `Enabled="false"`; monitoring remains an explicit per-user choice.

## Signed package

Use a trusted code-signing certificate whose subject exactly matches the manifest publisher:

```powershell
apps\Sift\build-msix.ps1 `
  -Publisher 'CN=Your Publisher' `
  -CertificateThumbprint 'CERTIFICATE_THUMBPRINT'
```

The signed path verifies all three executables before packaging, then signs and verifies the MSIX. Any failed `SignTool verify` check fails the release gate.

## Clean-account acceptance

On a disposable standard-user account, run:

```powershell
apps\Sift\scripts\validate-clean-account.ps1 `
  -PackagePath .\Sift-0.15.0.0-x64.msix `
  -ExpectedPublisher 'CN=Your Publisher' `
  -InstallRoundTrip
```

The script rejects unsigned or untrusted packages and pre-existing installs. It verifies the installed application, elevation helper, and monitor host, checks that background startup is disabled, performs non-mutating monitor and elevation-channel probes, launches the package, removes it, and verifies that registration and the current-user monitor are gone.

## Version ownership

`apps/Sift/Sift.csproj` owns the product version. `apps/Sift.ElevationHost/Sift.ElevationHost.csproj` and `apps/Sift.MonitorHost/Sift.MonitorHost.csproj` must carry the same version because all three executables ship as one release. The feature-audit validator checks this invariant, and `build-msix.ps1` derives the four-part package version from the application project.
