using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using Sift.Models;

namespace Sift.Services;

public interface IInstalledAppTrustInspector
{
    InstalledAppTrustReport Inspect(InstalledApp app, CancellationToken cancellationToken = default);
}

public sealed class InstalledAppTrustInspector(IInstalledAppInventory inventory) : IInstalledAppTrustInspector
{
    public InstalledAppTrustReport Inspect(InstalledApp app, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var current = inventory.FindExact(app.RegistryLocation);
        if (current is null)
            return Unavailable("The registration disappeared before signature inspection.");
        if (!string.Equals(current.DisplayName, app.DisplayName, StringComparison.Ordinal) ||
            !string.Equals(current.UninstallString, app.UninstallString, StringComparison.Ordinal))
            return Unavailable("The registration changed before signature inspection. Refresh first.");
        if (!InstalledAppPolicy.TryParseUninstallCommand(current.UninstallString, out var plan, out var reason) || plan is null)
            return Unavailable(reason);

        cancellationToken.ThrowIfCancellationRequested();
        var isMsi = Path.GetFileName(plan.FileName).Equals("msiexec.exe", StringComparison.OrdinalIgnoreCase);
        var executable = InspectExecutable(plan.FileName, current.Publisher, cancellationToken);
        if (!isMsi) return executable;
        return executable with
        {
            Status = InstalledAppSignatureStatus.WindowsInstaller,
            Summary = executable.Status == InstalledAppSignatureStatus.Trusted
                ? "Windows Installer host is trusted; product signer is not exposed by this registry command"
                : "Windows Installer product signer is not exposed by this registry command",
            Detail = "Sift verified the local msiexec host only. The MSI product/package signature cannot be attributed from a product-code uninstall command, so the registered publisher is not treated as cryptographically verified.",
            PublisherMatch = InstalledAppPublisherMatch.NotAvailable
        };
    }

    private static InstalledAppTrustReport InspectExecutable(string path, string registeredPublisher,
        CancellationToken cancellationToken)
    {
        if (!File.Exists(path)) return Unavailable("The registered executable is no longer present.", path);
        cancellationToken.ThrowIfCancellationRequested();
        var verification = AuthenticodeVerifier.Verify(path);
        string signer = string.Empty;
        string thumbprint = string.Empty;
        string validity = string.Empty;
        string chainDetail = verification.Detail;
        try
        {
            using var native = X509Certificate.CreateFromSignedFile(verification.CertificatePath);
            using var certificate = new X509Certificate2(native);
            signer = certificate.GetNameInfo(X509NameType.SimpleName, forIssuer: false);
            thumbprint = certificate.Thumbprint ?? string.Empty;
            validity = $"{certificate.NotBefore:d} – {certificate.NotAfter:d}";
            using var chain = new X509Chain();
            chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
            chain.ChainPolicy.DisableCertificateDownloads = true;
            chain.ChainPolicy.VerificationFlags = X509VerificationFlags.NoFlag;
            var chainValid = chain.Build(certificate);
            var statuses = chain.ChainStatus.Select(status => status.StatusInformation.Trim())
                .Where(value => !string.IsNullOrWhiteSpace(value)).Distinct().ToList();
            chainDetail += chainValid
                ? " Local certificate chain builds successfully; revocation and network retrieval were intentionally disabled."
                : $" Local certificate chain did not build: {(statuses.Count == 0 ? "unspecified chain error" : string.Join("; ", statuses))}. Revocation and network retrieval were intentionally disabled.";
        }
        catch (CryptographicException)
        {
            if (verification.Status != InstalledAppSignatureStatus.Unsigned)
                chainDetail += " Signer certificate details could not be extracted.";
        }

        cancellationToken.ThrowIfCancellationRequested();
        string hash;
        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete))
            hash = Convert.ToHexString(SHA256.HashData(stream));
        var version = FileVersionInfo.GetVersionInfo(path).FileVersion ?? string.Empty;
        var publisherMatch = ComparePublisher(registeredPublisher, signer);
        var summary = verification.Status switch
        {
            InstalledAppSignatureStatus.Trusted => string.IsNullOrWhiteSpace(signer) ? "Trusted Authenticode signature" : $"Trusted signer: {signer}",
            InstalledAppSignatureStatus.SignedUntrusted => string.IsNullOrWhiteSpace(signer) ? "Signed, but not locally trusted" : $"Signed by {signer}, but not locally trusted",
            InstalledAppSignatureStatus.Unsigned => "Registered uninstaller is unsigned",
            _ => "Signature could not be verified"
        };
        return new InstalledAppTrustReport(verification.Status, summary, chainDetail, path, version, signer,
            thumbprint, validity, publisherMatch, hash);
    }

    private static InstalledAppPublisherMatch ComparePublisher(string registeredPublisher, string signer)
    {
        if (string.IsNullOrWhiteSpace(registeredPublisher) || string.IsNullOrWhiteSpace(signer))
            return InstalledAppPublisherMatch.NotAvailable;
        var left = NormalizePublisher(registeredPublisher);
        var right = NormalizePublisher(signer);
        if (left.Length < 3 || right.Length < 3) return InstalledAppPublisherMatch.NotAvailable;
        return left.Contains(right, StringComparison.Ordinal) || right.Contains(left, StringComparison.Ordinal)
            ? InstalledAppPublisherMatch.Matches
            : InstalledAppPublisherMatch.Differs;
    }

    private static string NormalizePublisher(string value)
    {
        var normalized = new string(value.Where(char.IsLetterOrDigit).Select(char.ToLowerInvariant).ToArray());
        foreach (var suffix in new[] { "corporation", "incorporated", "company", "limited", "software", "corp", "inc", "llc", "ltd", "co" })
            if (normalized.EndsWith(suffix, StringComparison.Ordinal) && normalized.Length > suffix.Length + 2)
                normalized = normalized[..^suffix.Length];
        return normalized;
    }

    private static InstalledAppTrustReport Unavailable(string reason, string path = "") =>
        new(InstalledAppSignatureStatus.Unavailable, "Signature information unavailable", reason, path,
            string.Empty, string.Empty, string.Empty, string.Empty, InstalledAppPublisherMatch.NotAvailable, string.Empty);
}

internal static class AuthenticodeVerifier
{
    private const uint ErrorSuccess = 0;
    private const uint TrustENoSignature = 0x800B0100;
    private const uint WtdUseDefaultOsverCheck = 0x400;
    private const uint WtdCacheOnlyUrlRetrieval = 0x1000;
    private static readonly IntPtr InvalidHandleValue = new(-1);
    private static readonly Guid ActionGenericVerifyV2 = new("00AAC56B-CD44-11d0-8CC2-00C04FC295EE");

    public static (InstalledAppSignatureStatus Status, string Detail, string CertificatePath) Verify(string path)
    {
        using var fileInfo = new WinTrustFileInfo(path);
        using var data = new WinTrustData(fileInfo);
        var result = WinVerifyTrust(InvalidHandleValue, ActionGenericVerifyV2, data);
        if (result == ErrorSuccess)
            return (InstalledAppSignatureStatus.Trusted,
                "Windows verified the embedded Authenticode signature using local trust state without network retrieval.", path);
        if (result == TrustENoSignature && TryVerifyCatalog(path, out var catalogResult, out var catalogPath,
                out var catalogDetail))
        {
            return catalogResult == ErrorSuccess
                ? (InstalledAppSignatureStatus.Trusted,
                    $"Windows verified the system catalog signature using local trust state without network retrieval. {catalogDetail}",
                    catalogPath)
                : (InstalledAppSignatureStatus.SignedUntrusted,
                    $"Windows found the file in a system catalog but did not accept that catalog signature using local trust state (0x{catalogResult:X8}). {catalogDetail}",
                    catalogPath);
        }
        if (result == TrustENoSignature)
            return (InstalledAppSignatureStatus.Unsigned,
                "Windows found neither an embedded Authenticode signature nor a matching locally installed security catalog.", path);
        return (InstalledAppSignatureStatus.SignedUntrusted,
            $"Windows did not accept the embedded Authenticode signature using local trust state (0x{result:X8}).", path);
    }

    private static bool TryVerifyCatalog(string path, out uint result, out string catalogPath, out string detail)
    {
        result = TrustENoSignature;
        catalogPath = path;
        detail = string.Empty;
        IntPtr admin = IntPtr.Zero;
        IntPtr catalog = IntPtr.Zero;
        try
        {
            if (!CryptCATAdminAcquireContext2(out admin, IntPtr.Zero, null, IntPtr.Zero, 0))
            {
                detail = $"Catalog context acquisition failed ({Marshal.GetLastWin32Error()}).";
                return false;
            }

            using var stream = new FileStream(path, FileMode.Open, FileAccess.Read,
                FileShare.ReadWrite | FileShare.Delete);
            uint hashSize = 0;
            if (!CryptCATAdminCalcHashFromFileHandle2(admin, stream.SafeFileHandle.DangerousGetHandle(),
                    ref hashSize, null, 0) || hashSize is 0 or > 1024)
            {
                detail = $"Catalog hash sizing failed ({Marshal.GetLastWin32Error()}).";
                return false;
            }

            var hash = new byte[hashSize];
            if (!CryptCATAdminCalcHashFromFileHandle2(admin, stream.SafeFileHandle.DangerousGetHandle(),
                    ref hashSize, hash, 0))
            {
                detail = $"Catalog hash calculation failed ({Marshal.GetLastWin32Error()}).";
                return false;
            }

            catalog = CryptCATAdminEnumCatalogFromHash(admin, hash, hashSize, 0, IntPtr.Zero);
            if (catalog == IntPtr.Zero)
            {
                detail = "No locally installed catalog contains the file hash.";
                return false;
            }

            var info = new CatalogInfo { StructSize = (uint)Marshal.SizeOf<CatalogInfo>() };
            if (!CryptCATCatalogInfoFromContext(catalog, ref info, 0) || string.IsNullOrWhiteSpace(info.CatalogFile))
            {
                detail = $"Catalog path resolution failed ({Marshal.GetLastWin32Error()}).";
                return false;
            }

            catalogPath = info.CatalogFile;
            var memberTag = Convert.ToHexString(hash.AsSpan(0, checked((int)hashSize)));
            using var catalogInfo = new WinTrustCatalogInfo(catalogPath, memberTag, path,
                stream.SafeFileHandle.DangerousGetHandle(), admin, hash.AsSpan(0, checked((int)hashSize)).ToArray());
            using var trustData = new WinTrustData(catalogInfo);
            result = WinVerifyTrust(InvalidHandleValue, ActionGenericVerifyV2, trustData);
            detail = $"Catalog: {catalogPath}";
            return true;
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or OverflowException)
        {
            detail = $"Catalog verification could not complete: {exception.Message}";
            return false;
        }
        finally
        {
            if (catalog != IntPtr.Zero) CryptCATAdminReleaseCatalogContext(admin, catalog, 0);
            if (admin != IntPtr.Zero) CryptCATAdminReleaseContext(admin, 0);
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private sealed class WinTrustFileInfo : IDisposable
    {
        public uint StructSize = (uint)Marshal.SizeOf<WinTrustFileInfo>();
        public IntPtr FilePath;
        public IntPtr FileHandle = IntPtr.Zero;
        public IntPtr KnownSubject = IntPtr.Zero;

        public WinTrustFileInfo(string path) => FilePath = Marshal.StringToCoTaskMemUni(path);
        public void Dispose()
        {
            if (FilePath == IntPtr.Zero) return;
            Marshal.FreeCoTaskMem(FilePath);
            FilePath = IntPtr.Zero;
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private sealed class WinTrustData : IDisposable
    {
        public uint StructSize = (uint)Marshal.SizeOf<WinTrustData>();
        public IntPtr PolicyCallbackData = IntPtr.Zero;
        public IntPtr SipClientData = IntPtr.Zero;
        public uint UiChoice = 2;
        public uint RevocationChecks = 0;
        public uint UnionChoice = 1;
        public IntPtr FileInfo;
        public uint StateAction = 0;
        public IntPtr StateData = IntPtr.Zero;
        public IntPtr UrlReference = IntPtr.Zero;
        public uint ProviderFlags = WtdCacheOnlyUrlRetrieval;
        public uint UiContext = 0;

        public WinTrustData(WinTrustFileInfo fileInfo)
        {
            FileInfo = Marshal.AllocCoTaskMem(Marshal.SizeOf<WinTrustFileInfo>());
            Marshal.StructureToPtr(fileInfo, FileInfo, fDeleteOld: false);
        }

        public WinTrustData(WinTrustCatalogInfo catalogInfo)
        {
            UnionChoice = 2;
            ProviderFlags |= WtdUseDefaultOsverCheck;
            FileInfo = Marshal.AllocCoTaskMem(Marshal.SizeOf<WinTrustCatalogInfo>());
            Marshal.StructureToPtr(catalogInfo, FileInfo, fDeleteOld: false);
        }

        public void Dispose()
        {
            if (FileInfo == IntPtr.Zero) return;
            Marshal.FreeCoTaskMem(FileInfo);
            FileInfo = IntPtr.Zero;
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private sealed class WinTrustCatalogInfo : IDisposable
    {
        public uint StructSize = (uint)Marshal.SizeOf<WinTrustCatalogInfo>();
        public uint CatalogVersion = 0;
        public IntPtr CatalogFilePath;
        public IntPtr MemberTag;
        public IntPtr MemberFilePath;
        public IntPtr MemberFileHandle;
        public IntPtr CalculatedFileHash;
        public uint CalculatedFileHashSize;
        public IntPtr CatalogContext = IntPtr.Zero;
        public IntPtr CatalogAdmin;

        public WinTrustCatalogInfo(string catalogPath, string memberTag, string memberPath,
            IntPtr memberFileHandle, IntPtr catalogAdmin, byte[] calculatedHash)
        {
            CatalogFilePath = Marshal.StringToCoTaskMemUni(catalogPath);
            MemberTag = Marshal.StringToCoTaskMemUni(memberTag);
            MemberFilePath = Marshal.StringToCoTaskMemUni(memberPath);
            MemberFileHandle = memberFileHandle;
            CatalogAdmin = catalogAdmin;
            CalculatedFileHash = Marshal.AllocCoTaskMem(calculatedHash.Length);
            Marshal.Copy(calculatedHash, 0, CalculatedFileHash, calculatedHash.Length);
            CalculatedFileHashSize = checked((uint)calculatedHash.Length);
        }

        public void Dispose()
        {
            foreach (var pointer in new[] { CatalogFilePath, MemberTag, MemberFilePath })
                if (pointer != IntPtr.Zero) Marshal.FreeCoTaskMem(pointer);
            if (CalculatedFileHash != IntPtr.Zero) Marshal.FreeCoTaskMem(CalculatedFileHash);
            CatalogFilePath = MemberTag = MemberFilePath = IntPtr.Zero;
            CalculatedFileHash = IntPtr.Zero;
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct CatalogInfo
    {
        public uint StructSize;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string CatalogFile;
    }

    [DllImport("wintrust.dll", ExactSpelling = true, SetLastError = true, PreserveSig = true)]
    private static extern uint WinVerifyTrust(IntPtr windowHandle,
        [MarshalAs(UnmanagedType.LPStruct)] Guid actionId, WinTrustData trustData);

    [DllImport("wintrust.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptCATAdminAcquireContext2(out IntPtr catalogAdmin, IntPtr subsystem,
        string? hashAlgorithm, IntPtr strongHashPolicy, uint flags);

    [DllImport("wintrust.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptCATAdminCalcHashFromFileHandle2(IntPtr catalogAdmin, IntPtr fileHandle,
        ref uint hashSize, byte[]? hash, uint flags);

    [DllImport("wintrust.dll", SetLastError = true)]
    private static extern IntPtr CryptCATAdminEnumCatalogFromHash(IntPtr catalogAdmin, byte[] hash,
        uint hashSize, uint flags, IntPtr previousCatalogInfo);

    [DllImport("wintrust.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptCATCatalogInfoFromContext(IntPtr catalogInfo, ref CatalogInfo information,
        uint flags);

    [DllImport("wintrust.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptCATAdminReleaseCatalogContext(IntPtr catalogAdmin, IntPtr catalogInfo,
        uint flags);

    [DllImport("wintrust.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptCATAdminReleaseContext(IntPtr catalogAdmin, uint flags);
}
