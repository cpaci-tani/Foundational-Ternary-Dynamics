namespace Sift.WinUI.Infrastructure.Interop;

public interface IClipboardService
{
    void CopyText(string text, bool persistAfterExit = true);
}
