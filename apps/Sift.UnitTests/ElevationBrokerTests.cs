using System.Diagnostics;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class ElevationBrokerTests
{
    [Fact]
    public void Elevated_policy_resolves_only_bounded_allowlisted_hklm_tweaks()
    {
        Assert.True(ElevatedOperationPolicy.TryResolveMachineTweaks(["privacy.activity"],
            out var machine, out _));
        var item = Assert.Single(machine);
        Assert.StartsWith("HKLM\\", item.Target, StringComparison.OrdinalIgnoreCase);

        Assert.False(ElevatedOperationPolicy.TryResolveMachineTweaks(["privacy.adid"], out _, out _));
        Assert.False(ElevatedOperationPolicy.TryResolveMachineTweaks(["not-a-real-setting"], out _, out _));
        Assert.False(ElevatedOperationPolicy.TryResolveMachineTweaks([], out _, out _));
        Assert.False(ElevatedOperationPolicy.TryResolveMachineTweaks(
            Enumerable.Range(0, 65).Select(index => "unknown-" + index), out _, out _));
    }

    [Fact]
    public void Elevated_policy_resolves_allowlisted_repair_commands_and_rejects_local_commands()
    {
        Assert.True(ElevatedOperationPolicy.TryResolveMachineTweaks(
            ["repair.dism-component-cleanup", "repair.sfc-scan"], out var repair, out _));
        Assert.Equal(2, repair.Count);
        Assert.All(repair, tweak => Assert.True(tweak.RequiresElevation));

        Assert.True(ElevatedOperationPolicy.TryResolveMachineTweaks(["power.hibernate"], out var power, out _));
        Assert.True(Assert.Single(power).RequiresElevation);
        Assert.False(ElevatedOperationPolicy.TryResolveMachineTweaks(["apps.clipchamp"], out _, out _));
    }

    [Theory]
    [InlineData("WinDefend", ServiceActionKind.Restart)]
    [InlineData("EventLog", ServiceActionKind.Start)]
    [InlineData("missing/service", ServiceActionKind.Start)]
    public void Elevated_service_policy_rejects_protected_forged_and_unsupported_requests(
        string serviceName, ServiceActionKind action)
    {
        var expectedState = action == ServiceActionKind.Start
            ? ServiceObservedState.Stopped
            : ServiceObservedState.Running;
        Assert.False(ElevatedOperationPolicy.TryResolveServiceAction(
            serviceName, action, expectedState, out _, out _));
    }

    [Fact]
    public void Elevated_service_policy_rejects_unsupported_action_kinds()
    {
        Assert.False(ElevatedOperationPolicy.TryResolveServiceAction(
            "AcmeService", (ServiceActionKind)99, ServiceObservedState.Running, out _, out _));
    }

    [Fact]
    public void Elevation_files_reject_paths_outside_the_exact_per_user_request_root()
    {
        var outside = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N") + ".request.json");
        Assert.Throws<InvalidDataException>(() =>
            ElevationOperationFiles.ValidateExactPath(outside, ".request.json"));

        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        ElevationOperationFiles.ValidateExactPath(paths.RequestPath, ".request.json");
        ElevationOperationFiles.ValidateExactPath(paths.ResponsePath, ".response.json");
    }

    [Fact]
    public void Elevation_request_round_trip_preserves_nonce_and_typed_operation()
    {
        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        try
        {
            var request = new ElevatedOperationRequest(id, "nonce", ElevatedOperationKind.ApplyMachineTweaks,
                ["privacy.activity"]);
            ElevationOperationFiles.WriteRequest(paths.RequestPath, request);
            var loaded = ElevationOperationFiles.ReadRequest(paths.RequestPath);

            Assert.Equal(id, loaded.RequestId);
            Assert.Equal("nonce", loaded.Nonce);
            Assert.Equal(ElevatedOperationKind.ApplyMachineTweaks, loaded.Operation);
            Assert.Equal(["privacy.activity"], loaded.TweakIds);
        }
        finally
        {
            if (File.Exists(paths.RequestPath)) File.Delete(paths.RequestPath);
            if (File.Exists(paths.ResponsePath)) File.Delete(paths.ResponsePath);
        }
    }

    [Fact]
    public void Elevation_request_lease_allows_helper_read_but_denies_tampering()
    {
        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        try
        {
            var request = new ElevatedOperationRequest(id, new string('C', 64),
                ElevatedOperationKind.ApplyMachineTweaks, ["privacy.activity"]);
            using (ElevationOperationFiles.WriteRequestLease(paths.RequestPath, request))
            {
                Assert.Equal(id, ElevationOperationFiles.ReadRequest(paths.RequestPath).RequestId);
                Assert.Throws<IOException>(() => new FileStream(paths.RequestPath, FileMode.Open,
                    FileAccess.Write, FileShare.ReadWrite).Dispose());
                Assert.Throws<IOException>(() => File.Delete(paths.RequestPath));
            }
        }
        finally
        {
            if (File.Exists(paths.RequestPath)) File.Delete(paths.RequestPath);
            if (File.Exists(paths.ResponsePath)) File.Delete(paths.ResponsePath);
        }
    }

    [Fact]
    public void Service_elevation_request_round_trip_preserves_only_typed_fields()
    {
        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        try
        {
            var request = new ElevatedOperationRequest(id, "nonce", ElevatedOperationKind.ManageService, [],
                ServiceName: "AcmeService", ServiceAction: ServiceActionKind.Restart,
                ExpectedServiceState: ServiceObservedState.Running);
            ElevationOperationFiles.WriteRequest(paths.RequestPath, request);

            var loaded = ElevationOperationFiles.ReadRequest(paths.RequestPath);

            Assert.Equal(ElevatedOperationKind.ManageService, loaded.Operation);
            Assert.Equal("AcmeService", loaded.ServiceName);
            Assert.Equal(ServiceActionKind.Restart, loaded.ServiceAction);
            Assert.Equal(ServiceObservedState.Running, loaded.ExpectedServiceState);
            Assert.Empty(loaded.TweakIds);
            Assert.Null(loaded.BackupFileName);
        }
        finally
        {
            if (File.Exists(paths.RequestPath)) File.Delete(paths.RequestPath);
            if (File.Exists(paths.ResponsePath)) File.Delete(paths.ResponsePath);
        }
    }

    [Fact]
    public void Service_request_shape_requires_action_and_matching_confirmed_state()
    {
        var id = Guid.NewGuid().ToString("N");
        var nonce = new string('A', 64);
        var missingState = new ElevatedOperationRequest(id, nonce, ElevatedOperationKind.ManageService, [],
            ServiceName: "AcmeService", ServiceAction: ServiceActionKind.Restart);
        var mismatchedState = missingState with { ExpectedServiceState = ServiceObservedState.Stopped };
        var matchedState = missingState with { ExpectedServiceState = ServiceObservedState.Running };

        Assert.False(ElevatedOperationPolicy.TryValidateRequestShape(missingState, out _));
        Assert.False(ElevatedOperationPolicy.TryValidateRequestShape(mismatchedState, out _));
        Assert.True(ElevatedOperationPolicy.TryValidateRequestShape(matchedState, out _));
    }

    [Fact]
    public async Task Missing_helper_blocks_before_requesting_uac()
    {
        var machine = Assert.Single(TweakCatalog.Create(), tweak => tweak.Id == "privacy.activity");
        var broker = new ElevationBroker(Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"), "missing.exe"));

        var result = await broker.ApplyMachineTweaksAsync([machine], TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.False(result.Cancelled);
        Assert.Contains("helper is missing", result.Message);
    }

    [Fact]
    public void Restore_point_request_shape_rejects_payload_fields()
    {
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            new string('A', 64), ElevatedOperationKind.CreateSystemRestorePoint, ["privacy.activity"]);
        Assert.False(ElevatedOperationPolicy.TryValidateRequestShape(request, out var reason));
        Assert.Contains("unrelated fields", reason, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Restore_point_request_shape_accepts_parameter_free_request()
    {
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            new string('B', 64), ElevatedOperationKind.CreateSystemRestorePoint, []);
        Assert.True(ElevatedOperationPolicy.TryValidateRequestShape(request, out _));
    }

    [Fact]
    public void Elevation_write_rejects_a_preexisting_request_name()
    {
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        File.WriteAllText(paths.RequestPath, "{}");
        try
        {
            var request = new ElevatedOperationRequest(id, new string('A', 64),
                ElevatedOperationKind.ApplyMachineTweaks, ["privacy.activity"]);
            // CreateNew must reject an existing name; the Win32 ERROR_FILE_EXISTS is mapped to a
            // typed InvalidDataException rather than being followed or silently overwriting.
            Assert.Throws<InvalidDataException>(() =>
                ElevationOperationFiles.WriteRequestLease(paths.RequestPath, request));
        }
        finally
        {
            if (File.Exists(paths.RequestPath)) File.Delete(paths.RequestPath);
        }
    }

    [Fact]
    public void Elevation_files_reject_a_reparse_point_request_name()
    {
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        var id = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(id);
        var target = Path.Combine(Path.GetTempPath(), "sift-junction-target-" + id);
        if (!TryCreateJunction(paths.RequestPath, target))
            Assert.Skip("Could not create a junction to exercise the reparse-point guard in this environment.");
        try
        {
            // A reparse point planted at the request name must be rejected by name (dangling links
            // included), and the guarded write must refuse to follow it.
            Assert.Throws<InvalidDataException>(() =>
                ElevationOperationFiles.ValidateExactPath(paths.RequestPath, ".request.json"));
            var request = new ElevatedOperationRequest(id, new string('A', 64),
                ElevatedOperationKind.ApplyMachineTweaks, ["privacy.activity"]);
            Assert.Throws<InvalidDataException>(() =>
                ElevationOperationFiles.WriteRequestLease(paths.RequestPath, request));
        }
        finally
        {
            try { Directory.Delete(paths.RequestPath); } catch { /* best-effort cleanup */ }
            try { Directory.Delete(target, recursive: true); } catch { /* best-effort cleanup */ }
        }
    }

    private static bool TryCreateJunction(string link, string target)
    {
        try
        {
            Directory.CreateDirectory(target);
            using var process = Process.Start(new ProcessStartInfo("cmd.exe", $"/c mklink /J \"{link}\" \"{target}\"")
            {
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true
            });
            if (process is null) return false;
            process.WaitForExit(5000);
            return process.ExitCode == 0 && Directory.Exists(link);
        }
        catch
        {
            return false;
        }
    }
}
