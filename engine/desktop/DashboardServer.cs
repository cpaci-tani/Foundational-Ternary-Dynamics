using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.FileProviders;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System.Text.Json;

namespace FtdDesktop;

public sealed class DashboardServer : IAsyncDisposable
{
    private WebApplication? _application;
    private PhysicalFileProvider? _fileProvider;

    public async Task StartAsync(string webRoot, int port, CancellationToken cancellationToken)
    {
        if (_application is not null)
            return;

        if (!File.Exists(Path.Combine(webRoot, "index.html")))
            throw new FileNotFoundException("Dashboard index.html was not found.", webRoot);

        var options = new WebApplicationOptions
        {
            Args = Array.Empty<string>(),
            ContentRootPath = webRoot,
            WebRootPath = webRoot,
            ApplicationName = typeof(DashboardServer).Assembly.FullName,
        };

        WebApplicationBuilder builder = WebApplication.CreateSlimBuilder(options);
        builder.Logging.ClearProviders();
        builder.WebHost.UseUrls($"http://127.0.0.1:{port}");

        WebApplication app = builder.Build();
        var fileProvider = new PhysicalFileProvider(webRoot);

        // Match engine/web/serve.py. Scale workers and the standalone WASM
        // Scale 1/2 backends require SharedArrayBuffer, which WebView2 exposes
        // only in a cross-origin-isolated document.
        app.Use(async (context, next) =>
        {
            context.Response.Headers["Cross-Origin-Opener-Policy"] = "same-origin";
            context.Response.Headers["Cross-Origin-Embedder-Policy"] = "require-corp";
            context.Response.Headers["Cross-Origin-Resource-Policy"] = "same-origin";
            await next();
        });

        app.UseDefaultFiles(new DefaultFilesOptions
        {
            FileProvider = fileProvider,
        });
        app.UseStaticFiles(new StaticFileOptions
        {
            FileProvider = fileProvider,
            OnPrepareResponse = context =>
            {
                context.Context.Response.Headers.CacheControl = "no-store, no-cache, must-revalidate";
                context.Context.Response.Headers.Pragma = "no-cache";
                context.Context.Response.Headers.Expires = "0";
            },
        });
        app.MapGet("/__ftd_desktop/health", () => Results.Json(new { ok = true }));

        try
        {
            await app.StartAsync(cancellationToken);
            await VerifyHealthAsync(port, cancellationToken);
            _application = app;
            _fileProvider = fileProvider;
        }
        catch (OperationCanceledException)
        {
            await app.DisposeAsync();
            fileProvider.Dispose();
            throw;
        }
        catch (Exception ex)
        {
            try
            {
                using var stopTimeout = new CancellationTokenSource(TimeSpan.FromSeconds(1));
                await app.StopAsync(stopTimeout.Token);
            }
            catch
            {
                // Startup may have failed before Kestrel entered the running state.
            }
            await app.DisposeAsync();
            fileProvider.Dispose();
            throw new InvalidOperationException(
                $"The dashboard server could not bind or pass its health check on " +
                $"127.0.0.1:{port}. Choose another --dashboard-port.",
                ex);
        }
    }

    private static async Task VerifyHealthAsync(int port, CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(3));
        using var client = new HttpClient
        {
            Timeout = Timeout.InfiniteTimeSpan,
        };
        using HttpResponseMessage response = await client.GetAsync(
            $"http://127.0.0.1:{port}/__ftd_desktop/health",
            timeout.Token);
        response.EnsureSuccessStatusCode();
        string payload = await response.Content.ReadAsStringAsync(timeout.Token);
        using JsonDocument document = JsonDocument.Parse(payload);
        if (!document.RootElement.TryGetProperty("ok", out JsonElement ok) ||
            ok.ValueKind != JsonValueKind.True)
            throw new InvalidDataException("Dashboard health response was not authoritative.");
    }

    public async ValueTask DisposeAsync()
    {
        if (_application is null)
            return;

        try
        {
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(3));
            await _application.StopAsync(timeout.Token);
        }
        finally
        {
            await _application.DisposeAsync();
            _application = null;
            _fileProvider?.Dispose();
            _fileProvider = null;
        }
    }
}
