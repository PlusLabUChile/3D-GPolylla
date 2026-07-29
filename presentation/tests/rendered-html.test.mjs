import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";

async function render() {
  const workerUrl = new URL("../dist/server/index.js", import.meta.url);
  workerUrl.searchParams.set("test", `${process.pid}-${Date.now()}`);
  const { default: worker } = await import(workerUrl.href);

  return worker.fetch(
    new Request("http://localhost/", { headers: { accept: "text/html" } }),
    { ASSETS: { fetch: async () => new Response("Not found", { status: 404 }) } },
    { waitUntil() {}, passThroughOnException() {} },
  );
}

test("renders the complete autonomous presentation", async () => {
  const response = await render();
  assert.equal(response.status, 200);
  assert.match(response.headers.get("content-type") ?? "", /^text\/html\b/i);

  const html = await response.text();
  assert.match(html, /<html[^>]+lang="es"/i);
  assert.match(html, /<title>Rediseño de 3D-GPolylla<\/title>/i);
  assert.equal((html.match(/data-presentation-section=/g) ?? []).length, 17);
  assert.equal((html.match(/data-visual=/g) ?? []).length, 17);

  for (const visual of [
    "north-star",
    "before-after",
    "priorities",
    "architecture",
    "axes",
    "memory",
    "eigen",
    "pipeline",
    "extensibility",
    "backends",
    "adapters",
    "executables",
    "build",
    "testing",
    "style",
    "migration",
    "acceptance",
  ]) {
    assert.match(html, new RegExp(`data-visual="${visual}"`));
  }

  for (const id of [
    "north-star",
    "why-redesign",
    "priorities",
    "architecture",
    "three-axes",
    "model-memory",
    "eigen",
    "pipeline",
    "extensibility",
    "parallel-cuda",
    "statistics-adapters",
    "executables",
    "build-portability",
    "testing",
    "style-guide",
    "migration",
    "success-risks-sources",
  ]) {
    assert.match(html, new RegExp(`id="${id}"`));
  }

  assert.match(html, /familia de algoritmos de generación de poliedros/i);
  assert.match(html, /Eigen calcula; GPolylla controla/i);
  assert.match(html, /Label/);
  assert.match(html, /Construct/);
  assert.match(html, /Repair/);
  assert.match(html, /CPU secuencial/);
  assert.match(html, /CUDA/);
  assert.match(html, /CLI/);
  assert.match(html, /Experiments/);
  assert.match(html, /Demo/);
  assert.match(html, /href="#content"[^>]*>\s*Saltar al contenido/i);
  assert.match(html, /aria-label="Navegación de lectura"/i);
  assert.match(html, /aria-label="Progreso de lectura"/i);
  assert.match(html, /href="#north-star"/i);
  assert.match(html, /href="#success-risks-sources"/i);
  assert.doesNotMatch(html, /codex-preview|loading skeleton|taking shape/i);
});

test("keeps fonts local and includes responsive accessibility rules", async () => {
  const [layout, css] = await Promise.all([
    readFile(new URL("../app/layout.tsx", import.meta.url), "utf8"),
    readFile(new URL("../app/globals.css", import.meta.url), "utf8"),
  ]);

  assert.doesNotMatch(layout, /next\/font|fonts\.googleapis|codex-preview/i);
  assert.match(css, /prefers-reduced-motion:\s*reduce/);
  assert.match(css, /@media\s*\(max-width:\s*760px\)/);
  assert.match(css, /:focus-visible/);
  assert.match(css, /--paper:\s*#f4f0e8/i);
  assert.match(css, /--ink:\s*#102a2e/i);
  assert.match(css, /--copper:\s*#a8502a/i);
  assert.match(css, /--teal:\s*#176b68/i);
  assert.match(css, /\.visual--pipeline/);
  assert.match(css, /\.visual--memory/);
  assert.match(css, /\.visual--migration/);
  assert.match(css, /\.mobile-nav/);
  assert.match(css, /\.toc/);
});
