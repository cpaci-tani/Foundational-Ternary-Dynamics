# Sift.Core

`Sift.Core` is Sift's presentation-neutral application layer. It physically owns all models,
activity and operation infrastructure, persistence, scanners, inventories, execution services, and
guarded system policy used by the WinUI 3 client.

No WPF or WinUI types belong in this assembly. Preflight, rollback, protected-process,
protected-service, and protected-task policy remain below the presentation boundary and are tested
without constructing a desktop window.
