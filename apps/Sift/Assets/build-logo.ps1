$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class SiftIconNative {
  [DllImport("user32.dll")] public static extern bool DestroyIcon(IntPtr handle);
}
'@

$size = 256
$bitmap = [System.Drawing.Bitmap]::new($size, $size)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
try {
  $graphics.Clear([System.Drawing.ColorTranslator]::FromHtml('#1D1A17'))

  $ringPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#413930'), 2)
  try { $graphics.DrawEllipse($ringPen, 24, 24, 208, 208) } finally { $ringPen.Dispose() }

  $sage = [System.Drawing.ColorTranslator]::FromHtml('#A9BB98')
  $layers = @(
    @{ X = 49; Y = 73; W = 158; H = 18; A = 82 },
    @{ X = 59; Y = 103; W = 138; H = 18; A = 132 },
    @{ X = 69; Y = 133; W = 118; H = 18; A = 200 }
  )
  foreach ($layer in $layers) {
    $brush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb($layer.A, $sage))
    try { $graphics.FillRectangle($brush, $layer.X, $layer.Y, $layer.W, $layer.H) } finally { $brush.Dispose() }
  }

  $clay = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#D89462'), 14)
  $clay.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
  $clay.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
  $clay.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round
  try {
    $graphics.DrawBezier($clay,
      [System.Drawing.PointF]::new(128, 49),
      [System.Drawing.PointF]::new(196, 49),
      [System.Drawing.PointF]::new(201, 117),
      [System.Drawing.PointF]::new(172, 164))
    $graphics.DrawBezier($clay,
      [System.Drawing.PointF]::new(172, 164),
      [System.Drawing.PointF]::new(201, 200),
      [System.Drawing.PointF]::new(196, 234),
      [System.Drawing.PointF]::new(128, 217))
    $graphics.DrawBezier($clay,
      [System.Drawing.PointF]::new(128, 217),
      [System.Drawing.PointF]::new(84, 205),
      [System.Drawing.PointF]::new(55, 175),
      [System.Drawing.PointF]::new(55, 150))
    $graphics.DrawBezier($clay,
      [System.Drawing.PointF]::new(55, 150),
      [System.Drawing.PointF]::new(55, 110),
      [System.Drawing.PointF]::new(84, 83),
      [System.Drawing.PointF]::new(128, 83))
  } finally { $clay.Dispose() }

  $dot = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#D89462'))
  try { $graphics.FillEllipse($dot, 158, 150, 17, 17) } finally { $dot.Dispose() }

  $pngPath = Join-Path $PSScriptRoot 'SiftLogo.png'
  $bitmap.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
  $iconHandle = $bitmap.GetHicon()
  try {
    $icon = [System.Drawing.Icon]::FromHandle($iconHandle)
    $stream = [System.IO.File]::Create((Join-Path $PSScriptRoot 'Sift.ico'))
    try { $icon.Save($stream) } finally { $stream.Dispose(); $icon.Dispose() }
  } finally { [SiftIconNative]::DestroyIcon($iconHandle) | Out-Null }
  Write-Host "Regenerated SiftLogo.png and Sift.ico with the Sift sieve identity."
}
finally {
  $graphics.Dispose()
  $bitmap.Dispose()
}
