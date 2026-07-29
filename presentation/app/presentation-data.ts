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
