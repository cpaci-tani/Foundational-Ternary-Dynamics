using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Infrastructure.Dialogs;

public static class ConfirmationDialogStyle
{
    public static void Apply(ContentDialog dialog)
    {
        dialog.PrimaryButtonStyle = (Style)Application.Current.Resources["PrimaryButtonStyle"];
        dialog.SecondaryButtonStyle = (Style)Application.Current.Resources["SecondaryButtonStyle"];
        dialog.CloseButtonStyle = (Style)Application.Current.Resources["SecondaryButtonStyle"];
        // Enter must never be an implicit mutation and the Windows accent must not recolor the safe default.
        dialog.DefaultButton = ContentDialogButton.None;
    }
}
