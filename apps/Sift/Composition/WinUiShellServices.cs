using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.WinUI.Infrastructure.Interop;

namespace Sift.WinUI.Composition;

public sealed record WinUiShellServices(
    AppSettings Settings,
    ActivityHub Activity,
    SettingsPersistenceCoordinator SettingsPersistence,
    IClipboardService Clipboard);
