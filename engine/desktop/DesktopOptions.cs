namespace FtdDesktop;

public sealed record DesktopOptions(
    string? RepositoryRoot,
    string WslDistribution,
    int LatticeSize,
    int EnginePort,
    int DashboardPort,
    bool SkipEngineBuild,
    bool SmokeTest)
{
    public static DesktopOptions Parse(IReadOnlyList<string> args)
    {
        string? repositoryRoot = null;
        string distro = "Ubuntu-22.04";
        int latticeSize = 64;
        int enginePort = 9100;
        int dashboardPort = 8080;
        bool skipEngineBuild = false;
        bool smokeTest = false;

        for (int i = 0; i < args.Count; ++i)
        {
            string arg = args[i];
            switch (arg)
            {
                case "--repo":
                    repositoryRoot = ReadValue(args, ref i, arg);
                    break;
                case "--distro":
                    distro = ReadValue(args, ref i, arg);
                    break;
                case "--lattice":
                    latticeSize = ReadInt(args, ref i, arg, 4, 256);
                    break;
                case "--engine-port":
                    enginePort = ReadInt(args, ref i, arg, 1, 65535);
                    break;
                case "--dashboard-port":
                    dashboardPort = ReadInt(args, ref i, arg, 1, 65535);
                    break;
                case "--skip-engine-build":
                    skipEngineBuild = true;
                    break;
                case "--smoke-test":
                    smokeTest = true;
                    break;
                default:
                    throw new ArgumentException($"Unknown option '{arg}'.");
            }
        }

        if (enginePort == dashboardPort)
            throw new ArgumentException("Engine and dashboard ports must be different.");

        return new DesktopOptions(
            repositoryRoot,
            distro,
            latticeSize,
            enginePort,
            dashboardPort,
            skipEngineBuild,
            smokeTest);
    }

    private static string ReadValue(IReadOnlyList<string> args, ref int index, string option)
    {
        if (++index >= args.Count || string.IsNullOrWhiteSpace(args[index]))
            throw new ArgumentException($"{option} requires a value.");
        return args[index];
    }

    private static int ReadInt(
        IReadOnlyList<string> args,
        ref int index,
        string option,
        int minimum,
        int maximum)
    {
        string value = ReadValue(args, ref index, option);
        if (!int.TryParse(value, out int parsed) || parsed < minimum || parsed > maximum)
            throw new ArgumentException($"{option} must be in [{minimum}, {maximum}].");
        return parsed;
    }
}
