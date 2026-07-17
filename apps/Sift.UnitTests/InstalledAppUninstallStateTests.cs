using Sift.Models;

namespace Sift.UnitTests;

public sealed class InstalledAppUninstallStateTests
{
    [Fact]
    public void Continuation_is_bound_to_registration_identity_and_display_name()
    {
        var target = App("HKCU|64-bit|fixture", "Fixture");
        var state = new InstalledAppUninstallState();

        state.AuthorizeCleanup(target, "continuation", "Removal verified");

        Assert.Equal("continuation", state.ContinuationFor(target));
        Assert.Null(state.ContinuationFor(App("HKCU|64-bit|other", "Fixture")));
        Assert.Null(state.ContinuationFor(App("HKCU|64-bit|fixture", "Renamed fixture")));
    }

    [Fact]
    public void Tracking_a_new_uninstaller_revokes_the_previous_continuation()
    {
        var target = App("HKCU|64-bit|fixture", "Fixture");
        var state = new InstalledAppUninstallState();
        state.AuthorizeCleanup(target, "old-continuation", "Removal verified");

        state.TrackUninstaller(target, "session", "Uninstaller opened");

        Assert.True(state.HasPendingSession);
        Assert.False(state.CleanupAuthorized);
        Assert.Null(state.ContinuationFor(target));
        Assert.Equal("session", state.SessionId);
    }

    [Fact]
    public void Clear_removes_all_workflow_authority()
    {
        var target = App("HKCU|64-bit|fixture", "Fixture");
        var state = new InstalledAppUninstallState();
        state.TrackUninstaller(target, "session", "Waiting");

        state.Clear();

        Assert.Null(state.Target);
        Assert.Null(state.SessionId);
        Assert.Null(state.ContinuationToken);
        Assert.False(state.Matches(target));
        Assert.Equal(string.Empty, state.Status);
    }

    private static InstalledApp App(string identity, string displayName)
    {
        var parts = identity.Split('|');
        return new InstalledApp(
            new InstalledAppRegistryLocation(parts[0], parts[1], parts[2]),
            displayName, "Publisher", "1.0", string.Empty, string.Empty, 0,
            "MsiExec.exe /X{01234567-89AB-CDEF-0123-456789ABCDEF}",
            "Current user", true, "Available");
    }
}
