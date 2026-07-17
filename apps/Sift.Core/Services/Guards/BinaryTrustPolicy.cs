using System.Security.Cryptography.X509Certificates;
using Sift.Models;

namespace Sift.Services;

public static class BinaryTrustPolicy
{
    public static bool IsTrusted(string? path) =>
        !string.IsNullOrWhiteSpace(path) && File.Exists(path) &&
        AuthenticodeVerifier.Verify(path).Status == InstalledAppSignatureStatus.Trusted;

    public static bool HaveSameTrustedSigner(string? leftPath, string? rightPath, out string reason)
    {
        try
        {
            if (!IsTrusted(leftPath) || !IsTrusted(rightPath))
            {
                reason = "Both Sift binaries must have locally trusted Authenticode signatures.";
                return false;
            }
            using var leftNative = X509Certificate.CreateFromSignedFile(leftPath!);
            using var rightNative = X509Certificate.CreateFromSignedFile(rightPath!);
            using var left = new X509Certificate2(leftNative);
            using var right = new X509Certificate2(rightNative);
            if (!string.Equals(left.Thumbprint, right.Thumbprint, StringComparison.OrdinalIgnoreCase))
            {
                reason = "The Sift binary signers do not match.";
                return false;
            }
            reason = "Trusted matching Sift signers.";
            return true;
        }
        catch (Exception exception)
        {
            reason = $"Sift binary trust could not be verified: {exception.Message}";
            return false;
        }
    }
}
