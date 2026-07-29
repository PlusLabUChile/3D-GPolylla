import type { VisualKind } from "./presentation-data";

const visualItems: Record<VisualKind, readonly string[]> = {
  "north-star": ["Face", "Edge", "Cavity", "Algoritmos futuros"],
  "before-after": [
    "Estado mutable",
    "Dependencias filtradas",
    "Aplicaciones monolíticas",
    "Fronteras explícitas",
  ],
  priorities: [
    "Corrección",
    "Extensibilidad",
    "Control",
    "Portabilidad",
    "Reproducibilidad",
  ],
  architecture: [
    "API pequeña",
    "Pipeline tipado",
    "Workspace privado",
    "Catálogo runtime",
  ],
  axes: ["model", "statistics", "algorithms"],
  memory: ["x0 y0 z0", "x1 y1 z1", "x2 y2 z2", "topología separada"],
  eigen: [
    "Buffers GPolylla",
    "Eigen::Map privado",
    "Operación matemática",
    "Resultado controlado",
  ],
  pipeline: ["Prepare", "Label", "Construct", "Repair", "Materialize"],
  extensibility: [
    "Formato → adapter",
    "Criterio → policy",
    "Algoritmo → spec",
    "Backend → workspace",
  ],
  backends: ["CPU secuencial", "CPU paralelo", "CUDA opcional"],
  adapters: [
    "TetGen / VisF / OFF",
    "TetrahedralMesh",
    "statistics::core",
    "QuickHull opcional",
  ],
  executables: ["CLI", "Experiments", "Demo"],
  build: [
    "gpolylla::model",
    "gpolylla::statistics",
    "gpolylla::algorithms",
    "targets opcionales",
  ],
  testing: [
    "Casos conocidos",
    "Contratos",
    "Invariantes",
    "Metamórficas",
    "Experimentos",
  ],
  style: ["format", "clang-tidy", "warnings", "tests", "architecture"],
  migration: [
    "Caracterizar",
    "Modelo",
    "Cavity vertical",
    "Familia",
    "CPU paralelo",
    "Apps",
    "CUDA",
    "Cutover",
  ],
  acceptance: [
    "Cambio localizado",
    "Resultados equivalentes",
    "Build portable",
    "Experimentos reproducibles",
  ],
};

const visualLabels: Record<VisualKind, string> = {
  "north-star": "Familia de algoritmos",
  "before-after": "Problemas actuales y frontera propuesta",
  priorities: "Prioridad de atributos arquitectónicos",
  architecture: "Capas de especialización",
  axes: "Ejes de la biblioteca",
  memory: "Layout canónico de memoria",
  eigen: "Frontera privada de Eigen",
  pipeline: "Pipeline algorítmico",
  extensibility: "Puntos de extensión",
  backends: "Backends compatibles",
  adapters: "Adaptadores y modelo",
  executables: "Ejecutables separados",
  build: "Targets CMake",
  testing: "Capas de verificación",
  style: "Target de comprobación local",
  migration: "Etapas de migración",
  acceptance: "Criterios de aceptación",
};

export function SectionVisual({ kind }: { kind: VisualKind }) {
  return (
    <div
      aria-label={visualLabels[kind]}
      className={`visual visual--${kind}`}
      data-visual={kind}
      role="img"
    >
      {visualItems[kind].map((item, index) => (
        <div className="visual__item" key={item}>
          <span aria-hidden="true" className="visual__index">
            {String(index + 1).padStart(2, "0")}
          </span>
          <strong>{item}</strong>
        </div>
      ))}
    </div>
  );
}
