using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Documents;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

public sealed partial class ScriptCenterWorkspaceView
{
    private const int MaximumTerminalLines = 1_000;

    public void Append(string line, bool error = false) => AppendBatch([(line, error)]);

    public void AppendBatch(IReadOnlyList<(string Line, bool Error)> lines)
    {
        foreach (var (line, error) in lines)
        {
            var paragraph = new Paragraph();
            paragraph.Inlines.Add(new Run
            {
                Text = line,
                Foreground = new SolidColorBrush(error
                    ? Windows.UI.Color.FromArgb(255, 220, 126, 105)
                    : Windows.UI.Color.FromArgb(255, 183, 196, 177))
            });
            TerminalOutput.Blocks.Add(paragraph);
            PostStudio(new { type = "terminal.write", text = line, error });
        }
        while (TerminalOutput.Blocks.Count > MaximumTerminalLines) TerminalOutput.Blocks.RemoveAt(0);
        TerminalScroller.UpdateLayout();
        TerminalScroller.ChangeView(null, TerminalScroller.ScrollableHeight, null, true);
    }

    private void ClearButton_Click(object sender, RoutedEventArgs e)
    {
        TerminalOutput.Blocks.Clear();
        PostStudio(new { type = "terminal.clear" });
        Append("Terminal cleared.");
    }
}
