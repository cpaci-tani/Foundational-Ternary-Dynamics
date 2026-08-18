namespace FtdDesktop;

public sealed record DesktopPaths(
    string RepositoryRoot,
    string WebRoot,
    string WslRepositoryRoot,
    string WslServerPath,
    string LogDirectory)
{
    public static DesktopPaths Resolve(string? explicitRoot)
    {
        var candidates = new List<string?>
        {
            explicitRoot,
            Environment.GetEnvironmentVariable("FTD_REPO_ROOT"),
            Environment.CurrentDirectory,
            AppContext.BaseDirectory,
        };

        string? repositoryRoot = candidates
            .Where(path => !string.IsNullOrWhiteSpace(path))
            .SelectMany(Ancestors)
            .FirstOrDefault(IsRepositoryRoot);

        if (repositoryRoot is null)
        {
            throw new DirectoryNotFoundException(
                "Could not locate the FTD repository. Start FtdDesktop from the repository " +
                "or pass --repo <path>.");
        }

        repositoryRoot = Path.GetFullPath(repositoryRoot);
        string webRoot = Path.Combine(repositoryRoot, "engine", "web");
        string wslRepositoryRoot = ToWslPath(repositoryRoot);
        string localData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        string logDirectory = Path.Combine(localData, "FTD", "Desktop", "logs");
        Directory.CreateDirectory(logDirectory);

        return new DesktopPaths(
            repositoryRoot,
            webRoot,
            wslRepositoryRoot,
            $"{wslRepositoryRoot}/engine/build_wsl/ws_server",
            logDirectory);
    }

    private static IEnumerable<string> Ancestors(string? path)
    {
        if (string.IsNullOrWhiteSpace(path))
            yield break;

        DirectoryInfo? current;
        try
        {
            string fullPath = Path.GetFullPath(path);
            current = File.Exists(fullPath)
                ? new FileInfo(fullPath).Directory
                : new DirectoryInfo(fullPath);
        }
        catch
        {
            yield break;
        }

        while (current is not null)
        {
            yield return current.FullName;
            current = current.Parent;
        }
    }

    private static bool IsRepositoryRoot(string path) =>
        File.Exists(Path.Combine(path, "engine", "CMakeLists.txt")) &&
        File.Exists(Path.Combine(path, "engine", "web", "index.html"));

    private static string ToWslPath(string windowsPath)
    {
        string fullPath = Path.GetFullPath(windowsPath).Replace('\\', '/');
        if (fullPath.Length >= 3 && char.IsLetter(fullPath[0]) && fullPath[1] == ':')
        {
            char drive = char.ToLowerInvariant(fullPath[0]);
            return $"/mnt/{drive}/{fullPath[3..]}";
        }

        throw new NotSupportedException(
            $"Repository path '{windowsPath}' is not on a Windows drive mounted by WSL2.");
    }
}
