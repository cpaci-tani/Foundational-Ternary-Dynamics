using Sift.Models;

namespace Sift.WinUI.Composition;

// Typed widget-action intents raised by the Home dashboard view and consumed by the dashboard action
// router and workspace module. They live in the composition layer rather than a view code-behind so
// the router contract (IDashboardActionRouter) does not depend on a specific view's internals.
public sealed record DashboardWidgetMoveIntent(string InstanceId, DashboardBreakpoint Breakpoint, int Row, int Column);
public sealed record DashboardWidgetSizeIntent(string InstanceId, DashboardBreakpoint Breakpoint, int RowSpan, int ColumnSpan);
public sealed record DashboardWidgetVisibilityIntent(string InstanceId, DashboardBreakpoint Breakpoint, bool AllBreakpoints);
public sealed record DashboardWidgetConfigurationIntent(
    string InstanceId, string? Title, string? Accent, string TimeRange,
    string? Metric, string? Sensor, string? Volume, string Sort, int Count, string? Filter, bool ShowActions);
public sealed record DashboardWidgetActionIntent(
    string InstanceId,
    DashboardActionKind Action,
    ProcessSnapshot? Process = null,
    DashboardServiceSnapshot? Service = null,
    string? AlertId = null);
public sealed record DashboardWidgetPlacementIntent(
    string InstanceId, DashboardBreakpoint Breakpoint, int Row, int Column, int RowSpan, int ColumnSpan);
