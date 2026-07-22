import { useEffect, useRef } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import type { Camera3DState, CurveAnalysis, CurveSample, CurveStyle, PedagogySettings } from "../core/types";

interface Props {
  sample: CurveSample;
  style: CurveStyle;
  cameraState: Camera3DState;
  phase: number;
  analysis: CurveAnalysis | null;
  pedagogy: PedagogySettings;
  canvasRef: React.RefObject<HTMLCanvasElement>;
  fitNonce: number;
  onCameraChange: (camera: Camera3DState) => void;
}

export function ThreeViewport({ sample, style, cameraState, phase, analysis, pedagogy, canvasRef, fitNonce, onCameraChange }: Props) {
  const hostRef = useRef<HTMLDivElement>(null);
  const appliedFitRef = useRef(0);
  const runtimeRef = useRef<{
    renderer: THREE.WebGLRenderer;
    scene: THREE.Scene;
    camera: THREE.PerspectiveCamera;
    controls: OrbitControls;
    line: THREE.Line;
    marker: THREE.Mesh;
    grid: THREE.GridHelper;
    axes: THREE.AxesHelper;
    tangent: THREE.ArrowHelper;
    normal: THREE.ArrowHelper;
    binormal: THREE.ArrowHelper;
    osculatingCircle: THREE.LineLoop;
    radiusLine: THREE.Line;
    projectionLines: THREE.LineSegments;
  } | null>(null);

  useEffect(() => {
    const host = hostRef.current;
    const canvas = canvasRef.current;
    if (!host || !canvas) return;
    const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: false, preserveDrawingBuffer: true });
    renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    renderer.setClearColor(0x08100e, 1);
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(cameraState.fov, 1, 0.001, 10000);
    camera.position.fromArray(cameraState.position);
    const controls = new OrbitControls(camera, canvas);
    controls.enableDamping = true;
    controls.target.fromArray(cameraState.target);
    const line = new THREE.Line(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({ color: style.color }));
    const marker = new THREE.Mesh(
      new THREE.SphereGeometry(0.05, 20, 12),
      new THREE.MeshBasicMaterial({ color: 0xf4f8f6 })
    );
    const grid = new THREE.GridHelper(20, 20, 0x5e7770, 0x20302c);
    grid.rotation.x = Math.PI / 2;
    const axes = new THREE.AxesHelper(2);
    const tangent = new THREE.ArrowHelper(new THREE.Vector3(1, 0, 0), undefined, 1, 0xe7b453);
    const normal = new THREE.ArrowHelper(new THREE.Vector3(0, 1, 0), undefined, 1, 0xef7d8d);
    const binormal = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), undefined, 1, 0x70a8ff);
    const osculatingCircle = new THREE.LineLoop(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({ color: 0xe7b453, transparent: true, opacity: 0.7 }));
    const radiusLine = new THREE.Line(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({ color: 0xf4f8f6, transparent: true, opacity: 0.68 }));
    const projectionLines = new THREE.LineSegments(new THREE.BufferGeometry(), new THREE.LineDashedMaterial({ color: 0x64d8c8, transparent: true, opacity: 0.55, dashSize: 0.08, gapSize: 0.05 }));
    scene.add(line, marker, grid, axes, tangent, normal, binormal, osculatingCircle, radiusLine, projectionLines);
    runtimeRef.current = { renderer, scene, camera, controls, line, marker, grid, axes, tangent, normal, binormal, osculatingCircle, radiusLine, projectionLines };

    const resize = () => {
      const width = Math.max(1, host.clientWidth);
      const height = Math.max(1, host.clientHeight);
      renderer.setSize(width, height, false);
      camera.aspect = width / height;
      camera.updateProjectionMatrix();
    };
    const observer = new ResizeObserver(resize);
    observer.observe(host);
    resize();
    let frame = 0;
    const animate = () => {
      controls.update();
      renderer.render(scene, camera);
      frame = requestAnimationFrame(animate);
    };
    animate();
    const emitCamera = () => onCameraChange({
      position: camera.position.toArray() as [number, number, number],
      target: controls.target.toArray() as [number, number, number],
      fov: camera.fov
    });
    controls.addEventListener("end", emitCamera);
    return () => {
      cancelAnimationFrame(frame);
      observer.disconnect();
      controls.removeEventListener("end", emitCamera);
      controls.dispose();
      line.geometry.dispose();
      (line.material as THREE.Material).dispose();
      marker.geometry.dispose();
      (marker.material as THREE.Material).dispose();
      [tangent, normal, binormal].forEach((arrow) => {
        arrow.line.geometry.dispose();
        (arrow.line.material as THREE.Material).dispose();
        arrow.cone.geometry.dispose();
        (arrow.cone.material as THREE.Material).dispose();
      });
      osculatingCircle.geometry.dispose();
      (osculatingCircle.material as THREE.Material).dispose();
      radiusLine.geometry.dispose();
      (radiusLine.material as THREE.Material).dispose();
      projectionLines.geometry.dispose();
      (projectionLines.material as THREE.Material).dispose();
      renderer.dispose();
      runtimeRef.current = null;
    };
  }, []);

  useEffect(() => {
    const runtime = runtimeRef.current;
    if (!runtime) return;
    runtime.line.geometry.dispose();
    runtime.line.geometry = new THREE.BufferGeometry();
    runtime.line.geometry.setAttribute("position", new THREE.BufferAttribute(sample.positions, 3));
    const colors = new Float32Array(sample.count * 3);
    const first = new THREE.Color(style.color);
    const second = new THREE.Color(style.secondaryColor);
    for (let index = 0; index < sample.count; index += 1) {
      const color = first.clone().lerp(second, index / Math.max(1, sample.count - 1));
      color.toArray(colors, index * 3);
    }
    runtime.line.geometry.setAttribute("color", new THREE.BufferAttribute(colors, 3));
    runtime.line.geometry.setDrawRange(0, style.drawScene ? 1 : sample.count);
    const material = runtime.line.material as THREE.LineBasicMaterial;
    material.vertexColors = true;
    material.needsUpdate = true;
    runtime.grid.visible = style.showGrid;
    runtime.axes.visible = style.showAxes;
  }, [sample, style]);

  useEffect(() => {
    const runtime = runtimeRef.current;
    if (!runtime || !sample.count) return;
    const visibleCount = style.drawScene ? Math.max(1, Math.floor(phase * (sample.count - 1)) + 1) : sample.count;
    runtime.line.geometry.setDrawRange(0, visibleCount);
    const index = style.drawScene ? visibleCount - 1 : Math.min(sample.count - 1, Math.floor(phase * (sample.count - 1)));
    runtime.marker.position.fromArray(sample.positions, index * 3);
    runtime.marker.scale.setScalar(Math.max(sample.bounds.radius * 0.025, 0.02));
    runtime.marker.visible = pedagogy.pointMarker;
  }, [pedagogy.pointMarker, phase, sample, style.drawScene]);

  useEffect(() => {
    const runtime = runtimeRef.current;
    if (!runtime || !analysis) return;
    const origin = new THREE.Vector3(...analysis.position);
    const length = Math.max(sample.bounds.radius * 0.3 * pedagogy.vectorScale, 1e-3);
    const updateArrow = (arrow: THREE.ArrowHelper, vector: [number, number, number], visible: boolean) => {
      arrow.visible = visible && Math.hypot(...vector) > 1e-9;
      if (!arrow.visible) return;
      arrow.position.copy(origin);
      arrow.setDirection(new THREE.Vector3(...vector).normalize());
      arrow.setLength(length, Math.min(length * 0.22, 0.25), Math.min(length * 0.12, 0.12));
    };
    updateArrow(runtime.tangent, analysis.tangent, pedagogy.tangent);
    updateArrow(runtime.normal, analysis.normal, pedagogy.normal);
    updateArrow(runtime.binormal, analysis.binormal, pedagogy.binormal);
    runtime.radiusLine.visible = pedagogy.radiusVector;
    if (runtime.radiusLine.visible) {
      runtime.radiusLine.geometry.dispose();
      runtime.radiusLine.geometry = new THREE.BufferGeometry().setFromPoints([new THREE.Vector3(), origin]);
    }
    runtime.projectionLines.visible = pedagogy.projections;
    if (runtime.projectionLines.visible) {
      const xy = new THREE.Vector3(analysis.position[0], analysis.position[1], 0);
      const xAxis = new THREE.Vector3(analysis.position[0], 0, 0);
      runtime.projectionLines.geometry.dispose();
      runtime.projectionLines.geometry = new THREE.BufferGeometry().setFromPoints([origin, xy, xy, xAxis, xAxis, new THREE.Vector3()]);
      runtime.projectionLines.computeLineDistances();
    }
    const radius = analysis.osculatingRadius;
    runtime.osculatingCircle.visible = Boolean(pedagogy.osculatingCircle && radius && radius <= sample.bounds.radius * 4);
    if (runtime.osculatingCircle.visible && radius) {
      const center = origin.clone().add(new THREE.Vector3(...analysis.normal).multiplyScalar(radius));
      const tangent = new THREE.Vector3(...analysis.tangent);
      const normal = new THREE.Vector3(...analysis.normal);
      const points = Array.from({ length: 96 }, (_, index) => {
        const angle = index / 96 * Math.PI * 2;
        return center.clone().add(normal.clone().multiplyScalar(-radius * Math.cos(angle))).add(tangent.clone().multiplyScalar(radius * Math.sin(angle)));
      });
      runtime.osculatingCircle.geometry.dispose();
      runtime.osculatingCircle.geometry = new THREE.BufferGeometry().setFromPoints(points);
    }
  }, [analysis, pedagogy, sample.bounds.radius]);

  useEffect(() => {
    const runtime = runtimeRef.current;
    if (!runtime) return;
    runtime.camera.position.fromArray(cameraState.position);
    runtime.camera.fov = cameraState.fov;
    runtime.camera.updateProjectionMatrix();
    runtime.controls.target.fromArray(cameraState.target);
    runtime.controls.update();
  }, [cameraState]);

  useEffect(() => {
    const runtime = runtimeRef.current;
    if (!runtime || !fitNonce || appliedFitRef.current === fitNonce) return;
    appliedFitRef.current = fitNonce;
    const radius = sample.bounds.radius;
    const center = new THREE.Vector3().fromArray(sample.bounds.center);
    const distance = radius / Math.tan(THREE.MathUtils.degToRad(runtime.camera.fov / 2)) * 1.25;
    const direction = runtime.camera.position.clone().sub(runtime.controls.target).normalize();
    runtime.controls.target.copy(center);
    runtime.camera.position.copy(center).add(direction.multiplyScalar(distance));
    runtime.controls.update();
    onCameraChange({
      position: runtime.camera.position.toArray() as [number, number, number],
      target: center.toArray() as [number, number, number],
      fov: runtime.camera.fov
    });
  }, [fitNonce, onCameraChange, sample.bounds]);

  return (
    <div className="viewport-host" ref={hostRef}>
      <canvas ref={canvasRef} aria-label="Three-dimensional mathematical curve view" />
      <div className="viewport-label">R³ · spatial frame</div>
    </div>
  );
}
