namespace Sift.Services;

public sealed record ScriptRecipeCategory(string Name, string Description, int Order);

public static class ScriptRecipeTaxonomy
{
    public static IReadOnlyList<ScriptRecipeCategory> Categories { get; } =
    [
        new("Network", "IP configuration, routes, sockets, neighbors, and connectivity", 10),
        new("DNS", "Name resolution, resolver configuration, and cache inspection", 20),
        new("Wi-Fi", "Wireless interfaces, drivers, radios, and visible networks", 30),
        new("System", "Windows identity, version, capabilities, locale, and environment", 40),
        new("Drivers", "Installed driver and Plug and Play device diagnostics", 50),
        new("Storage", "Volumes, disks, partitions, filesystem health, and space", 60),
        new("Repair", "Windows component and protected-system-file verification or repair", 70),
        new("Diagnostics", "Built-in Windows diagnostic and monitoring tools", 80),
        new("Processes", "Running processes, sessions, modules, and open resources", 90),
        new("Services", "Windows service inventory and state diagnostics", 100),
        new("Startup", "Registered startup commands and scheduled execution", 110),
        new("Security", "Firewall, Defender, encryption, accounts, audit, and exposure", 120),
        new("Event logs", "Recent operating-system, application, hardware, and security events", 130),
        new("Updates", "Windows update history, services, packages, and reboot state", 140),
        new("Recovery", "Restore snapshots and recovery storage inspection", 150),
        new("Power", "Power plans, sleep, wake, battery, and processor power settings", 160),
        new("Time", "Windows time service status, peers, and configuration", 170),
        new("Apps", "Installed application and package inventory", 180),
        new("WSL / Bash", "Read-only Linux and WSL environment diagnostics", 190)
    ];

    public static ScriptRecipeCategory Get(string name) =>
        Categories.Single(category => string.Equals(category.Name, name, StringComparison.Ordinal));

    public static bool IsKnown(string name) =>
        Categories.Any(category => string.Equals(category.Name, name, StringComparison.Ordinal));

    public static int OrderOf(string name) => Get(name).Order;
}
