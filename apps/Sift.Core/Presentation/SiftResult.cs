namespace Sift.Presentation;

/// <summary>
/// Typed success/failure for Core policy outcomes. Prefer this over ad-hoc bool+string pairs
/// when a call site already has a stable <see cref="SiftReasonCode"/>.
/// </summary>
public readonly struct SiftResult
{
    private SiftResult(bool success, SiftReasonCode reasonCode, string message)
    {
        IsSuccess = success;
        ReasonCode = reasonCode;
        Message = message;
    }

    public bool IsSuccess { get; }
    public bool IsFailure => !IsSuccess;
    public SiftReasonCode ReasonCode { get; }
    public string Message { get; }

    public static SiftResult Ok() => new(true, SiftReasonCode.Unspecified, string.Empty);

    public static SiftResult Fail(SiftReasonCode code, params object?[] args) =>
        new(false, code, ReasonMessages.Format(code, args));

    public static SiftResult Fail(SiftReasonCode code, string message) =>
        new(false, code, string.IsNullOrWhiteSpace(message) ? ReasonMessages.Format(code) : message);
}

/// <summary>
/// Typed success/failure that carries a value on success.
/// </summary>
public readonly struct SiftResult<T>
{
    private SiftResult(bool success, T? value, SiftReasonCode reasonCode, string message)
    {
        IsSuccess = success;
        Value = value;
        ReasonCode = reasonCode;
        Message = message;
    }

    public bool IsSuccess { get; }
    public bool IsFailure => !IsSuccess;
    public T? Value { get; }
    public SiftReasonCode ReasonCode { get; }
    public string Message { get; }

    public static SiftResult<T> Ok(T value) => new(true, value, SiftReasonCode.Unspecified, string.Empty);

    public static SiftResult<T> Fail(SiftReasonCode code, params object?[] args) =>
        new(false, default, code, ReasonMessages.Format(code, args));

    public static SiftResult<T> Fail(SiftReasonCode code, string message) =>
        new(false, default, code, string.IsNullOrWhiteSpace(message) ? ReasonMessages.Format(code) : message);

    public SiftResult AsResult() =>
        IsSuccess ? SiftResult.Ok() : SiftResult.Fail(ReasonCode, Message);
}
