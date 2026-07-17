using Sift.Presentation;

namespace Sift.WinUI.Infrastructure.Localization;

/// <summary>
/// Maps <see cref="SiftReasonCode"/> values to presentation strings.
/// Unpackaged WinUI builds must not call <c>ResourceLoader.GetForViewIndependentUse</c>
/// during early UI work — PRI lookup can fail-fast outside managed catch blocks.
/// Core English <see cref="ReasonMessages"/> remain the authoritative fallback.
/// </summary>
public static class ReasonPresenter
{
    public static string Present(SiftReasonCode code, params object?[] args) =>
        ReasonMessages.Format(code, args);

    public static string PresentOrFallback(SiftReasonCode code, string? fallback, params object?[] args) =>
        code == SiftReasonCode.Unspecified
            ? (string.IsNullOrWhiteSpace(fallback)
                ? ReasonMessages.Format(SiftReasonCode.TargetUnavailable)
                : fallback)
            : Present(code, args);
}
