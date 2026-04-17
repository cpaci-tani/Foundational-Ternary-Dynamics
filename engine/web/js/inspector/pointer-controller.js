export function bindInspectorPointerControls({
    viewport,
    dragThresholdPx = 6,
    onClick,
    onEscape,
}) {
    const canvas = viewport?.renderer?.domElement;
    if (!canvas) return () => {};

    let activePointerId = null;
    let pointerStart = null;
    let pointerMoved = false;

    const handlePointerDown = (e) => {
        if (e.button !== 0) return;
        activePointerId = e.pointerId;
        pointerStart = { x: e.clientX, y: e.clientY };
        pointerMoved = false;
    };

    const handlePointerMove = (e) => {
        if (activePointerId !== e.pointerId || !pointerStart) return;
        const dx = e.clientX - pointerStart.x;
        const dy = e.clientY - pointerStart.y;
        if (Math.hypot(dx, dy) > dragThresholdPx) pointerMoved = true;
    };

    const finishPointer = (e) => {
        if (activePointerId !== e.pointerId) return;
        const start = pointerStart;
        const wasClick = !!start && !pointerMoved &&
            Math.hypot(e.clientX - start.x, e.clientY - start.y) <= dragThresholdPx;
        activePointerId = null;
        pointerStart = null;
        pointerMoved = false;
        if (wasClick) onClick?.(e);
    };

    const handlePointerCancel = (e) => {
        if (activePointerId !== e.pointerId) return;
        activePointerId = null;
        pointerStart = null;
        pointerMoved = false;
    };

    const handleKeyDown = (e) => {
        if (e.key === 'Escape') onEscape?.(e);
    };

    canvas.addEventListener('pointerdown', handlePointerDown);
    window.addEventListener('pointermove', handlePointerMove);
    window.addEventListener('pointerup', finishPointer);
    window.addEventListener('pointercancel', handlePointerCancel);
    document.addEventListener('keydown', handleKeyDown);

    return () => {
        canvas.removeEventListener('pointerdown', handlePointerDown);
        window.removeEventListener('pointermove', handlePointerMove);
        window.removeEventListener('pointerup', finishPointer);
        window.removeEventListener('pointercancel', handlePointerCancel);
        document.removeEventListener('keydown', handleKeyDown);
    };
}
