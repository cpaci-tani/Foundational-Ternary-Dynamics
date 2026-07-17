using Sift.Models;

namespace Sift.Services;

internal static class ScriptRecipeCatalog
{
    public static IReadOnlyList<ScriptRecipe> Create()
    {
        var rows = new List<ScriptRecipe>();
        void Add(string id, string title, string category, string description, string command,
            ScriptShell shell = ScriptShell.Cmd, ScriptRisk risk = ScriptRisk.ReadOnly, bool admin = false,
            bool network = false, bool sensitive = false) =>
            rows.Add(new(id, title, category, description, shell, command, risk, admin, network, sensitive));

        Add("net-ipconfig-all", "Full IP configuration", "Network", "Addresses, adapters, gateways, and DNS.", "ipconfig /all");
        Add("net-ipconfig-brief", "Brief IP configuration", "Network", "Current adapter addresses.", "ipconfig");
        Add("net-dns-cache", "Show DNS resolver cache", "DNS", "Inspect cached DNS answers.", "ipconfig /displaydns");
        Add("net-dns-flush", "Flush DNS resolver cache", "DNS", "Clears locally cached DNS answers.", "ipconfig /flushdns", risk: ScriptRisk.ChangesState, admin: true);
        Add("net-arp", "ARP neighbor cache", "Network", "Show IPv4 neighbor mappings.", "arp -a");
        Add("net-route", "IPv4 route table", "Network", "Show active routes.", "route print -4");
        Add("net-route6", "IPv6 route table", "Network", "Show active IPv6 routes.", "route print -6");
        Add("net-netstat", "Active connections", "Network", "Show TCP/UDP endpoints with owning PIDs.", "netstat -ano");
        Add("net-netstat-stats", "Protocol statistics", "Network", "Show protocol counters.", "netstat -s");
        Add("net-netstat-route", "Connections and routes", "Network", "Show endpoints and routing table.", "netstat -r");
        Add("net-wlan", "Wi-Fi interface state", "Wi-Fi", "Show current Wi-Fi radio and link details.", "netsh wlan show interfaces");
        Add("net-wlan-drivers", "Wi-Fi driver capabilities", "Wi-Fi", "Show installed WLAN driver features.", "netsh wlan show drivers");
        Add("net-wlan-networks", "Visible Wi-Fi networks", "Wi-Fi", "Scan visible SSIDs without connecting.", "netsh wlan show networks mode=bssid", network: true);
        Add("net-winhttp", "WinHTTP proxy", "Network", "Show machine WinHTTP proxy settings.", "netsh winhttp show proxy");
        Add("net-interface", "Network interfaces", "Network", "Show IPv4 interface metrics and state.", "netsh interface ipv4 show interfaces");
        Add("net-dnsservers", "Configured DNS servers", "DNS", "Show DNS configuration per interface.", "netsh interface ip show dns");
        Add("net-ping-gateway", "Test local gateway name", "Network", "Test the conventional gateway hostname if resolvable.", "ping -n 4 gateway", network: true);
        Add("net-ping-cloudflare", "Test internet reachability", "Network", "Four ICMP probes to 1.1.1.1.", "ping -n 4 1.1.1.1", network: true);
        Add("net-trace-cloudflare", "Trace internet route", "Network", "Trace up to 20 hops to 1.1.1.1.", "tracert -d -h 20 1.1.1.1", network: true);
        Add("net-nslookup", "Test DNS resolution", "DNS", "Resolve microsoft.com using configured DNS.", "nslookup microsoft.com", network: true);
        Add("net-nbt", "NetBIOS name cache", "Network", "Show NetBIOS names and cache.", "nbtstat -n && nbtstat -c");
        Add("net-winsock", "Winsock catalog", "Network", "Inspect registered Winsock providers.", "netsh winsock show catalog");
        Add("net-winsock-reset", "Reset Winsock catalog", "Network", "Resets Winsock; restart may be required.", "netsh winsock reset", risk: ScriptRisk.Advanced, admin: true);

        Add("sys-systeminfo", "System summary", "System", "Windows, hardware, hotfix, and boot summary.", "systeminfo");
        Add("sys-ver", "Windows version", "System", "Show command processor OS version.", "ver");
        Add("sys-whoami", "Current identity", "Security", "Show current account and groups.", "whoami /all");
        Add("sys-env", "Environment variables", "System", "Show Script Studio's sanitized command-library environment. Output may contain account paths and identifiers.", "set", sensitive: true);
        Add("sys-hostname", "Computer name", "System", "Show local host name.", "hostname");
        Add("sys-drivers", "Signed driver inventory", "Drivers", "List installed drivers.", "driverquery /v");
        Add("sys-drivers-csv", "Driver inventory CSV", "Drivers", "List drivers in readable CSV form.", "driverquery /fo csv /v");
        Add("sys-pnp", "Problem devices", "Drivers", "PnP devices reporting an error.", "Get-PnpDevice | Where-Object Status -ne 'OK' | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("sys-hotfix", "Installed Windows updates", "Updates", "List installed hotfix records.", "Get-HotFix | Sort-Object InstalledOn -Descending | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("sys-features", "Optional feature states", "System", "List Windows optional features without changing them.", "Get-WindowsOptionalFeature -Online | Sort-Object State,FeatureName | Format-Table -AutoSize", ScriptShell.PowerShell, admin: true);
        Add("sys-capabilities", "Windows capabilities", "System", "List capability installation states.", "Get-WindowsCapability -Online | Sort-Object State,Name | Format-Table -AutoSize", ScriptShell.PowerShell, admin: true);
        Add("sys-license", "Windows license status", "System", "Show concise activation status.", "cscript //nologo %windir%\\system32\\slmgr.vbs /dli");
        Add("sys-time", "Time service status", "Time", "Show Windows Time synchronization state.", "w32tm /query /status");
        Add("sys-time-peers", "Time service peers", "Time", "Show configured time peers.", "w32tm /query /peers");
        Add("sys-time-config", "Time service configuration", "Time", "Show effective time settings.", "w32tm /query /configuration");
        Add("sys-locale", "Regional settings", "System", "Show culture and regional configuration.", "Get-Culture; Get-WinSystemLocale; Get-WinUserLanguageList", ScriptShell.PowerShell);

        Add("disk-volumes", "Volumes", "Storage", "Show mounted volumes and free space.", "Get-Volume | Sort-Object DriveLetter | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("disk-physical", "Physical disks", "Storage", "Show physical media health and bus type.", "Get-PhysicalDisk | Format-Table FriendlyName,MediaType,BusType,HealthStatus,OperationalStatus,Size -AutoSize", ScriptShell.PowerShell);
        Add("disk-logical", "Logical disks", "Storage", "Show capacity and free space.", "Get-CimInstance Win32_LogicalDisk | Select-Object DeviceID,Description,FileSystem,FreeSpace,Size,VolumeName | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("disk-partitions", "Disk partitions", "Storage", "Show partition layout.", "Get-Partition | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("disk-health", "Storage reliability counters", "Storage", "Read available device wear and error counters.", "Get-PhysicalDisk | Get-StorageReliabilityCounter | Format-List", ScriptShell.PowerShell, admin: true);
        Add("disk-dirty", "System volume dirty bit", "Storage", "Query whether Windows scheduled a filesystem check.", "fsutil dirty query %SystemDrive%", admin: true);
        Add("disk-chkdsk", "System volume check", "Storage", "Online read-only filesystem check.", "chkdsk %SystemDrive% /scan", admin: true);
        Add("disk-shadow", "Restore snapshots", "Recovery", "List volume shadow copies.", "vssadmin list shadows", admin: true);
        Add("disk-shadow-storage", "Snapshot storage", "Recovery", "Show VSS storage allocation.", "vssadmin list shadowstorage", admin: true);
        Add("disk-bitlocker", "BitLocker status", "Security", "Read encryption and protector status.", "manage-bde -status", admin: true);
        Add("disk-compact", "Compact OS status", "Storage", "Query Compact OS state.", "compact.exe /compactos:query");
        Add("disk-temp-size", "User temp footprint", "Storage", "Measure files under the current user's temp folder.", "$x=Get-ChildItem $env:TEMP -File -Recurse -ErrorAction SilentlyContinue | Measure-Object Length -Sum; '{0:N2} MB in {1:N0} files' -f ($x.Sum/1MB),$x.Count", ScriptShell.PowerShell);
        Add("disk-recycle", "Recycle Bin footprint", "Storage", "Measure current user's Recycle Bin.", "$x=(New-Object -ComObject Shell.Application).Namespace(0xA).Items(); '{0:N0} items' -f $x.Count", ScriptShell.PowerShell);

        Add("health-sfc-verify", "Verify protected system files", "Repair", "Non-repairing SFC verification.", "sfc /verifyonly", admin: true);
        Add("health-sfc-scan", "Repair protected system files", "Repair", "Runs SFC repair against protected files.", "sfc /scannow", risk: ScriptRisk.ChangesState, admin: true);
        Add("health-dism-check", "Check component store", "Repair", "Quick component-store corruption check.", "dism /online /cleanup-image /checkhealth", admin: true);
        Add("health-dism-scan", "Scan component store", "Repair", "Deep non-repairing component-store scan.", "dism /online /cleanup-image /scanhealth", admin: true);
        Add("health-dism-restore", "Repair component store from local sources", "Repair", "Repairs from configured local sources only; Windows Update access is disabled.", "dism /online /cleanup-image /restorehealth /limitaccess", risk: ScriptRisk.ChangesState, admin: true);
        Add("health-dism-analyze", "Analyze component store", "Storage", "Reports reclaimable component-store space.", "dism /online /cleanup-image /analyzecomponentstore", admin: true);
        Add("health-memory", "Memory diagnostics schedule", "Diagnostics", "Opens the built-in memory diagnostic UI; no automatic reboot.", "mdsched.exe");
        Add("health-reliability", "Reliability Monitor", "Diagnostics", "Opens Windows Reliability Monitor.", "perfmon /rel");
        Add("health-perf-report", "Performance diagnostics", "Diagnostics", "Runs the built-in system diagnostics collector.", "perfmon /report", admin: true);
        Add("health-resource", "Resource Monitor", "Diagnostics", "Opens Windows Resource Monitor.", "resmon");
        Add("health-dxdiag", "DirectX diagnostics", "Diagnostics", "Opens DirectX Diagnostic Tool.", "dxdiag");

        Add("proc-list", "Running processes", "Processes", "Show processes with memory and CPU time.", "Get-Process | Sort-Object WorkingSet64 -Descending | Select-Object -First 100 Name,Id,CPU,WorkingSet64,Path | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("proc-tasklist", "Task list with services", "Processes", "Map processes to hosted services.", "tasklist /svc");
        Add("proc-modules", "Process module inventory", "Processes", "Show loaded modules where permitted.", "tasklist /m");
        Add("proc-startup", "Startup commands", "Startup", "Show registered startup entries.", "Get-CimInstance Win32_StartupCommand | Select-Object Caption,Command,Location,User | Format-Table -Wrap", ScriptShell.PowerShell);
        Add("proc-scheduled", "Scheduled task summary", "Startup", "Read scheduled task states and next runs.", "schtasks /query /fo table /v");
        Add("proc-services", "Service inventory", "Services", "List services by state and start mode.", "Get-CimInstance Win32_Service | Sort-Object State,Name | Format-Table Name,State,StartMode,DisplayName -AutoSize", ScriptShell.PowerShell);
        Add("proc-failed-services", "Stopped automatic services", "Services", "Find automatic services that are not running.", "Get-CimInstance Win32_Service | Where-Object {$_.StartMode -eq 'Auto' -and $_.State -ne 'Running'} | Format-Table Name,State,StartMode,DisplayName -AutoSize", ScriptShell.PowerShell);
        Add("proc-handles", "Open shared files", "Processes", "Show files opened through Windows file sharing.", "openfiles /query", admin: true);
        Add("proc-sessions", "User sessions", "Processes", "Show local and remote logon sessions.", "query user");

        Add("sec-firewall", "Firewall profiles", "Security", "Show enabled state and policy for all profiles.", "netsh advfirewall show allprofiles");
        Add("sec-defender", "Microsoft Defender status", "Security", "Read protection state and signature age.", "Get-MpComputerStatus | Format-List", ScriptShell.PowerShell);
        Add("sec-threats", "Detected threats", "Security", "Read Defender threat history.", "Get-MpThreatDetection | Sort-Object InitialDetectionTime -Descending | Select-Object -First 30 | Format-List", ScriptShell.PowerShell);
        Add("sec-secureboot", "Secure Boot state", "Security", "Query UEFI Secure Boot.", "Confirm-SecureBootUEFI", ScriptShell.PowerShell, admin: true);
        Add("sec-tpm", "TPM state", "Security", "Read TPM readiness without changing it.", "Get-Tpm | Format-List", ScriptShell.PowerShell, admin: true);
        Add("sec-audit", "Audit policy", "Security", "Show effective advanced audit policy.", "auditpol /get /category:*", admin: true);
        Add("sec-shares", "Network shares", "Security", "List local SMB shares.", "net share");
        Add("sec-smb", "SMB configuration", "Security", "Read SMB server security settings.", "Get-SmbServerConfiguration | Format-List EnableSMB1Protocol,EnableSMB2Protocol,EncryptData,RequireSecuritySignature", ScriptShell.PowerShell, admin: true);
        Add("sec-users", "Local users", "Security", "List local accounts and enabled state.", "Get-LocalUser | Format-Table Name,Enabled,LastLogon,PasswordExpires -AutoSize", ScriptShell.PowerShell);
        Add("sec-admins", "Local administrators", "Security", "List members of the built-in Administrators group by well-known SID.", "$g=Get-LocalGroup -SID 'S-1-5-32-544'; Get-LocalGroupMember -Group $g | Format-Table -AutoSize", ScriptShell.PowerShell, admin: true);
        Add("sec-connections", "Listening endpoints", "Security", "Show listening TCP ports and owning PIDs.", "Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Format-Table LocalAddress,LocalPort,OwningProcess -AutoSize", ScriptShell.PowerShell);
        Add("sec-rdp", "Remote Desktop setting", "Security", "Read whether Remote Desktop connections are permitted.", "Get-ItemProperty 'HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server' -Name fDenyTSConnections | Format-List", ScriptShell.PowerShell);
        Add("sec-uac", "UAC policy", "Security", "Read core User Account Control policy values.", "Get-ItemProperty 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System' EnableLUA,ConsentPromptBehaviorAdmin,PromptOnSecureDesktop | Format-List", ScriptShell.PowerShell);

        Add("evt-system-errors", "Recent system errors", "Event logs", "Latest 50 System errors.", "Get-WinEvent -FilterHashtable @{LogName='System';Level=2} -MaxEvents 50 | Format-Table TimeCreated,Id,ProviderName,Message -Wrap", ScriptShell.PowerShell);
        Add("evt-app-errors", "Recent application errors", "Event logs", "Latest 50 Application errors.", "Get-WinEvent -FilterHashtable @{LogName='Application';Level=2} -MaxEvents 50 | Format-Table TimeCreated,Id,ProviderName,Message -Wrap", ScriptShell.PowerShell);
        Add("evt-critical", "Recent critical events", "Event logs", "Latest critical System events.", "Get-WinEvent -FilterHashtable @{LogName='System';Level=1} -MaxEvents 30 | Format-List TimeCreated,Id,ProviderName,Message", ScriptShell.PowerShell);
        Add("evt-boot", "Boot performance events", "Event logs", "Recent Diagnostics-Performance boot events.", "Get-WinEvent -FilterHashtable @{LogName='Microsoft-Windows-Diagnostics-Performance/Operational';Id=100} -MaxEvents 20 | Format-List TimeCreated,Message", ScriptShell.PowerShell);
        Add("evt-update", "Windows Update events", "Updates", "Recent Windows Update client events.", "Get-WinEvent -FilterHashtable @{LogName='System';ProviderName='Microsoft-Windows-WindowsUpdateClient'} -MaxEvents 40 | Format-Table TimeCreated,Id,LevelDisplayName,Message -Wrap", ScriptShell.PowerShell);
        Add("evt-defender", "Defender events", "Security", "Recent Defender operational events.", "Get-WinEvent -LogName 'Microsoft-Windows-Windows Defender/Operational' -MaxEvents 40 | Format-Table TimeCreated,Id,LevelDisplayName,Message -Wrap", ScriptShell.PowerShell);
        Add("evt-crashes", "Application crash events", "Event logs", "Recent Application Error event 1000 records.", "Get-WinEvent -FilterHashtable @{LogName='Application';Id=1000} -MaxEvents 30 | Format-List TimeCreated,Message", ScriptShell.PowerShell);
        Add("evt-shutdowns", "Unexpected shutdowns", "Event logs", "Recent Kernel-Power event 41 records.", "Get-WinEvent -FilterHashtable @{LogName='System';Id=41} -MaxEvents 20 | Format-List TimeCreated,Message", ScriptShell.PowerShell);
        Add("evt-whea", "Hardware error events", "Event logs", "Recent WHEA hardware reports.", "Get-WinEvent -FilterHashtable @{LogName='System';ProviderName='Microsoft-Windows-WHEA-Logger'} -MaxEvents 30 | Format-List TimeCreated,Id,Message", ScriptShell.PowerShell);
        Add("evt-disk", "Disk warning events", "Event logs", "Recent disk provider warnings and errors.", "Get-WinEvent -FilterHashtable @{LogName='System';ProviderName='disk';Level=2,3} -MaxEvents 30 | Format-List TimeCreated,Id,Message", ScriptShell.PowerShell);

        Add("upd-history", "Windows Update history", "Updates", "Read installed update history through Windows Update API.", "$s=New-Object -ComObject Microsoft.Update.Session; $q=$s.CreateUpdateSearcher().QueryHistory(0,50); $q | Select-Object Date,Title,ResultCode | Format-Table -Wrap", ScriptShell.PowerShell);
        Add("upd-reboot", "Pending reboot indicators", "Updates", "Check well-known reboot-pending markers.", "$p=@('HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootPending','HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired'); $p | ForEach-Object { [pscustomobject]@{Path=$_;Present=Test-Path $_} } | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("upd-services", "Update service states", "Updates", "Read Windows Update and transfer service states.", "Get-Service wuauserv,bits,cryptsvc,TrustedInstaller | Format-Table Name,Status,StartType -AutoSize", ScriptShell.PowerShell);
        Add("upd-store", "Microsoft Store package inventory", "Apps", "List installed Store packages without changing them.", "Get-AppxPackage | Sort-Object Name | Select-Object Name,Version,Publisher | Format-Table -AutoSize", ScriptShell.PowerShell);
        Add("power-config", "Active power scheme", "Power", "Show the active Windows power plan.", "powercfg /getactivescheme");
        Add("power-list", "Power schemes", "Power", "List available power plans.", "powercfg /list");
        Add("power-requests", "Power requests", "Power", "Show processes preventing sleep or display idle.", "powercfg /requests", admin: true);
        Add("power-wake", "Wake-capable devices", "Power", "Show devices allowed to wake the PC.", "powercfg /devicequery wake_armed");
        Add("power-lastwake", "Last wake source", "Power", "Show what last woke the computer.", "powercfg /lastwake");
        Add("power-sleep", "Supported sleep states", "Power", "Show available and unavailable sleep modes.", "powercfg /a");
        Add("power-battery", "Battery inventory", "Power", "Read battery status through CIM.", "Get-CimInstance Win32_Battery | Format-List", ScriptShell.PowerShell);
        Add("power-cpu", "Processor power settings", "Power", "Show processor settings in the active scheme.", "powercfg /query scheme_current sub_processor");

        Add("bash-kernel", "WSL kernel and architecture", "WSL / Bash", "Run uname in the default WSL distribution.", "uname -a", ScriptShell.Bash);
        Add("bash-disk", "WSL filesystem space", "WSL / Bash", "Show WSL mount usage.", "df -h", ScriptShell.Bash);
        Add("bash-memory", "WSL memory view", "WSL / Bash", "Show memory visible to WSL.", "free -h", ScriptShell.Bash);
        Add("bash-processes", "WSL process snapshot", "WSL / Bash", "Show top WSL processes by CPU.", "ps -eo pid,ppid,comm,%cpu,%mem --sort=-%cpu | head -25", ScriptShell.Bash);
        Add("bash-network", "WSL network interfaces", "WSL / Bash", "Show WSL network addresses.", "ip -brief address", ScriptShell.Bash);
        Add("bash-routes", "WSL routes", "WSL / Bash", "Show WSL route table.", "ip route", ScriptShell.Bash);
        Add("bash-dns", "WSL resolver configuration", "WSL / Bash", "Show current WSL resolver file.", "sed -n '1,80p' /etc/resolv.conf", ScriptShell.Bash);
        Add("bash-mounts", "WSL mounts", "WSL / Bash", "Show mounted filesystems.", "findmnt -D", ScriptShell.Bash);
        Add("bash-release", "WSL distribution release", "WSL / Bash", "Show Linux distribution identity.", "sed -n '1,80p' /etc/os-release", ScriptShell.Bash);
        Add("bash-uptime", "WSL uptime and load", "WSL / Bash", "Show WSL uptime and load averages.", "uptime", ScriptShell.Bash);

        var unknownCategories = rows.Select(recipe => recipe.Category).Distinct()
            .Where(category => !ScriptRecipeTaxonomy.IsKnown(category)).ToList();
        if (unknownCategories.Count > 0)
            throw new InvalidOperationException($"Script catalog contains unknown categories: {string.Join(", ", unknownCategories)}");
        return rows.OrderBy(recipe => ScriptRecipeTaxonomy.OrderOf(recipe.Category))
            .ThenBy(recipe => recipe.Title, StringComparer.OrdinalIgnoreCase).ToList();
    }
}
