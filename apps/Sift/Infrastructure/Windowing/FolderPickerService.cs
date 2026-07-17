using Windows.Storage.Pickers;

namespace Sift.WinUI.Infrastructure.Windowing;

public interface IFolderPickerService
{
    Task<string?> PickFolderAsync();
}

public sealed class FolderPickerService(Func<IntPtr> windowHandle) : IFolderPickerService
{
    public async Task<string?> PickFolderAsync()
    {
        var handle = windowHandle();
        if (handle == IntPtr.Zero) throw new InvalidOperationException("The Sift window is not ready for folder selection.");
        var picker = new FolderPicker { SuggestedStartLocation = PickerLocationId.ComputerFolder };
        picker.FileTypeFilter.Add("*");
        WinRT.Interop.InitializeWithWindow.Initialize(picker, handle);
        var folder = await picker.PickSingleFolderAsync();
        return folder?.Path;
    }
}
