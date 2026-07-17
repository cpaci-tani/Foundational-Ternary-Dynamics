using Microsoft.UI.Xaml;
using Sift.Models;
using Sift.Services;

namespace Sift.WinUI.Controls;

public sealed class SensorGraphBoardPresenter(SensorHistoryStore history) : IDockBoardPresenter
{
    private readonly SensorHistoryStore _history = history ?? throw new ArgumentNullException(nameof(history));

    public FrameworkElement CreateBoard(IDockSession session, DockBoardNode board)
    {
        var view = new GraphBoardView();
        view.Bind(session, board, _history);
        return view;
    }

    public void BindBoard(FrameworkElement view, IDockSession session, DockBoardNode board)
    {
        if (view is GraphBoardView graph)
            graph.Bind(session, board, _history);
    }

    public void ApplyData(FrameworkElement view, object? data)
    {
        if (view is not GraphBoardView graph) return;
        graph.ApplyHistories(data as IReadOnlyDictionary<string, string>);
    }
}
