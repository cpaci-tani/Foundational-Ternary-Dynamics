using Microsoft.UI.Xaml;
using Sift.Models;
using Sift.Services;

namespace Sift.WinUI.Controls;

/// <summary>
/// Creates and refreshes leaf board content for a dock shell. Workspaces supply their own presenter.
/// </summary>
public interface IDockBoardPresenter
{
    FrameworkElement CreateBoard(IDockSession session, DockBoardNode board);
    void BindBoard(FrameworkElement view, IDockSession session, DockBoardNode board);
    void ApplyData(FrameworkElement view, object? data);
}

/// <summary>
/// Marker for board views hosted inside <see cref="DockHostControl"/>.
/// </summary>
public interface IDockBoardView;
