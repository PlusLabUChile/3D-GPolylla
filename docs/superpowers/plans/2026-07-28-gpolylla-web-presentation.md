# GPolylla Architecture Web Presentation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and deploy a simple, autonomous, single-page web presentation that explains the complete 3D-GPolylla redesign document through 17 readable chapters and purpose-built HTML/CSS diagrams.

**Architecture:** A standalone Sites/vinext application lives under `presentation/` and remains independent from the CMake build. Static typed content drives a semantic server-rendered page; one small client component provides current-section highlighting and reading progress, while all essential content and anchor navigation continue to work without JavaScript.

**Tech Stack:** TypeScript 5.9, React 19, Next-compatible vinext, Vite 8, CSS, Node test runner, Cloudflare Sites hosting.

## Global Constraints

- The source of truth is `docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md`.
- The presentation is a single route with 17 chapters in the exact approved order.
- The experience targets autonomous reading, not live slide delivery.
- Use a light ivory background, near-black petroleum text, copper decisions, and teal modules.
- Use system fonts only; no external font, image, API, authentication, persistence, D1, or R2 dependency.
- Build diagrams with semantic HTML and CSS; do not add authored SVG illustrations.
- Keep every essential idea visible without interaction; `details` is only for supporting material.
- Preserve keyboard navigation, focus visibility, semantic headings, sufficient contrast, and `prefers-reduced-motion`.
- Keep the presentation outside the CMake targets and do not modify the C++ implementation.
- Remove all starter loading content and the `react-loading-skeleton` dependency.
- Do not modify the user's untracked `AGENTS.md`.

---

## File Structure

```text
presentation/
|-- .openai/hosting.json          # Sites capability declaration; no D1 or R2
|-- app/
|   |-- globals.css               # Complete visual system and responsive layout
|   |-- layout.tsx                # Spanish document shell and metadata
|   |-- page.tsx                  # Semantic composition of the 17 chapters
|   |-- presentation-data.ts      # Typed chapter copy and navigation labels
|   |-- reading-navigation.tsx    # Current chapter and reading progress
|   `-- section-visual.tsx        # Purpose-built HTML diagrams
|-- build/sites-vite-plugin.ts    # Generated Sites packaging plugin
|-- tests/rendered-html.test.mjs  # Rendered-content and source-level checks
|-- worker/index.ts               # Generated vinext worker entry point
|-- package.json
|-- package-lock.json
|-- README.md
|-- tsconfig.json
`-- vite.config.ts
```

The generated `db/`, `drizzle.config.ts`, and auth helper may remain unused because
they do not affect the build and are part of the Sites starter. Do not introduce a
database binding in `.openai/hosting.json`.

---

### Task 1: Scaffold the standalone site and replace the starter with a semantic shell

**Files:**
- Create: `presentation/` using the Sites initializer
- Create: `presentation/app/presentation-data.ts`
- Modify: `presentation/app/layout.tsx`
- Modify: `presentation/app/page.tsx`
- Modify: `presentation/package.json`
- Modify: `presentation/package-lock.json`
- Modify: `presentation/README.md`
- Replace: `presentation/tests/rendered-html.test.mjs`
- Delete: `presentation/app/_sites-preview/SkeletonPreview.tsx`
- Delete: `presentation/app/_sites-preview/preview.css`
- Delete: `presentation/public/favicon.svg`
- Delete: `presentation/public/file.svg`
- Delete: `presentation/public/globe.svg`
- Delete: `presentation/public/window.svg`

**Interfaces:**
- Consumes: the approved design specification and the Sites starter.
- Produces: `PresentationSection`, `sections`, a Spanish document shell, 17 semantic
  sections, and a production-render test used by all later tasks.

- [ ] **Step 1: Initialize the site and attach it to the parent repository**

Run from `/Users/vgonzalez/Documents/Tesis/3D-GPolylla`:

```bash
/Users/vgonzalez/.codex/plugins/cache/openai-bundled/sites/0.1.31/scripts/init-site.sh presentation
mv presentation/.git /private/tmp/gpolylla-presentation-nested-git
```

Expected: dependencies install successfully, `presentation/.openai/hosting.json`
contains `{ "d1": null, "r2": null }`, and `git status --short` reports files under
the parent repository rather than a nested repository.

- [ ] **Step 2: Replace the starter test with the failing product contract**

Replace `presentation/tests/rendered-html.test.mjs` with:

```javascript
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
});
```

- [ ] **Step 3: Run the product test and verify the starter fails it**

Run:

```bash
cd presentation
npm test
```

Expected: FAIL because the starter title, language, 17 sections, and presentation
content are absent.

- [ ] **Step 4: Define the exact chapter data**

Create `presentation/app/presentation-data.ts`:

```typescript
export type VisualKind =
  | "north-star"
  | "before-after"
  | "priorities"
  | "architecture"
  | "axes"
  | "memory"
  | "eigen"
  | "pipeline"
  | "extensibility"
  | "backends"
  | "adapters"
  | "executables"
  | "build"
  | "testing"
  | "style"
  | "migration"
  | "acceptance";

export interface PresentationSection {
  id: string;
  index: number;
  label: string;
  title: string;
  thesis: string;
  bullets: readonly string[];
  detail?: string;
  visual: VisualKind;
}

export const sections = [
  {
    id: "north-star",
    index: 1,
    label: "North star",
    title: "Una familia, no un algoritmo aislado",
    thesis:
      "3D-GPolylla será la consolidación de una familia de algoritmos de generación de poliedros.",
    bullets: [
      "El repositorio se organiza alrededor del dominio y no de la demo.",
      "Face, Edge, Cavity y algoritmos futuros comparten lenguaje, no estado interno.",
      "La arquitectura debe aceptar variación sin propagar cambios innecesarios.",
    ],
    visual: "north-star",
  },
  {
    id: "why-redesign",
    index: 2,
    label: "Problema actual",
    title: "El prototipo ya probó el dominio; ahora limita su evolución",
    thesis:
      "La deuda principal no es estética: ownership, dependencias y responsabilidades están mezclados.",
    bullets: [
      "El modelo público es mutable y Vertex hereda de Eigen.",
      "Cavity retiene estado y usa recorridos difíciles de paralelizar.",
      "Renderer y CLI mezclan UI, estadísticas, experimentos y algoritmos.",
      "Dependencias opcionales se filtran al target central.",
    ],
    visual: "before-after",
  },
  {
    id: "priorities",
    index: 3,
    label: "Prioridades",
    title: "Las decisiones siguen un orden explícito",
    thesis:
      "La eficiencia importa, pero nunca a costa de perder corrección o control observable.",
    bullets: [
      "1. Corrección algorítmica.",
      "2. Extensibilidad localizada.",
      "3. Control de memoria y rendimiento.",
      "4. Portabilidad y reproducibilidad.",
    ],
    detail:
      "Usabilidad completa la lista, mientras compatibilidad con la API anterior, plugins binarios y CI empresarial quedan fuera del alcance inicial.",
    visual: "priorities",
  },
  {
    id: "architecture",
    index: 4,
    label: "Arquitectura",
    title: "Microkernel lógico con pipelines tipados",
    thesis:
      "Las fronteras públicas son pequeñas; la especialización de alto rendimiento permanece privada.",
    bullets: [
      "No hay una jerarquía universal para estados que son distintos.",
      "No se expone un producto cartesiano de templates en la API.",
      "La selección runtime existe sólo en el borde de las aplicaciones.",
    ],
    visual: "architecture",
  },
  {
    id: "three-axes",
    index: 5,
    label: "Tres ejes",
    title: "Model, statistics y algorithms expresan el repositorio",
    thesis:
      "Adaptadores y aplicaciones rodean esos ejes; CPU y CUDA no forman un cuarto eje público.",
    bullets: [
      "Model posee datos e invariantes.",
      "Statistics mide sin conocer algoritmos.",
      "Algorithms transforma modelos mediante pipelines explícitos.",
      "Las dependencias siempre apuntan hacia componentes más estables.",
    ],
    visual: "axes",
  },
  {
    id: "model-memory",
    index: 6,
    label: "Modelo y memoria",
    title: "El modelo canónico controla ownership y layout",
    thesis:
      "Geometría, topología y atributos viven en buffers separados, contiguos y validados.",
    bullets: [
      "IDs fuertes evitan mezclar vértices, tetraedros y poliedros.",
      "MeshBuilder concentra la construcción mutable.",
      "La malla validada es inmutable durante cada ejecución.",
      "Cada algoritmo crea un workspace privado y tailored.",
    ],
    visual: "memory",
  },
  {
    id: "eigen",
    index: 7,
    label: "Eigen",
    title: "Eigen es una herramienta matemática, no el modelo",
    thesis: "Eigen calcula; GPolylla controla memoria, flujo y concurrencia.",
    bullets: [
      "Ningún tipo Eigen cruza la API pública.",
      "Eigen::Map interpreta buffers privados en operaciones acotadas.",
      "El paralelismo de Eigen se deshabilita dentro de la biblioteca.",
      "Los kernels CUDA no dependen de Eigen.",
    ],
    visual: "eigen",
  },
  {
    id: "pipeline",
    index: 8,
    label: "Pipeline",
    title: "Tres fases conservan la semántica",
    thesis:
      "Prepare y Materialize son fronteras técnicas; Label, Construct y Repair son el contrato algorítmico.",
    bullets: [
      "Label calcula información local y candidatos.",
      "Construct forma ownership, regiones y fronteras.",
      "Repair detecta, resuelve y verifica defectos.",
      "Cada fase tiene postcondiciones y seams de prueba.",
    ],
    visual: "pipeline",
  },
  {
    id: "extensibility",
    index: 9,
    label: "Extensibilidad",
    title: "Cada tipo de cambio tiene un único punto de extensión",
    thesis:
      "Formatos, criterios, algoritmos, workspaces, backends y métricas evolucionan de forma independiente.",
    bullets: [
      "Los formatos producen el modelo canónico.",
      "Los criterios son policies tipadas sin virtual dispatch por elemento.",
      "Cada algoritmo define sus estados y representación interna.",
      "Las aplicaciones registran sólo combinaciones compiladas.",
    ],
    visual: "extensibility",
  },
  {
    id: "parallel-cuda",
    index: 10,
    label: "CPU y CUDA",
    title: "Una semántica, implementaciones especializadas",
    thesis:
      "CPU secuencial es la referencia; CPU paralelo y CUDA optimizan layout y scheduling sin alterar el resultado observable.",
    bullets: [
      "El workspace se prepara una vez y evita asignaciones en hot paths.",
      "La equivalencia compara particiones canonicalizadas e invariantes.",
      "CUDA transfiere al inicio y materializa al final.",
      "Auto puede hacer fallback; una solicitud CUDA obligatoria falla explícitamente.",
    ],
    visual: "backends",
  },
  {
    id: "statistics-adapters",
    index: 11,
    label: "Estadísticas e IO",
    title: "Métricas centrales; dependencias avanzadas en la periferia",
    thesis:
      "Statistics y los formatos son módulos explícitos, no métodos acumulados dentro de Mesh.",
    bullets: [
      "statistics::core depende sólo de model.",
      "QuickHull y polyhedron-kernel son adaptadores opcionales.",
      "TetGen, VisF y OFF construyen TetrahedralMesh mediante MeshBuilder.",
      "Agregar un formato no modifica ningún algoritmo.",
    ],
    visual: "adapters",
  },
  {
    id: "executables",
    index: 12,
    label: "Ejecutables",
    title: "CLI, experiments y demo tienen razones distintas para cambiar",
    thesis:
      "Los ejecutables coordinan casos de uso; la construcción de poliedros permanece en la biblioteca.",
    bullets: [
      "CLI interpreta, ejecuta y escribe resultados.",
      "Experiments registra campañas reproducibles.",
      "Demo separa application, rendering y UI.",
      "La visualización sirve para manual testing sin dictar la API.",
    ],
    visual: "executables",
  },
  {
    id: "build-portability",
    index: 13,
    label: "Build y portabilidad",
    title: "La biblioteca base no arrastra capacidades opcionales",
    thesis:
      "CMake exporta targets pequeños y vcpkg reproduce el entorno sin imponerse al consumidor.",
    bullets: [
      "find_package(GPolylla) instala únicamente las capacidades solicitadas.",
      "No se fijan dependencias a ramas main o master.",
      "Demo, CUDA y estadísticas avanzadas son opciones separadas.",
      "CPU C++20 es común a Linux, Windows y macOS.",
    ],
    visual: "build",
  },
  {
    id: "testing",
    index: 14,
    label: "Pruebas y robustez",
    title: "Corrección, robustez y rendimiento son preguntas diferentes",
    thesis:
      "La referencia secuencial ayuda, pero los oráculos reales son casos conocidos, contratos e invariantes.",
    bullets: [
      "Unit tests cubren modelo, estadísticas, fases y criterios.",
      "Pruebas metamórficas cubren permutación y transformaciones.",
      "Backends se comparan mediante resultados canonicalizados.",
      "Experiments registra dataset, configuración, seed y entorno.",
    ],
    visual: "testing",
  },
  {
    id: "style-guide",
    index: 15,
    label: "Guía de estilo",
    title: "El estilo también protege las fronteras",
    thesis:
      "Formato, nombres, ownership, reglas numéricas y hot paths se verifican desde un único target local.",
    bullets: [
      "LLVM, cuatro espacios y columnas de 100 caracteres.",
      "PascalCase para tipos; camelCase para funciones y variables.",
      "Sin raw owning pointers, estado global o logging desde la biblioteca.",
      "check reúne formato, clang-tidy, warnings, tests y reglas arquitectónicas.",
    ],
    visual: "style",
  },
  {
    id: "migration",
    index: 16,
    label: "Migración",
    title: "Rediseño profundo, entrega incremental",
    thesis:
      "Una ruta nueva y verificable sustituye la implementación actual después de demostrar cortes verticales.",
    bullets: [
      "Caracterizar antes de reemplazar.",
      "Construir modelo y primer Cavity CPU completo.",
      "Extender familia y luego paralelizar.",
      "Migrar aplicaciones, probar CUDA y finalmente hacer cutover.",
    ],
    visual: "migration",
  },
  {
    id: "success-risks-sources",
    index: 17,
    label: "Cierre",
    title: "El diseño es exitoso cuando el cambio queda localizado",
    thesis:
      "La arquitectura se evalúa con evidencia: extensión independiente, equivalencia, portabilidad y reproducción.",
    bullets: [
      "Un formato nuevo no modifica modelo o algoritmos.",
      "Un backend nuevo no cambia la API observable.",
      "CLI, experiments y demo no contienen lógica algorítmica.",
      "Cada experimento se reconstruye desde dataset, configuración y seed.",
    ],
    detail:
      "Los riesgos principales son sofisticación excesiva, explosión de combinaciones, conversiones de layout, divergencia de backends y una frontera GPU sólo teórica. El corte vertical y el spike CUDA los convierten en verificaciones concretas.",
    visual: "acceptance",
  },
] as const satisfies readonly PresentationSection[];
```

- [ ] **Step 5: Replace metadata and render the semantic sections**

Replace `presentation/app/layout.tsx` with:

```tsx
import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Rediseño de 3D-GPolylla",
  description:
    "Presentación autónoma de la arquitectura propuesta para consolidar una familia de algoritmos de generación de poliedros.",
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="es">
      <body>{children}</body>
    </html>
  );
}
```

Replace `presentation/app/page.tsx` with this initial semantic version:

```tsx
import { sections } from "./presentation-data";

const sourceDocument =
  "https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md";

export default function Home() {
  return (
    <>
      <a className="skip-link" href="#content">Saltar al contenido</a>
      <nav aria-label="Índice de capítulos">
        <ol>
          {sections.map((section) => (
            <li key={section.id}><a href={`#${section.id}`}>{section.label}</a></li>
          ))}
        </ol>
      </nav>
      <main id="content">
        {sections.map((section) => (
          <section
            id={section.id}
            key={section.id}
            data-presentation-section
            aria-labelledby={`${section.id}-title`}
          >
            <p>Capítulo {String(section.index).padStart(2, "0")}</p>
            <h2 id={`${section.id}-title`}>{section.title}</h2>
            <p>{section.thesis}</p>
            <ul>{section.bullets.map((bullet) => <li key={bullet}>{bullet}</li>)}</ul>
            {section.detail ? <details><summary>Profundizar</summary><p>{section.detail}</p></details> : null}
          </section>
        ))}
        <footer>
          <a href={sourceDocument}>Leer el documento técnico completo</a>
        </footer>
      </main>
    </>
  );
}
```

- [ ] **Step 6: Remove the loading dependency and starter files**

Run from `presentation/`:

```bash
npm uninstall react-loading-skeleton
```

Delete `app/_sites-preview/SkeletonPreview.tsx` and
`app/_sites-preview/preview.css`. Delete the four unused starter SVGs from `public/`;
the finished presentation does not need decorative images or an inherited starter
icon. Change the package name to
`gpolylla-architecture-presentation` and replace `README.md` with:

````markdown
# GPolylla architecture presentation

Autonomous web presentation of the approved 3D-GPolylla redesign.

```bash
npm run dev
npm test
```
````

- [ ] **Step 7: Add a minimal temporary stylesheet and run the contract**

Replace `presentation/app/globals.css` with:

```css
@import "tailwindcss";

:root { color-scheme: light; }
* { box-sizing: border-box; }
html { scroll-behavior: smooth; }
body { margin: 0; font-family: Arial, Helvetica, sans-serif; }
.skip-link { position: absolute; left: 1rem; top: -5rem; }
.skip-link:focus { top: 1rem; }
nav, main { max-width: 72rem; margin: 0 auto; padding: 1rem; }
section { min-height: 70vh; padding: 4rem 0; }
:focus-visible { outline: 3px solid #a8502a; outline-offset: 3px; }
@media (max-width: 760px) { section { min-height: auto; } }
@media (prefers-reduced-motion: reduce) { html { scroll-behavior: auto; } }
```

Run:

```bash
npm test
```

Expected: PASS with two tests and a successful production build.

- [ ] **Step 8: Commit the semantic shell**

```bash
git add presentation
git commit -m "feat(presentation): add autonomous architecture narrative"
```

---

### Task 2: Add purpose-built illustrations for all chapters

**Files:**
- Create: `presentation/app/section-visual.tsx`
- Modify: `presentation/app/page.tsx`
- Modify: `presentation/tests/rendered-html.test.mjs`

**Interfaces:**
- Consumes: `VisualKind` from `presentation-data.ts`.
- Produces: `SectionVisual({ kind }: { kind: VisualKind })`, with one semantic HTML
  composition per chapter and `data-visual` markers for verification.

- [ ] **Step 1: Extend the test with the visual contract**

Add inside the first rendered-page test, immediately after the section count:

```javascript
assert.equal((html.match(/data-visual=/g) ?? []).length, 17);
for (const visual of [
  "north-star", "before-after", "priorities", "architecture", "axes",
  "memory", "eigen", "pipeline", "extensibility", "backends", "adapters",
  "executables", "build", "testing", "style", "migration", "acceptance",
]) {
  assert.match(html, new RegExp(`data-visual="${visual}"`));
}
```

- [ ] **Step 2: Run the test and verify the missing visuals fail**

Run: `npm test`

Expected: FAIL because no `data-visual` attributes exist.

- [ ] **Step 3: Implement the visual component**

Create `presentation/app/section-visual.tsx` with a `SectionVisual` switch over all
17 `VisualKind` values. Use these exact visual payloads:

```tsx
import type { VisualKind } from "./presentation-data";

const data: Record<VisualKind, readonly string[]> = {
  "north-star": ["Face", "Edge", "Cavity", "Algoritmos futuros"],
  "before-after": ["Estado mutable", "Dependencias filtradas", "Aplicaciones monolíticas", "Fronteras explícitas"],
  priorities: ["Corrección", "Extensibilidad", "Control", "Portabilidad", "Reproducibilidad"],
  architecture: ["API pequeña", "Pipeline tipado", "Workspace privado", "Catálogo runtime"],
  axes: ["model", "statistics", "algorithms"],
  memory: ["x0 y0 z0", "x1 y1 z1", "x2 y2 z2", "topología separada"],
  eigen: ["Buffers GPolylla", "Eigen::Map privado", "Operación matemática", "Resultado controlado"],
  pipeline: ["Prepare", "Label", "Construct", "Repair", "Materialize"],
  extensibility: ["Formato → adapter", "Criterio → policy", "Algoritmo → spec", "Backend → workspace"],
  backends: ["CPU secuencial", "CPU paralelo", "CUDA opcional"],
  adapters: ["TetGen / VisF / OFF", "TetrahedralMesh", "statistics::core", "QuickHull opcional"],
  executables: ["CLI", "Experiments", "Demo"],
  build: ["gpolylla::model", "gpolylla::statistics", "gpolylla::algorithms", "targets opcionales"],
  testing: ["Casos conocidos", "Contratos", "Invariantes", "Metamórficas", "Experimentos"],
  style: ["format", "clang-tidy", "warnings", "tests", "architecture"],
  migration: ["Caracterizar", "Modelo", "Cavity vertical", "Familia", "CPU paralelo", "Apps", "CUDA", "Cutover"],
  acceptance: ["Cambio localizado", "Resultados equivalentes", "Build portable", "Experimentos reproducibles"],
};

export function SectionVisual({ kind }: { kind: VisualKind }) {
  const items = data[kind];
  return (
    <div className={`visual visual--${kind}`} data-visual={kind} aria-label={`Diagrama: ${kind}`}>
      {items.map((item, index) => (
        <div className="visual__item" key={item}>
          <span className="visual__index" aria-hidden="true">{String(index + 1).padStart(2, "0")}</span>
          <strong>{item}</strong>
        </div>
      ))}
    </div>
  );
}
```

This deliberately uses a small shared vocabulary. Distinct layout meaning is added
through the per-kind CSS classes, not through extra component state.

- [ ] **Step 4: Render the visual after each thesis**

Import `SectionVisual` in `page.tsx` and insert:

```tsx
<SectionVisual kind={section.visual} />
```

between the thesis paragraph and bullet list.

- [ ] **Step 5: Run the contract and commit**

Run: `npm test`

Expected: PASS.

```bash
git add presentation/app/section-visual.tsx presentation/app/page.tsx presentation/tests/rendered-html.test.mjs
git commit -m "feat(presentation): illustrate architecture decisions"
```

---

### Task 3: Add reading progress and responsive chapter navigation

**Files:**
- Create: `presentation/app/reading-navigation.tsx`
- Modify: `presentation/app/page.tsx`
- Modify: `presentation/tests/rendered-html.test.mjs`

**Interfaces:**
- Consumes: `{ id: string; index: number; label: string }[]` derived from `sections`.
- Produces: `ReadingNavigation`, `data-active-section`, `aria-current`, and a progress
  bar whose value is clamped from 0 to 100.

- [ ] **Step 1: Add navigation assertions**

Add to the first test:

```javascript
assert.match(html, /href="#content"[^>]*>Saltar al contenido</i);
assert.match(html, /aria-label="Navegación de lectura"/i);
assert.match(html, /aria-label="Progreso de lectura"/i);
assert.match(html, /href="#north-star"/i);
assert.match(html, /href="#success-risks-sources"/i);
```

- [ ] **Step 2: Run the test and verify the product navigation is absent**

Run: `npm test`

Expected: FAIL on “Navegación de lectura” or “Progreso de lectura”.

- [ ] **Step 3: Implement the client component**

Create `presentation/app/reading-navigation.tsx`:

```tsx
"use client";

import { useEffect, useState } from "react";

interface NavigationItem {
  id: string;
  index: number;
  label: string;
}

export function ReadingNavigation({ items }: { items: readonly NavigationItem[] }) {
  const [activeId, setActiveId] = useState(items[0]?.id ?? "");
  const [progress, setProgress] = useState(0);

  useEffect(() => {
    const updateProgress = () => {
      const scrollable = document.documentElement.scrollHeight - window.innerHeight;
      const next = scrollable <= 0 ? 100 : (window.scrollY / scrollable) * 100;
      setProgress(Math.min(100, Math.max(0, next)));
    };

    const observer = new IntersectionObserver(
      (entries) => {
        const visible = entries
          .filter((entry) => entry.isIntersecting)
          .sort((left, right) => right.intersectionRatio - left.intersectionRatio)[0];
        if (visible?.target.id) setActiveId(visible.target.id);
      },
      { rootMargin: "-20% 0px -60%", threshold: [0.1, 0.35, 0.65] },
    );

    for (const item of items) {
      const section = document.getElementById(item.id);
      if (section) observer.observe(section);
    }

    updateProgress();
    window.addEventListener("scroll", updateProgress, { passive: true });
    window.addEventListener("resize", updateProgress);
    return () => {
      observer.disconnect();
      window.removeEventListener("scroll", updateProgress);
      window.removeEventListener("resize", updateProgress);
    };
  }, [items]);

  const active = items.find((item) => item.id === activeId) ?? items[0];

  return (
    <>
      <div className="reading-progress" aria-label="Progreso de lectura">
        <span style={{ width: `${progress}%` }} />
      </div>
      <aside className="toc" aria-label="Navegación de lectura">
        <p className="toc__title">Rediseño 3D-GPolylla</p>
        <ol>
          {items.map((item) => (
            <li key={item.id}>
              <a href={`#${item.id}`} aria-current={activeId === item.id ? "location" : undefined}>
                <span>{String(item.index).padStart(2, "0")}</span>{item.label}
              </a>
            </li>
          ))}
        </ol>
      </aside>
      <details className="mobile-nav">
        <summary>{String(active?.index ?? 1).padStart(2, "0")} · {active?.label}</summary>
        <nav aria-label="Capítulos">
          {items.map((item) => <a href={`#${item.id}`} key={item.id}>{item.label}</a>)}
        </nav>
      </details>
    </>
  );
}
```

- [ ] **Step 4: Replace the static nav in `page.tsx`**

Import `ReadingNavigation` and render:

```tsx
<ReadingNavigation
  items={sections.map(({ id, index, label }) => ({ id, index, label }))}
/>
```

immediately after the skip link. Remove the original static `<nav>` block.

- [ ] **Step 5: Run tests and commit**

Run: `npm test`

Expected: PASS.

```bash
git add presentation/app/reading-navigation.tsx presentation/app/page.tsx presentation/tests/rendered-html.test.mjs
git commit -m "feat(presentation): add guided reading navigation"
```

---

### Task 4: Apply the complete visual system and responsive diagrams

**Files:**
- Modify: `presentation/app/globals.css`
- Modify: `presentation/app/page.tsx`
- Modify: `presentation/tests/rendered-html.test.mjs`

**Interfaces:**
- Consumes: semantic class names from `page.tsx`, `ReadingNavigation`, and
  `SectionVisual`.
- Produces: the approved ivory/petroleum/copper/teal theme, desktop sidebar, mobile
  chapter bar, chapter rhythm, per-diagram layouts, print-safe content, and reduced
  motion behavior.

- [ ] **Step 1: Strengthen the CSS source test**

Add these assertions to the CSS test:

```javascript
assert.match(css, /--paper:\s*#f4f0e8/i);
assert.match(css, /--ink:\s*#102a2e/i);
assert.match(css, /--copper:\s*#a8502a/i);
assert.match(css, /--teal:\s*#176b68/i);
assert.match(css, /\.visual--pipeline/);
assert.match(css, /\.visual--memory/);
assert.match(css, /\.visual--migration/);
assert.match(css, /\.mobile-nav/);
assert.match(css, /\.toc/);
```

- [ ] **Step 2: Run tests and verify the temporary CSS fails**

Run: `npm test`

Expected: FAIL on the missing theme variables or diagram selectors.

- [ ] **Step 3: Replace `globals.css` with the complete stylesheet**

Replace `presentation/app/globals.css` with:

```css
@import "tailwindcss";

:root {
  color-scheme: light;
  --paper: #f4f0e8;
  --paper-raised: #fffdf8;
  --ink: #102a2e;
  --muted: #587074;
  --line: #cad2cc;
  --copper: #a8502a;
  --copper-soft: #f0d8c8;
  --teal: #176b68;
  --teal-soft: #d4e8e2;
  --sans: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
  --mono: "SFMono-Regular", Consolas, "Liberation Mono", monospace;
}

* { box-sizing: border-box; }
html { scroll-behavior: smooth; background: var(--paper); }
body { margin: 0; background: var(--paper); color: var(--ink); font-family: var(--sans); }
a { color: inherit; text-underline-offset: 0.2em; }

.skip-link {
  position: fixed;
  z-index: 100;
  top: -5rem;
  left: 1rem;
  padding: 0.75rem 1rem;
  background: var(--ink);
  color: var(--paper-raised);
}
.skip-link:focus { top: 1rem; }
:focus-visible { outline: 3px solid var(--copper); outline-offset: 4px; }

.reading-progress {
  position: fixed;
  z-index: 90;
  inset: 0 0 auto;
  height: 4px;
  background: var(--line);
}
.reading-progress span { display: block; height: 100%; background: var(--copper); }

.toc {
  position: fixed;
  z-index: 20;
  top: 2.5rem;
  left: max(1.25rem, calc((100vw - 96rem) / 2));
  width: 15rem;
  max-height: calc(100vh - 5rem);
  overflow: auto;
  padding: 1rem;
  border-left: 1px solid var(--line);
}
.toc__title { margin: 0 0 1rem; color: var(--copper); font-weight: 750; }
.toc ol { display: grid; gap: 0.2rem; margin: 0; padding: 0; list-style: none; }
.toc a {
  display: grid;
  grid-template-columns: 2rem 1fr;
  gap: 0.4rem;
  padding: 0.38rem 0.5rem;
  color: var(--muted);
  font-size: 0.82rem;
  text-decoration: none;
  border-radius: 0.25rem;
}
.toc a span { font-family: var(--mono); color: var(--copper); }
.toc a[aria-current="location"] { color: var(--ink); background: var(--teal-soft); }
.mobile-nav { display: none; }

.presentation { width: min(100% - 2rem, 96rem); margin: 0 auto; padding-left: 18rem; }
.chapter {
  min-height: 78vh;
  max-width: 72rem;
  margin-left: auto;
  padding: clamp(4rem, 8vw, 8rem) 0;
  border-bottom: 1px solid var(--line);
  scroll-margin-top: 2rem;
}
.chapter--hero { min-height: 92vh; display: grid; align-content: center; }
.chapter__number {
  margin: 0 0 0.8rem;
  color: var(--copper);
  font-family: var(--mono);
  font-size: 0.82rem;
  font-weight: 700;
  letter-spacing: 0.12em;
  text-transform: uppercase;
}
.chapter h1,
.chapter h2 {
  max-width: 18ch;
  margin: 0;
  font-size: clamp(2.25rem, 5vw, 5.4rem);
  line-height: 0.98;
  letter-spacing: -0.045em;
}
.chapter h2 { font-size: clamp(2rem, 4.2vw, 4.4rem); }
.chapter__thesis { max-width: 62ch; margin: 1.5rem 0 2.25rem; color: var(--muted); font-size: clamp(1.15rem, 2vw, 1.55rem); line-height: 1.5; }
.chapter__points { display: grid; gap: 0.75rem; max-width: 70ch; margin: 2rem 0 0; padding-left: 1.2rem; line-height: 1.65; }
.chapter__points li::marker { color: var(--copper); }

.visual {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(10rem, 1fr));
  gap: 0.8rem;
  margin: 2rem 0;
}
.visual__item {
  position: relative;
  min-height: 7rem;
  display: grid;
  align-content: space-between;
  gap: 1rem;
  padding: 1rem;
  border: 1px solid var(--line);
  background: var(--paper-raised);
}
.visual__index { color: var(--copper); font-family: var(--mono); font-size: 0.72rem; }
.visual__item strong { line-height: 1.25; }

.visual--north-star .visual__item,
.visual--axes .visual__item { border-top: 4px solid var(--teal); background: var(--teal-soft); }
.visual--before-after .visual__item { border-top: 4px solid var(--copper); background: var(--copper-soft); }
.visual--before-after .visual__item:last-child { border-top-color: var(--teal); background: var(--teal-soft); }
.visual--priorities { display: flex; flex-direction: column; align-items: flex-start; }
.visual--priorities .visual__item { width: calc(100% - var(--shrink, 0%)); min-height: auto; grid-template-columns: 2.5rem 1fr; align-items: center; background: var(--paper-raised); }
.visual--priorities .visual__item:nth-child(2) { --shrink: 7%; }
.visual--priorities .visual__item:nth-child(3) { --shrink: 14%; }
.visual--priorities .visual__item:nth-child(4) { --shrink: 21%; }
.visual--priorities .visual__item:nth-child(5) { --shrink: 28%; }
.visual--memory { grid-template-columns: repeat(4, 1fr); font-family: var(--mono); }
.visual--memory .visual__item { min-height: 5rem; background: var(--teal-soft); }
.visual--eigen .visual__item:nth-child(2),
.visual--eigen .visual__item:nth-child(3) { border-color: var(--copper); }
.visual--pipeline { grid-template-columns: repeat(5, 1fr); }
.visual--pipeline .visual__item { min-height: 6rem; border-top: 4px solid var(--teal); }
.visual--pipeline .visual__item:first-child,
.visual--pipeline .visual__item:last-child { border-top-color: var(--muted); }
.visual--extensibility { grid-template-columns: repeat(2, 1fr); }
.visual--backends { grid-template-columns: repeat(3, 1fr); }
.visual--backends .visual__item { border-left: 5px solid var(--teal); }
.visual--backends .visual__item:last-child { border-left-color: var(--copper); }
.visual--executables { grid-template-columns: repeat(3, 1fr); }
.visual--testing { grid-template-columns: 1fr; }
.visual--testing .visual__item { min-height: auto; grid-template-columns: 3rem 1fr; align-items: center; }
.visual--style { font-family: var(--mono); }
.visual--migration { grid-template-columns: repeat(4, 1fr); counter-reset: migration; }
.visual--migration .visual__item { min-height: 6rem; border-top: 4px solid var(--copper); }
.visual--acceptance .visual__item { border-left: 5px solid var(--teal); }

details { max-width: 70ch; margin-top: 2rem; padding: 1rem; border: 1px solid var(--line); background: var(--paper-raised); }
summary { cursor: pointer; color: var(--teal); font-weight: 700; }
details p { color: var(--muted); line-height: 1.65; }

.presentation-footer { max-width: 72rem; margin-left: auto; padding: 5rem 0 8rem; }
.presentation-footer p { max-width: 60ch; color: var(--muted); line-height: 1.6; }
.source-link { display: inline-block; margin-top: 1rem; padding: 0.9rem 1.2rem; background: var(--ink); color: var(--paper-raised); font-weight: 700; text-decoration: none; }

@media (max-width: 1100px) {
  .toc { display: none; }
  .presentation { width: min(100% - 2rem, 72rem); padding-left: 0; }
  .mobile-nav { position: sticky; z-index: 30; top: 4px; display: block; padding: 0.75rem 1rem; border-bottom: 1px solid var(--line); background: var(--paper-raised); }
  .mobile-nav summary { list-style: none; }
  .mobile-nav nav { display: grid; grid-template-columns: repeat(2, 1fr); gap: 0.4rem; padding-top: 0.8rem; }
  .mobile-nav a { padding: 0.35rem; color: var(--muted); font-size: 0.85rem; }
}

@media (max-width: 760px) {
  .chapter { min-height: auto; padding: 4.5rem 0; }
  .chapter--hero { min-height: 82vh; }
  .chapter h1,
  .chapter h2 { max-width: none; }
  .visual,
  .visual--memory,
  .visual--pipeline,
  .visual--extensibility,
  .visual--backends,
  .visual--executables,
  .visual--migration { grid-template-columns: 1fr; }
  .visual--priorities .visual__item { width: 100%; }
  .mobile-nav nav { grid-template-columns: 1fr; max-height: 65vh; overflow: auto; }
}

@media print {
  .toc,
  .mobile-nav,
  .reading-progress,
  .skip-link { display: none; }
  .presentation { width: 100%; padding: 0; }
  .chapter { min-height: 0; break-inside: avoid; padding: 2rem 0; }
}

@media (prefers-reduced-motion: reduce) {
  html { scroll-behavior: auto; }
  *, *::before, *::after { transition-duration: 0.01ms !important; }
}
```

- [ ] **Step 4: Replace the page with the final semantic composition**

Replace `presentation/app/page.tsx` with:

```tsx
import { sections } from "./presentation-data";
import { ReadingNavigation } from "./reading-navigation";
import { SectionVisual } from "./section-visual";

const sourceDocument =
  "https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md";

export default function Home() {
  const navigationItems = sections.map(({ id, index, label }) => ({ id, index, label }));

  return (
    <>
      <a className="skip-link" href="#content">Saltar al contenido</a>
      <ReadingNavigation items={navigationItems} />
      <main className="presentation" id="content">
        {sections.map((section) => (
          <section
            aria-labelledby={`${section.id}-title`}
            className={`chapter${section.index === 1 ? " chapter--hero" : ""}`}
            data-presentation-section
            id={section.id}
            key={section.id}
          >
            <header>
              <p className="chapter__number">Capítulo {String(section.index).padStart(2, "0")} · {section.label}</p>
              {section.index === 1 ? (
                <h1 id={`${section.id}-title`}>{section.title}</h1>
              ) : (
                <h2 id={`${section.id}-title`}>{section.title}</h2>
              )}
              <p className="chapter__thesis">{section.thesis}</p>
            </header>
            <SectionVisual kind={section.visual} />
            <ul className="chapter__points">
              {section.bullets.map((bullet) => <li key={bullet}>{bullet}</li>)}
            </ul>
            {section.detail ? (
              <details><summary>Profundizar</summary><p>{section.detail}</p></details>
            ) : null}
          </section>
        ))}
        <footer className="presentation-footer">
          <h2>Documento técnico</h2>
          <p>La presentación resume las decisiones. El documento versionado conserva el razonamiento, los riesgos y los criterios completos.</p>
          <a className="source-link" href={sourceDocument} rel="noreferrer" target="_blank">
            Leer el documento completo
          </a>
        </footer>
      </main>
    </>
  );
}
```

- [ ] **Step 5: Run build, tests, and lint**

Run:

```bash
npm test
npm run lint
```

Expected: production build and tests PASS; ESLint exits 0.

- [ ] **Step 6: Commit the finished visual system**

```bash
git add presentation/app/globals.css presentation/app/page.tsx presentation/tests/rendered-html.test.mjs
git commit -m "feat(presentation): add responsive technical visual system"
```

---

### Task 5: Final content audit, production build, and Sites deployment

**Files:**
- Verify: `presentation/app/presentation-data.ts`
- Verify: `presentation/app/layout.tsx`
- Verify: `presentation/app/page.tsx`

**Interfaces:**
- Consumes: all prior tasks and the approved redesign document.
- Produces: a validated production artifact and the deployed Sites URL.

- [ ] **Step 1: Audit coverage against the source headings**

Run:

```bash
rg -n '^## |^### ' docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md
rg -n 'id:|title:|thesis:' presentation/app/presentation-data.ts
```

Expected: each major topic from north star through sources maps to one of the 17
chapters. Confirm explicitly that model, Eigen, pipeline, extensibility, CUDA,
statistics, formats, executables, portability, style, tests, experiments, migration,
acceptance, risks, alternatives, and bibliography are represented.

- [ ] **Step 2: Scan for starter or incomplete copy**

Run:

```bash
rg -n 'taking shape|Starter Project|codex-preview|SkeletonPreview|lorem|TO[D]O|TB[D]' presentation --glob '!node_modules/**' --glob '!dist/**'
```

Expected: no matches.

- [ ] **Step 3: Run the final production verification**

Run from `presentation/`:

```bash
npm run build
npm test
npm run lint
```

Expected: all commands exit 0.

- [ ] **Step 4: Inspect repository scope before deployment**

Run from the repository root:

```bash
git status --short
git diff --check
```

Expected: only intended presentation changes are present; the user's untracked
`AGENTS.md` remains untouched.

- [ ] **Step 5: Deploy using the `sites-hosting` skill**

Read the hosting skill, publish the validated `presentation/` site, and capture the
returned URL. Do not add D1, R2, authentication, environment variables, or custom
domains.

Expected: a working deployed URL and a local production build that matches the
deployed source. Return the URL in the implementation handoff; do not hard-code a
deployment-specific address into the repository.
