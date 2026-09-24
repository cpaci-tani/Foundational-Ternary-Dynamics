import type { Scene, DataTexture } from 'three';
import type { RGBELoader } from 'three/addons/loaders/RGBELoader.js';
export const HDRI_BASE: string;
export const HDRI_ENVIRONMENTS: Record<string, {label: string; file: string}>;
export function createHDRILoader(): RGBELoader;
export function loadHDRI(loader: RGBELoader, file: string, onLoad: (texture: DataTexture) => void, onError?: (error: unknown, url: string) => void): void;
export function applyHDRITexture(scene: Scene, texture: DataTexture): void;
