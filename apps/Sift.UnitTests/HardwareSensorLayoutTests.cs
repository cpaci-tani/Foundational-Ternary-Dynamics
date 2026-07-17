using Sift.Services;

namespace Sift.UnitTests;

public sealed class HardwareSensorLayoutTests
{
    [Theory]
    [InlineData("Load", "CPU Core #1", true)]
    [InlineData("Load", "CPU Core #16", true)]
    [InlineData("Clock", "Core #3", true)]
    [InlineData("Clock", "Core #3 (Effective)", true)]
    [InlineData("Load", "CPU Total", false)]
    [InlineData("Load", "CPU Core Max", false)]
    [InlineData("Clock", "Cores (Average)", false)]
    [InlineData("Clock", "Bus Speed", false)]
    [InlineData("Temperature", "Core (Tctl/Tdie)", false)]
    [InlineData("Voltage", "Core #1 VID", false)]
    public void IsPerCoreSensor_classifies_expected_names(string type, string name, bool expected) =>
        Assert.Equal(expected, HardwareSensorLayout.IsPerCoreSensor(type, name));

    [Fact]
    public void CoreShortLabel_and_sort_key()
    {
        Assert.Equal("#12", HardwareSensorLayout.CoreShortLabel("CPU Core #12"));
        Assert.Equal(12, HardwareSensorLayout.CoreSortKey("Core #12 (Effective)"));
        Assert.True(HardwareSensorLayout.TypeOrder("Load") < HardwareSensorLayout.TypeOrder("Fan"));
    }
}
