namespace Sift.Models;

public enum InstalledAppSignatureStatus
{
    Trusted,
    SignedUntrusted,
    Unsigned,
    WindowsInstaller,
    Unavailable,
    Error
}

public enum InstalledAppPublisherMatch
{
    Matches,
    Differs,
    NotAvailable
}

public sealed record InstalledAppTrustReport(
    InstalledAppSignatureStatus Status,
    string Summary,
    string Detail,
    string ExecutablePath,
    string FileVersion,
    string Signer,
    string CertificateThumbprint,
    string CertificateValidity,
    InstalledAppPublisherMatch PublisherMatch,
    string Sha256)
{
    public string PublisherMatchDisplay => PublisherMatch switch
    {
        InstalledAppPublisherMatch.Matches => "Registered publisher resembles signer",
        InstalledAppPublisherMatch.Differs => "Registered publisher differs from signer",
        _ => "Publisher comparison unavailable"
    };
}
