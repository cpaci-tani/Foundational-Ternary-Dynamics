using System.Globalization;
using System.Management;
using System.Security.Principal;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public interface ISystemInformationService
{
    SystemInformationReport Collect(IProgress<string>? progress = null, CancellationToken cancellationToken = default);
}

internal interface ISystemInformationDataSource
{
    IReadOnlyList<IReadOnlyDictionary<string, object?>> Query(
        string namespacePath,
        string wql,
        CancellationToken cancellationToken);
}

internal sealed class WmiSystemInformationDataSource : ISystemInformationDataSource
{
    public IReadOnlyList<IReadOnlyDictionary<string, object?>> Query(
        string namespacePath,
        string wql,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var scope = new ManagementScope($@"\\.\{namespacePath.TrimStart('\\')}");
        scope.Connect();
        using var searcher = new ManagementObjectSearcher(scope, new ObjectQuery(wql), new System.Management.EnumerationOptions
        {
            ReturnImmediately = false,
            Rewindable = false,
            Timeout = TimeSpan.FromSeconds(8)
        });
        using var results = searcher.Get();
        var rows = new List<IReadOnlyDictionary<string, object?>>();
        foreach (ManagementBaseObject result in results)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var row = new Dictionary<string, object?>(StringComparer.OrdinalIgnoreCase);
            foreach (PropertyData property in result.Properties)
                row[property.Name] = CopyValue(property.Value);
            rows.Add(row);
        }
        return rows;
    }

    private static object? CopyValue(object? value) => value switch
    {
        null => null,
        string[] strings => strings.ToArray(),
        Array array => array.Cast<object?>().ToArray(),
        _ => value
    };
}

public sealed class SystemInformationService : ISystemInformationService
{
    private const string Cimv2 = @"root\cimv2";
    private readonly ISystemInformationDataSource _dataSource;

    public SystemInformationService() : this(new WmiSystemInformationDataSource()) { }
    internal SystemInformationService(ISystemInformationDataSource dataSource) => _dataSource = dataSource;

    public SystemInformationReport Collect(IProgress<string>? progress = null, CancellationToken cancellationToken = default)
    {
        var items = new List<SystemInfoItem>();
        var warnings = new List<string>();

        AddEnvironment(items);
        progress?.Report("Reading Windows and computer identity…");
        var operatingSystems = Query(Cimv2,
            "SELECT Caption,Version,BuildNumber,OSArchitecture,InstallDate,LastBootUpTime,Locale,WindowsDirectory,SystemDrive,TotalVisibleMemorySize,FreePhysicalMemory,RegisteredUser,SerialNumber FROM Win32_OperatingSystem",
            "Windows", warnings, cancellationToken);
        var computers = Query(Cimv2,
            "SELECT Manufacturer,Model,SystemType,TotalPhysicalMemory,Domain,Workgroup,PartOfDomain,HypervisorPresent,NumberOfProcessors,NumberOfLogicalProcessors FROM Win32_ComputerSystem",
            "computer system", warnings, cancellationToken);
        AddOperatingSystem(items, operatingSystems.FirstOrDefault());
        AddComputer(items, computers.FirstOrDefault());
        AddWindowsLicense(items, Query(Cimv2,
            "SELECT Name,Description,LicenseStatus,PartialProductKey FROM SoftwareLicensingProduct WHERE PartialProductKey IS NOT NULL AND Name LIKE 'Windows%'",
            "Windows licensing", warnings, cancellationToken, warnWhenUnavailable: false));

        progress?.Report("Reading processor and memory topology…");
        var processors = Query(Cimv2,
            "SELECT Name,Manufacturer,Description,SocketDesignation,NumberOfCores,NumberOfLogicalProcessors,MaxClockSpeed,L2CacheSize,L3CacheSize,VirtualizationFirmwareEnabled,SecondLevelAddressTranslationExtensions,ProcessorId FROM Win32_Processor",
            "processor", warnings, cancellationToken);
        var memoryModules = Query(Cimv2,
            "SELECT BankLabel,DeviceLocator,Manufacturer,PartNumber,SerialNumber,Capacity,Speed,ConfiguredClockSpeed,FormFactor,SMBIOSMemoryType FROM Win32_PhysicalMemory",
            "physical memory", warnings, cancellationToken);
        AddProcessors(items, processors);
        AddMemory(items, memoryModules);

        progress?.Report("Reading firmware and security state…");
        AddFirmware(items,
            Query(Cimv2, "SELECT Manufacturer,Product,Version,SerialNumber FROM Win32_BaseBoard", "baseboard", warnings, cancellationToken),
            Query(Cimv2, "SELECT Manufacturer,SMBIOSBIOSVersion,ReleaseDate,SerialNumber,SMBIOSMajorVersion,SMBIOSMinorVersion FROM Win32_BIOS", "BIOS", warnings, cancellationToken));
        AddSecurity(items, warnings, cancellationToken);

        progress?.Report("Reading graphics and display adapters…");
        AddGraphics(items, Query(Cimv2,
            "SELECT Name,VideoProcessor,AdapterRAM,DriverVersion,DriverDate,CurrentHorizontalResolution,CurrentVerticalResolution,CurrentRefreshRate,Status,PNPDeviceID FROM Win32_VideoController",
            "graphics", warnings, cancellationToken));
        AddMonitors(items, Query(Cimv2,
            "SELECT Name,MonitorManufacturer,ScreenWidth,ScreenHeight,Status,PNPDeviceID FROM Win32_DesktopMonitor WHERE Availability = 3",
            "displays", warnings, cancellationToken, warnWhenUnavailable: false));

        progress?.Report("Reading physical and logical storage…");
        AddStorage(items,
            Query(Cimv2, "SELECT Index,Model,Manufacturer,SerialNumber,MediaType,InterfaceType,Size,FirmwareRevision,Partitions,Status,PNPDeviceID FROM Win32_DiskDrive", "physical storage", warnings, cancellationToken),
            Query(Cimv2, "SELECT DeviceID,VolumeName,FileSystem,Size,FreeSpace,DriveType,VolumeSerialNumber FROM Win32_LogicalDisk WHERE DriveType = 3", "logical storage", warnings, cancellationToken));

        progress?.Report("Reading active network adapters…");
        AddNetwork(items,
            Query(Cimv2, "SELECT Name,Description,Manufacturer,MACAddress,Speed,AdapterType,NetConnectionID,NetConnectionStatus,PhysicalAdapter,PNPDeviceID FROM Win32_NetworkAdapter WHERE NetEnabled = TRUE", "network adapters", warnings, cancellationToken),
            Query(Cimv2, "SELECT Description,MACAddress,DHCPEnabled,DHCPServer,IPAddress,IPSubnet,DefaultIPGateway,DNSServerSearchOrder,DNSDomain FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = TRUE", "network configuration", warnings, cancellationToken));

        progress?.Report("Reading power and audio devices…");
        AddPowerAndAudio(items,
            Query(Cimv2, "SELECT Name,DeviceID,BatteryStatus,EstimatedChargeRemaining,EstimatedRunTime,DesignVoltage,Status FROM Win32_Battery", "battery", warnings, cancellationToken, warnWhenUnavailable: false),
            Query(Cimv2, "SELECT Name,Manufacturer,Status,PNPDeviceID FROM Win32_SoundDevice", "audio", warnings, cancellationToken, warnWhenUnavailable: false));

        cancellationToken.ThrowIfCancellationRequested();
        var ordered = items
            .OrderBy(item => CategoryOrder(item.Category))
            .ThenBy(item => item.Component, StringComparer.OrdinalIgnoreCase)
            .ThenBy(item => item.Property, StringComparer.OrdinalIgnoreCase)
            .ToList();
        var os = operatingSystems.FirstOrDefault();
        var computer = computers.FirstOrDefault();
        var cpu = processors.FirstOrDefault();
        var totalMemory = ULong(computer, "TotalPhysicalMemory");
        return new SystemInformationReport(
            Environment.MachineName,
            JoinNonEmpty(Text(computer, "Manufacturer"), Text(computer, "Model"), "Unknown model"),
            JoinNonEmpty(Text(os, "Caption"), BuildVersion(os), "Unknown Windows version"),
            Text(cpu, "Name", "Unknown processor"),
            totalMemory > 0 ? Bytes(totalMemory) : "Unknown memory",
            Text(os, "OSArchitecture", RuntimeInformationLabel()),
            DateTime.Now,
            ordered,
            warnings.Distinct(StringComparer.OrdinalIgnoreCase).ToList());
    }

    private IReadOnlyList<IReadOnlyDictionary<string, object?>> Query(
        string namespacePath,
        string wql,
        string label,
        List<string> warnings,
        CancellationToken cancellationToken,
        bool warnWhenUnavailable = true)
    {
        try { return _dataSource.Query(namespacePath, wql, cancellationToken); }
        catch (OperationCanceledException) { throw; }
        catch (Exception exception)
        {
            if (warnWhenUnavailable) warnings.Add($"Could not read {label}: {exception.Message}");
            return [];
        }
    }

    private void AddSecurity(List<SystemInfoItem> items, List<string> warnings, CancellationToken cancellationToken)
    {
        Add(items, "Security", "Session", "Sift elevation", IsAdministrator() ? "Administrator" : "Standard user", "Windows access token");
        try
        {
            using var key = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\SecureBoot\State");
            var enabled = key?.GetValue("UEFISecureBootEnabled");
            Add(items, "Security", "Secure Boot", "State", enabled is int value ? YesNo(value == 1) : "Not reported", "HKLM SecureBoot state");
        }
        catch (Exception exception) { warnings.Add($"Could not read Secure Boot state: {exception.Message}"); }

        var tpm = Query(@"root\CIMV2\Security\MicrosoftTpm",
            "SELECT IsActivated_InitialValue,IsEnabled_InitialValue,IsOwned_InitialValue,ManufacturerIdTxt,ManufacturerVersion,SpecVersion FROM Win32_Tpm",
            "TPM", warnings, cancellationToken, warnWhenUnavailable: false).FirstOrDefault();
        if (tpm is not null)
        {
            Add(items, "Security", "TPM", "Enabled", Text(tpm, "IsEnabled_InitialValue"), "Win32_Tpm");
            Add(items, "Security", "TPM", "Activated", Text(tpm, "IsActivated_InitialValue"), "Win32_Tpm");
            Add(items, "Security", "TPM", "Owned", Text(tpm, "IsOwned_InitialValue"), "Win32_Tpm");
            Add(items, "Security", "TPM", "Specification", Text(tpm, "SpecVersion"), "Win32_Tpm");
            Add(items, "Security", "TPM", "Manufacturer", Text(tpm, "ManufacturerIdTxt"), "Win32_Tpm");
            Add(items, "Security", "TPM", "Firmware", Text(tpm, "ManufacturerVersion"), "Win32_Tpm");
        }
        else Add(items, "Security", "TPM", "State", "Not reported by Windows", "Win32_Tpm");

        var defender = Query(@"root\Microsoft\Windows\Defender",
            "SELECT AMServiceEnabled,AntivirusEnabled,AntispywareEnabled,BehaviorMonitorEnabled,IoavProtectionEnabled,NISEnabled,OnAccessProtectionEnabled,RealTimeProtectionEnabled,DefenderSignaturesOutOfDate,AntivirusSignatureLastUpdated FROM MSFT_MpComputerStatus",
            "Microsoft Defender", warnings, cancellationToken, warnWhenUnavailable: false).FirstOrDefault();
        if (defender is not null)
        {
            Add(items, "Security", "Microsoft Defender", "Antivirus", Text(defender, "AntivirusEnabled"), "MSFT_MpComputerStatus");
            Add(items, "Security", "Microsoft Defender", "Real-time protection", Text(defender, "RealTimeProtectionEnabled"), "MSFT_MpComputerStatus");
            Add(items, "Security", "Microsoft Defender", "Behavior monitoring", Text(defender, "BehaviorMonitorEnabled"), "MSFT_MpComputerStatus");
            Add(items, "Security", "Microsoft Defender", "Network inspection", Text(defender, "NISEnabled"), "MSFT_MpComputerStatus");
            Add(items, "Security", "Microsoft Defender", "Signatures current", NegatedBool(defender, "DefenderSignaturesOutOfDate"), "MSFT_MpComputerStatus");
            Add(items, "Security", "Microsoft Defender", "Signature updated", Date(defender, "AntivirusSignatureLastUpdated"), "MSFT_MpComputerStatus");
        }
        else Add(items, "Security", "Microsoft Defender", "State", "Not reported by Windows provider", "MSFT_MpComputerStatus");

        var deviceGuard = Query(@"root\Microsoft\Windows\DeviceGuard",
            "SELECT VirtualizationBasedSecurityStatus,SecurityServicesConfigured,SecurityServicesRunning,RequiredSecurityProperties,AvailableSecurityProperties FROM Win32_DeviceGuard",
            "virtualization-based security", warnings, cancellationToken, warnWhenUnavailable: false).FirstOrDefault();
        if (deviceGuard is not null)
        {
            Add(items, "Security", "Device Guard", "VBS status", VbsStatus(Text(deviceGuard, "VirtualizationBasedSecurityStatus")), "Win32_DeviceGuard");
            Add(items, "Security", "Device Guard", "Services configured", Text(deviceGuard, "SecurityServicesConfigured"), "Win32_DeviceGuard");
            Add(items, "Security", "Device Guard", "Services running", Text(deviceGuard, "SecurityServicesRunning"), "Win32_DeviceGuard");
        }
        else Add(items, "Security", "Device Guard", "VBS status", "Not reported by Windows provider", "Win32_DeviceGuard");

        var encryptedVolumes = Query(@"root\CIMV2\Security\MicrosoftVolumeEncryption",
            "SELECT DriveLetter,PersistentVolumeID,ProtectionStatus,ConversionStatus,EncryptionMethod FROM Win32_EncryptableVolume",
            "BitLocker volumes", warnings, cancellationToken, warnWhenUnavailable: false);
        if (encryptedVolumes.Count == 0)
            Add(items, "Security", "BitLocker", "Volume status", "Not available at current access level", "Win32_EncryptableVolume");
        foreach (var volume in encryptedVolumes)
        {
            var component = $"BitLocker {Text(volume, "DriveLetter", Text(volume, "PersistentVolumeID", "volume"))}";
            Add(items, "Security", component, "Protection", BitLockerProtection(ULong(volume, "ProtectionStatus")), "Win32_EncryptableVolume");
            Add(items, "Security", component, "Conversion", BitLockerConversion(ULong(volume, "ConversionStatus")), "Win32_EncryptableVolume");
            Add(items, "Security", component, "Encryption method", BitLockerMethod(ULong(volume, "EncryptionMethod")), "Win32_EncryptableVolume");
        }
    }

    private static void AddEnvironment(List<SystemInfoItem> items)
    {
        Add(items, "Overview", "Computer", "Device name", Environment.MachineName, "Environment");
        Add(items, "Overview", "Session", "User", $@"{Environment.UserDomainName}\{Environment.UserName}", "Environment");
        Add(items, "Overview", "Session", "Time zone", TimeZoneInfo.Local.DisplayName, "Environment");
        Add(items, "Overview", "Runtime", "Process architecture", System.Runtime.InteropServices.RuntimeInformation.ProcessArchitecture.ToString(), ".NET runtime");
        Add(items, "Overview", "Runtime", "Operating-system architecture", System.Runtime.InteropServices.RuntimeInformation.OSArchitecture.ToString(), ".NET runtime");
    }

    private static void AddOperatingSystem(List<SystemInfoItem> items, IReadOnlyDictionary<string, object?>? row)
    {
        if (row is null) return;
        Add(items, "Windows", "Operating system", "Edition", Text(row, "Caption"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "Version", Text(row, "Version"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "Build", Text(row, "BuildNumber"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "Architecture", Text(row, "OSArchitecture"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "Installed", Date(row, "InstallDate"), "Win32_OperatingSystem");
        var boot = DateValue(row, "LastBootUpTime");
        Add(items, "Windows", "Operating system", "Last boot", boot?.ToString("G"), "Win32_OperatingSystem");
        if (boot.HasValue) Add(items, "Windows", "Operating system", "Uptime", Duration(DateTime.Now - boot.Value), "Calculated from last boot");
        Add(items, "Windows", "Operating system", "Locale", Text(row, "Locale"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "Windows directory", Text(row, "WindowsDirectory"), "Win32_OperatingSystem");
        Add(items, "Windows", "Operating system", "System drive", Text(row, "SystemDrive"), "Win32_OperatingSystem");
        Add(items, "Windows", "Registration", "Registered user", Text(row, "RegisteredUser"), "Win32_OperatingSystem");
        Add(items, "Windows", "Registration", "Product identifier", Text(row, "SerialNumber"), "Win32_OperatingSystem");
        var total = ULong(row, "TotalVisibleMemorySize") * 1024;
        var free = ULong(row, "FreePhysicalMemory") * 1024;
        if (total > 0) Add(items, "Memory", "Windows memory", "Usable physical memory", Bytes(total), "Win32_OperatingSystem");
        if (free > 0) Add(items, "Memory", "Windows memory", "Available at scan", Bytes(free), "Win32_OperatingSystem");
    }

    private static void AddComputer(List<SystemInfoItem> items, IReadOnlyDictionary<string, object?>? row)
    {
        if (row is null) return;
        Add(items, "Overview", "Computer", "Manufacturer", Text(row, "Manufacturer"), "Win32_ComputerSystem");
        Add(items, "Overview", "Computer", "Model", Text(row, "Model"), "Win32_ComputerSystem");
        Add(items, "Overview", "Computer", "System type", Text(row, "SystemType"), "Win32_ComputerSystem");
        Add(items, "Overview", "Computer", "Physical memory", Bytes(ULong(row, "TotalPhysicalMemory")), "Win32_ComputerSystem");
        Add(items, "Overview", "Computer", "Physical processors", Text(row, "NumberOfProcessors"), "Win32_ComputerSystem");
        Add(items, "Overview", "Computer", "Logical processors", Text(row, "NumberOfLogicalProcessors"), "Win32_ComputerSystem");
        Add(items, "Overview", "Virtualization", "Hypervisor detected", Text(row, "HypervisorPresent"), "Win32_ComputerSystem");
        Add(items, "Network", "Membership", "Membership type", Bool(row, "PartOfDomain") ? "Domain" : "Workgroup", "Win32_ComputerSystem");
        Add(items, "Network", "Membership", "Domain / workgroup", Bool(row, "PartOfDomain") ? Text(row, "Domain") : Text(row, "Workgroup"), "Win32_ComputerSystem");
    }

    private static void AddWindowsLicense(List<SystemInfoItem> items, IReadOnlyList<IReadOnlyDictionary<string, object?>> rows)
    {
        var row = rows.FirstOrDefault(candidate => ULong(candidate, "LicenseStatus") == 1) ?? rows.FirstOrDefault();
        if (row is null) return;
        Add(items, "Windows", "Licensing", "Product", Text(row, "Name"), "SoftwareLicensingProduct");
        Add(items, "Windows", "Licensing", "Channel", Text(row, "Description"), "SoftwareLicensingProduct");
        Add(items, "Windows", "Licensing", "Status", LicenseStatus(ULong(row, "LicenseStatus")), "SoftwareLicensingProduct");
        Add(items, "Windows", "Licensing", "Partial product key", Text(row, "PartialProductKey"), "SoftwareLicensingProduct");
    }

    private static void AddProcessors(List<SystemInfoItem> items, IReadOnlyList<IReadOnlyDictionary<string, object?>> rows)
    {
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            var component = rows.Count == 1 ? "Processor" : $"Processor {i + 1}";
            Add(items, "Processor", component, "Name", Text(row, "Name"), "Win32_Processor");
            Add(items, "Processor", component, "Manufacturer", Text(row, "Manufacturer"), "Win32_Processor");
            Add(items, "Processor", component, "Socket", Text(row, "SocketDesignation"), "Win32_Processor");
            Add(items, "Processor", component, "Physical cores", Text(row, "NumberOfCores"), "Win32_Processor");
            Add(items, "Processor", component, "Logical processors", Text(row, "NumberOfLogicalProcessors"), "Win32_Processor");
            Add(items, "Processor", component, "Maximum clock", FrequencyMHz(ULong(row, "MaxClockSpeed")), "Win32_Processor");
            Add(items, "Processor", component, "L2 cache", Kibibytes(ULong(row, "L2CacheSize")), "Win32_Processor");
            Add(items, "Processor", component, "L3 cache", Kibibytes(ULong(row, "L3CacheSize")), "Win32_Processor");
            Add(items, "Processor", component, "Firmware virtualization", Text(row, "VirtualizationFirmwareEnabled"), "Win32_Processor");
            Add(items, "Processor", component, "Second-level address translation", Text(row, "SecondLevelAddressTranslationExtensions"), "Win32_Processor");
            Add(items, "Processor", component, "Processor ID", Text(row, "ProcessorId"), "Win32_Processor");
        }
    }

    private static void AddMemory(List<SystemInfoItem> items, IReadOnlyList<IReadOnlyDictionary<string, object?>> rows)
    {
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            var locator = Text(row, "DeviceLocator");
            var component = string.IsNullOrWhiteSpace(locator) ? $"Memory module {i + 1}" : locator;
            Add(items, "Memory", component, "Bank", Text(row, "BankLabel"), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Capacity", Bytes(ULong(row, "Capacity")), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Type", MemoryType(ULong(row, "SMBIOSMemoryType")), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Form factor", MemoryFormFactor(ULong(row, "FormFactor")), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Configured speed", MegaTransfers(ULong(row, "ConfiguredClockSpeed")), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Rated speed", MegaTransfers(ULong(row, "Speed")), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Manufacturer", Text(row, "Manufacturer"), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Part number", Text(row, "PartNumber"), "Win32_PhysicalMemory");
            Add(items, "Memory", component, "Serial number", Text(row, "SerialNumber"), "Win32_PhysicalMemory");
        }
    }

    private static void AddFirmware(List<SystemInfoItem> items,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> boards,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> biosRows)
    {
        var board = boards.FirstOrDefault();
        if (board is not null)
        {
            Add(items, "Firmware", "Baseboard", "Manufacturer", Text(board, "Manufacturer"), "Win32_BaseBoard");
            Add(items, "Firmware", "Baseboard", "Product", Text(board, "Product"), "Win32_BaseBoard");
            Add(items, "Firmware", "Baseboard", "Version", Text(board, "Version"), "Win32_BaseBoard");
            Add(items, "Firmware", "Baseboard", "Serial number", Text(board, "SerialNumber"), "Win32_BaseBoard");
        }
        var bios = biosRows.FirstOrDefault();
        if (bios is null) return;
        Add(items, "Firmware", "BIOS / UEFI", "Manufacturer", Text(bios, "Manufacturer"), "Win32_BIOS");
        Add(items, "Firmware", "BIOS / UEFI", "Version", Text(bios, "SMBIOSBIOSVersion"), "Win32_BIOS");
        Add(items, "Firmware", "BIOS / UEFI", "Release date", Date(bios, "ReleaseDate"), "Win32_BIOS");
        Add(items, "Firmware", "BIOS / UEFI", "SMBIOS", $"{Text(bios, "SMBIOSMajorVersion")}.{Text(bios, "SMBIOSMinorVersion")}", "Win32_BIOS");
        Add(items, "Firmware", "BIOS / UEFI", "Serial number", Text(bios, "SerialNumber"), "Win32_BIOS");
    }

    private static void AddGraphics(List<SystemInfoItem> items, IReadOnlyList<IReadOnlyDictionary<string, object?>> rows)
    {
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            var component = Text(row, "Name", $"Graphics adapter {i + 1}");
            Add(items, "Graphics", component, "Processor", Text(row, "VideoProcessor"), "Win32_VideoController");
            Add(items, "Graphics", component, "Adapter memory", Bytes(ULong(row, "AdapterRAM")), "Win32_VideoController");
            Add(items, "Graphics", component, "Driver version", Text(row, "DriverVersion"), "Win32_VideoController");
            Add(items, "Graphics", component, "Driver date", Date(row, "DriverDate"), "Win32_VideoController");
            var width = Text(row, "CurrentHorizontalResolution");
            var height = Text(row, "CurrentVerticalResolution");
            Add(items, "Graphics", component, "Current mode", string.IsNullOrWhiteSpace(width) ? null : $"{width} × {height} @ {Text(row, "CurrentRefreshRate")} Hz", "Win32_VideoController");
            Add(items, "Graphics", component, "Status", Text(row, "Status"), "Win32_VideoController");
            Add(items, "Graphics", component, "Device identifier", Text(row, "PNPDeviceID"), "Win32_VideoController");
        }
    }

    private static void AddMonitors(List<SystemInfoItem> items, IReadOnlyList<IReadOnlyDictionary<string, object?>> rows)
    {
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            var component = Text(row, "Name", $"Display {i + 1}");
            Add(items, "Graphics", component, "Manufacturer", Text(row, "MonitorManufacturer"), "Win32_DesktopMonitor");
            var width = Text(row, "ScreenWidth");
            Add(items, "Graphics", component, "Reported resolution", string.IsNullOrWhiteSpace(width) ? null : $"{width} × {Text(row, "ScreenHeight")}", "Win32_DesktopMonitor");
            Add(items, "Graphics", component, "Status", Text(row, "Status"), "Win32_DesktopMonitor");
            Add(items, "Graphics", component, "Device identifier", Text(row, "PNPDeviceID"), "Win32_DesktopMonitor");
        }
    }

    private static void AddStorage(List<SystemInfoItem> items,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> disks,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> volumes)
    {
        foreach (var row in disks)
        {
            var component = Text(row, "Model", $"Physical disk {Text(row, "Index")}");
            Add(items, "Storage", component, "Capacity", Bytes(ULong(row, "Size")), "Win32_DiskDrive");
            Add(items, "Storage", component, "Media type", Text(row, "MediaType"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Interface", Text(row, "InterfaceType"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Firmware", Text(row, "FirmwareRevision"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Partitions", Text(row, "Partitions"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Serial number", Text(row, "SerialNumber"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Status", Text(row, "Status"), "Win32_DiskDrive");
            Add(items, "Storage", component, "Device identifier", Text(row, "PNPDeviceID"), "Win32_DiskDrive");
        }
        foreach (var row in volumes)
        {
            var id = Text(row, "DeviceID", "Volume");
            var label = Text(row, "VolumeName");
            var component = string.IsNullOrWhiteSpace(label) ? id : $"{id}  {label}";
            var size = ULong(row, "Size");
            var free = ULong(row, "FreeSpace");
            Add(items, "Storage", component, "File system", Text(row, "FileSystem"), "Win32_LogicalDisk");
            Add(items, "Storage", component, "Capacity", Bytes(size), "Win32_LogicalDisk");
            Add(items, "Storage", component, "Free space", Bytes(free), "Win32_LogicalDisk");
            if (size > 0) Add(items, "Storage", component, "Free percentage", $"{100d * free / size:0.0}%", "Calculated");
            Add(items, "Storage", component, "Volume serial", Text(row, "VolumeSerialNumber"), "Win32_LogicalDisk");
        }
    }

    private static void AddNetwork(List<SystemInfoItem> items,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> adapters,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> configurations)
    {
        foreach (var row in adapters)
        {
            var component = Text(row, "NetConnectionID", Text(row, "Name", "Network adapter"));
            Add(items, "Network", component, "Adapter", Text(row, "Name"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Description", Text(row, "Description"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Manufacturer", Text(row, "Manufacturer"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Type", Text(row, "AdapterType"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Link speed", BitsPerSecond(ULong(row, "Speed")), "Win32_NetworkAdapter");
            Add(items, "Network", component, "MAC address", Text(row, "MACAddress"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Connection state", NetworkStatus(ULong(row, "NetConnectionStatus")), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Physical adapter", Text(row, "PhysicalAdapter"), "Win32_NetworkAdapter");
            Add(items, "Network", component, "Device identifier", Text(row, "PNPDeviceID"), "Win32_NetworkAdapter");
        }
        foreach (var row in configurations)
        {
            var component = Text(row, "Description", "IP configuration");
            Add(items, "Network", component, "IP addresses", Text(row, "IPAddress"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "Subnets", Text(row, "IPSubnet"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "Default gateways", Text(row, "DefaultIPGateway"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "DNS servers", Text(row, "DNSServerSearchOrder"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "DNS domain", Text(row, "DNSDomain"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "DHCP", Text(row, "DHCPEnabled"), "Win32_NetworkAdapterConfiguration");
            Add(items, "Network", component, "DHCP server", Text(row, "DHCPServer"), "Win32_NetworkAdapterConfiguration");
        }
    }

    private static void AddPowerAndAudio(List<SystemInfoItem> items,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> batteries,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> audioDevices)
    {
        foreach (var row in batteries)
        {
            var component = Text(row, "Name", "Battery");
            Add(items, "Power & devices", component, "Charge remaining", $"{Text(row, "EstimatedChargeRemaining")}%", "Win32_Battery");
            Add(items, "Power & devices", component, "Battery state", BatteryStatus(ULong(row, "BatteryStatus")), "Win32_Battery");
            Add(items, "Power & devices", component, "Estimated runtime", Minutes(ULong(row, "EstimatedRunTime")), "Win32_Battery");
            Add(items, "Power & devices", component, "Design voltage", Millivolts(ULong(row, "DesignVoltage")), "Win32_Battery");
            Add(items, "Power & devices", component, "Status", Text(row, "Status"), "Win32_Battery");
        }
        foreach (var row in audioDevices)
        {
            var component = Text(row, "Name", "Audio device");
            Add(items, "Power & devices", component, "Manufacturer", Text(row, "Manufacturer"), "Win32_SoundDevice");
            Add(items, "Power & devices", component, "Status", Text(row, "Status"), "Win32_SoundDevice");
            Add(items, "Power & devices", component, "Device identifier", Text(row, "PNPDeviceID"), "Win32_SoundDevice");
        }
    }

    private static void Add(List<SystemInfoItem> items, string category, string component, string property, string? value, string source)
    {
        value = value?.Trim();
        if (string.IsNullOrWhiteSpace(value) || value is "0 B" or "0 KB" or "0 Mb/s" or "%") return;
        items.Add(new SystemInfoItem(category, component.Trim(), property, value, source));
    }

    private static string Text(IReadOnlyDictionary<string, object?>? row, string key, string fallback = "")
    {
        if (row is null || !row.TryGetValue(key, out var value) || value is null) return fallback;
        return value switch
        {
            bool boolean => YesNo(boolean),
            string text => text.Trim(),
            string[] strings => string.Join(", ", strings.Where(text => !string.IsNullOrWhiteSpace(text))),
            object?[] values => string.Join(", ", values.Where(item => item is not null).Select(item => Convert.ToString(item, CultureInfo.InvariantCulture))),
            Array array => string.Join(", ", array.Cast<object?>().Where(item => item is not null).Select(item => Convert.ToString(item, CultureInfo.InvariantCulture))),
            _ => Convert.ToString(value, CultureInfo.InvariantCulture) ?? fallback
        };
    }

    private static ulong ULong(IReadOnlyDictionary<string, object?>? row, string key)
    {
        if (row is null || !row.TryGetValue(key, out var value) || value is null) return 0;
        try { return Convert.ToUInt64(value, CultureInfo.InvariantCulture); }
        catch { return 0; }
    }

    private static bool Bool(IReadOnlyDictionary<string, object?> row, string key) =>
        row.TryGetValue(key, out var value) && value is not null &&
        (value is bool boolean ? boolean : bool.TryParse(Convert.ToString(value, CultureInfo.InvariantCulture), out var parsed) && parsed);

    private static DateTime? DateValue(IReadOnlyDictionary<string, object?> row, string key)
    {
        if (!row.TryGetValue(key, out var value) || value is null) return null;
        if (value is DateTime dateTime) return dateTime;
        var text = Convert.ToString(value, CultureInfo.InvariantCulture);
        if (string.IsNullOrWhiteSpace(text)) return null;
        try { return ManagementDateTimeConverter.ToDateTime(text); }
        catch { return DateTime.TryParse(text, CultureInfo.CurrentCulture, DateTimeStyles.AssumeLocal, out var parsed) ? parsed : null; }
    }

    private static string Date(IReadOnlyDictionary<string, object?> row, string key) => DateValue(row, key)?.ToString("g") ?? string.Empty;
    private static string NegatedBool(IReadOnlyDictionary<string, object?> row, string key) => YesNo(!Bool(row, key));
    private static string BuildVersion(IReadOnlyDictionary<string, object?>? row) => row is null ? string.Empty : $"{Text(row, "Version")} (build {Text(row, "BuildNumber")})";
    private static string RuntimeInformationLabel() => System.Runtime.InteropServices.RuntimeInformation.OSArchitecture.ToString();
    private static string JoinNonEmpty(string first, string second, string fallback) =>
        string.Join(" ", new[] { first, second }.Where(value => !string.IsNullOrWhiteSpace(value))) is { Length: > 0 } joined ? joined : fallback;
    private static string YesNo(bool value) => value ? "Yes" : "No";
    private static string Bytes(ulong bytes) => bytes switch
    {
        0 => string.Empty,
        < 1024 => $"{bytes:N0} B",
        < 1_048_576 => $"{bytes / 1024d:N1} KB",
        < 1_073_741_824 => $"{bytes / 1_048_576d:N1} MB",
        < 1_099_511_627_776 => $"{bytes / 1_073_741_824d:N2} GB",
        _ => $"{bytes / 1_099_511_627_776d:N2} TB"
    };
    private static string Kibibytes(ulong kibibytes) => kibibytes == 0 ? string.Empty : Bytes(kibibytes * 1024);
    private static string FrequencyMHz(ulong mhz) => mhz == 0 ? string.Empty : $"{mhz / 1000d:0.00} GHz ({mhz:N0} MHz)";
    private static string MegaTransfers(ulong speed) => speed == 0 ? string.Empty : $"{speed:N0} MT/s";
    private static string BitsPerSecond(ulong speed) => speed == 0 ? string.Empty : speed >= 1_000_000_000 ? $"{speed / 1_000_000_000d:0.##} Gb/s" : $"{speed / 1_000_000d:0.##} Mb/s";
    private static string Millivolts(ulong value) => value == 0 ? string.Empty : $"{value / 1000d:0.###} V";
    private static string Minutes(ulong value) => value is 0 or 71582788 ? string.Empty : $"{value:N0} min";
    private static string Duration(TimeSpan value) => value.TotalDays >= 1 ? $"{(int)value.TotalDays}d {value.Hours}h {value.Minutes}m" : $"{(int)value.TotalHours}h {value.Minutes}m";
    private static string MemoryType(ulong value) => value switch { 20 => "DDR", 21 => "DDR2", 24 => "DDR3", 26 => "DDR4", 30 => "LPDDR4", 34 => "DDR5", 35 => "LPDDR5", 0 => string.Empty, _ => $"SMBIOS type {value}" };
    private static string MemoryFormFactor(ulong value) => value switch { 8 => "DIMM", 12 => "SODIMM", 13 => "SRIMM", 0 => string.Empty, _ => $"Form factor {value}" };
    private static string NetworkStatus(ulong value) => value switch { 0 => "Disconnected", 1 => "Connecting", 2 => "Connected", 3 => "Disconnecting", 7 => "Media disconnected", 12 => "Credentials required", _ => value == 0 ? string.Empty : $"Status {value}" };
    private static string BatteryStatus(ulong value) => value switch { 1 => "Discharging", 2 => "AC power", 3 => "Fully charged", 6 => "Charging", 7 => "Charging / high", 8 => "Charging / low", 9 => "Charging / critical", 11 => "Partially charged", _ => value == 0 ? string.Empty : $"Status {value}" };
    private static string VbsStatus(string value) => value switch { "0" => "Disabled", "1" => "Enabled but not running", "2" => "Enabled and running", _ => value };
    private static string LicenseStatus(ulong value) => value switch { 0 => "Unlicensed", 1 => "Licensed", 2 => "Initial grace", 3 => "Additional grace", 4 => "Non-genuine grace", 5 => "Notification", 6 => "Extended grace", _ => $"Status {value}" };
    private static string BitLockerProtection(ulong value) => value switch { 0 => "Protection off", 1 => "Protection on", 2 => "Unknown", _ => $"Status {value}" };
    private static string BitLockerConversion(ulong value) => value switch { 0 => "Fully decrypted", 1 => "Fully encrypted", 2 => "Encrypting", 3 => "Decrypting", 4 => "Encryption paused", 5 => "Decryption paused", _ => $"Status {value}" };
    private static string BitLockerMethod(ulong value) => value switch { 0 => "None", 1 => "AES 128 with diffuser", 2 => "AES 256 with diffuser", 3 => "AES 128", 4 => "AES 256", 5 => "Hardware encryption", 6 => "XTS-AES 128", 7 => "XTS-AES 256", _ => $"Method {value}" };
    private static bool IsAdministrator()
    {
        using var identity = WindowsIdentity.GetCurrent();
        return new WindowsPrincipal(identity).IsInRole(WindowsBuiltInRole.Administrator);
    }
    private static int CategoryOrder(string category) => category switch
    {
        "Overview" => 0, "Windows" => 1, "Security" => 2, "Processor" => 3, "Memory" => 4,
        "Firmware" => 5, "Graphics" => 6, "Storage" => 7, "Network" => 8, "Power & devices" => 9, _ => 99
    };
}
