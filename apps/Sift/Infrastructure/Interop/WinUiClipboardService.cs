using Windows.ApplicationModel.DataTransfer;

namespace Sift.WinUI.Infrastructure.Interop;

public sealed class WinUiClipboardService : IClipboardService
{
    public void CopyText(string text, bool persistAfterExit = true)
    {
        ArgumentNullException.ThrowIfNull(text);
        var package = new DataPackage { RequestedOperation = DataPackageOperation.Copy };
        package.SetText(text);
        Clipboard.SetContent(package);
        if (persistAfterExit) Clipboard.Flush();
    }
}
