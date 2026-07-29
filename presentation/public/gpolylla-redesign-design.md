# Rediseño arquitectónico de 3D-GPolylla

**Fecha:** 2026-07-28

**Estado:** borrador consolidado para revisión final

**Alcance:** rediseño completo de la biblioteca, aplicaciones, dependencias, pruebas y estructura de desarrollo

## 1. North star

3D-GPolylla debe convertirse en la consolidación de una familia de algoritmos de
generación de poliedros. El repositorio no se organiza alrededor de una única
implementación de Cavity ni alrededor de la visualización, sino alrededor de tres
capacidades permanentes:

1. Representar mallas y sus invariantes.
2. Construir poliedros mediante una familia extensible de algoritmos.
3. Medir y analizar los resultados.

La arquitectura debe permitir incorporar nuevos formatos, criterios de unión,
representaciones internas, algoritmos y backends sin modificar componentes que no
participan de esa variación.

## 2. Contexto observado

El código actual demuestra la viabilidad del dominio, pero concentra demasiadas
responsabilidades:

- `include/gpolylla/polylla.h` expone datos mutables y mezcla geometría, topología y
  comportamiento.
- `Vertex` hereda de un tipo Eigen, acoplando identidad de dominio y representación
  matemática.
- `CavityAlgorithm` conserva resultados y estados intermedios dentro del objeto, lo
  que dificulta reentrancia y paralelismo.
- Las fases `labelCavities`, `buildCavities` y `fixCavities` ya insinúan el pipeline
  casi invariante, pero no existen como contratos explícitos.
- El recorrido recursivo y el ownership compartido de Cavity no constituyen una
  base adecuada para paralelismo masivo.
- `src/renderer/main.cpp` concentra aplicación, OpenGL, cámara, picking, UI,
  estadísticas y ejecución de algoritmos.
- El CLI mezcla ejecución normal, generación de CSV y código de experimentación.
- `src/stat.cpp` conoce dependencias externas e incluso una ruta del directorio de
  build.
- El target central propaga dependencias que deberían ser privadas u opcionales.
- Existen dependencias fijadas a ramas móviles y presets específicos de una sola
  plataforma.
- Las pruebas actuales son pequeñas y algunas reflejan APIs anteriores.

Las carpetas `old/` y `old2/`, junto con el documento de tesis, muestran que Face,
Edge, Cavity y los criterios de unión son miembros de una misma familia. Se usarán
como evidencia histórica, no como fuente que deba copiarse sin revisión.

## 3. Objetivos y no objetivos

### 3.1 Objetivos

- Compatibilidad CPU con Linux, Windows y macOS.
- API nueva; no se requiere compatibilidad con la API actual.
- Biblioteca utilizable sin compilar CLI, demo ni experiments.
- Ejes públicos claros: `model`, `statistics` y `algorithms`.
- Pipeline semántico `Label -> Construct -> Repair`.
- Variación independiente por formato, algoritmo, criterio y representación.
- CPU secuencial como referencia de corrección.
- Paralelismo CPU sin estado global ni mutación compartida implícita.
- Frontera preparada para un backend CUDA opcional.
- Control explícito sobre ownership, layout, asignaciones y scheduling.
- Dependencias externas pequeñas, justificadas, fijadas y aisladas.
- Pruebas unitarias, contractuales, diferenciales y de robustez reproducible.
- Demo útil para visualización y manual testing, pero ajena al núcleo algorítmico.
- Guía de estilo propia y verificable con un único comando.

### 3.2 No objetivos iniciales

- Mantener una capa de compatibilidad con la API anterior.
- Ofrecer plugins binarios cargados dinámicamente.
- Producir desde el inicio una implementación CUDA completa.
- Convertir la demo en una aplicación de producción.
- Implementar CI/CD empresarial, publicación automática o infraestructura de
  benchmarks permanente.
- Preservar código comentado o alternativas históricas dentro del build activo.

## 4. Prioridad de atributos arquitectónicos

Los atributos se priorizan en este orden:

1. **Corrección algorítmica:** invariantes y resultados reproducibles.
2. **Extensibilidad:** variaciones localizadas y dependencias acíclicas.
3. **Control de rendimiento:** layouts, asignaciones y scheduling explícitos.
4. **Portabilidad:** semántica CPU común para los tres sistemas objetivo.
5. **Reproducibilidad científica:** inputs, configuración, seeds y resultados
   trazables.
6. **Usabilidad:** API de dominio clara y aplicaciones sencillas.

Una optimización no puede debilitar silenciosamente las invariantes. Una abstracción
no se incorpora si oculta costos esenciales o fuerza un layout universal.

## 5. Arquitectura seleccionada

Se adopta un microkernel lógico con pipelines tipados. Las fronteras públicas son
pequeñas y estables; las combinaciones de alto rendimiento se resuelven mediante
tipos y workspaces privados.

```text
                         gpolylla::gpolylla
                       fachada/target paraguas
                                  |
          +-----------------------+-----------------------+
          |                       |                       |
       model                 statistics              algorithms
          ^                       ^                       |
          |                       |                       |
    adapters/io          statistics/adapters       CPU / CUDA
          ^                                               |
          +-------------------- applications -------------+
                           CLI / demo / experiments
```

No existe un eje público llamado `execution`. CPU secuencial, CPU paralelo y CUDA
son implementaciones internas del módulo `algorithms`, seleccionadas al comienzo de
una operación.

### 5.1 Regla de dependencias

```text
model
  ^
statistics::core
  ^ opcional, sólo para métricas geométricas básicas reutilizables
algorithms

adapters/io ----------> model
statistics/adapters --> statistics::core
applications ---------> fachada y adaptadores públicos
```

Reglas obligatorias:

- `model` no depende de algoritmos, estadísticas, formatos, Eigen, CUDA o UI.
- `statistics` depende de `model`, nunca de `algorithms`.
- `algorithms` depende de `model` y, cuando sea necesario, de métricas pequeñas de
  `statistics::core`; nunca de adaptadores opcionales.
- La biblioteca nunca depende de los ejecutables.
- Ningún target público incluye headers provenientes de un directorio de build.
- El grafo de targets debe permanecer acíclico.

## 6. Modelo y control de memoria

### 6.1 Tipos principales

- `TetrahedralMesh`: entrada canónica validada.
- `PolyhedralMesh`: salida independiente de cualquier algoritmo concreto.
- `MeshBuilder`: única frontera general de construcción mutable.
- `MeshValidator`: validación explícita con un reporte tipado.
- `VertexId`, `TetrahedronId`, `FaceId` y `PolyhedronId`: IDs fuertes y no
  intercambiables.
- Vistas no poseedoras para acceso eficiente a posiciones, incidencias y rangos.

### 6.2 Ownership y layout

GPolylla es propietario del almacenamiento. La geometría, la topología y los
atributos viven en buffers separados y contiguos. Un layout canónico inicial para
las posiciones es:

```text
x0 y0 z0 | x1 y1 z1 | x2 y2 z2 | ...
```

La conectividad se almacena separadamente mediante IDs o sus representaciones
subyacentes. La implementación debe verificar en compilación las propiedades de
layout y trivialidad de los elementos destinados a copias masivas.

El modelo validado se trata como inmutable durante una ejecución. Cada llamada a
un algoritmo crea o recibe un workspace exclusivo. No existen punteros dueños
expuestos ni grafos de objetos con ownership implícito.

### 6.3 Política escalar

El vocabulario numérico define un único `Scalar` canónico. La primera implementación
usará `double` para favorecer robustez geométrica. Una conversión a otra precisión
para un backend será explícita, registrada en `RunInfo` y validada por pruebas de
equivalencia. No se templará toda la API pública por precisión hasta que exista un
caso experimental que justifique ese costo.

### 6.4 Eigen

Eigen no forma parte del modelo ni de la API pública. Se usa exclusivamente como
dependencia privada para operaciones matemáticas avanzadas dentro de `algorithms`
y `statistics`.

- Ningún tipo Eigen cruza una frontera pública.
- El estado del dominio no se almacena en `Eigen::MatrixX*`.
- Adaptadores privados pueden construir `Eigen::Map` sobre buffers controlados por
  GPolylla.
- Eigen no decide ownership, asignaciones del pipeline ni scheduling.
- Los targets se compilan con el paralelismo interno de Eigen deshabilitado; el
  paralelismo exterior pertenece a GPolylla.
- Los kernels CUDA no dependen de Eigen.

La regla resumida es: **Eigen calcula; GPolylla controla memoria, flujo y
concurrencia.**

## 7. Módulo de algoritmos

### 7.1 Contrato observable

Una ejecución recibe un mesh validado, una configuración explícita y un criterio.
Devuelve un valor nuevo; el objeto algoritmo no retiene estado entre llamadas. Cada
algoritmo incorporado expone un entry point propio y tipado; no existe un request
universal que deba interpretar todas las configuraciones posibles.

```cpp
template<UnionCriterion Criterion>
Result<AlgorithmResult, AlgorithmError> runCavity(
    const TetrahedralMesh& mesh,
    const CavityOptions& options,
    Criterion criterion);
```

Face, Edge y algoritmos futuros siguen la misma forma general, pero conservan
options y estados de dominio propios. `Result` es un pequeño tipo de vocabulario
propiedad del proyecto, compatible con C++20; no introduce otra dependencia y no se
usa dentro del loop por elemento.

`AlgorithmResult` contiene:

- `PolyhedralMesh`.
- `RunInfo` con tiempos por fase, backend, configuración efectiva y warnings.
- Snapshots de fase opcionales cuando se habilitan diagnósticos.

La biblioteca no escribe directamente en consola. Los errores esperables se
representan mediante resultados tipados; las excepciones no se usan como control
de flujo normal.

### 7.2 Pipeline casi invariante

```text
TetrahedralMesh
      |
      v
prepare<AlgorithmSpec, Backend>
      |
      v
private Workspace
      |
      +--> Label
      +--> Construct
      +--> Repair
      |
      v
materialize
      |
      v
AlgorithmResult
```

`prepare` y `materialize` son fronteras técnicas, no fases semánticas adicionales.

#### Label

Calcula información local, fitness, candidatos y etiquetas iniciales. Debe
favorecer trabajo independiente por tetraedro, cara o arista.

#### Construct

Construye ownership, regiones y fronteras. Puede usar frontiers, union-find,
pointer jumping u otra estrategia apropiada para el algoritmo y el backend. No se
prescribe un DFS recursivo común.

#### Repair

Detecta defectos, propone correcciones, resuelve conflictos y aplica cambios.
Puede ser iterativa; sus condiciones de terminación y postcondiciones deben ser
explícitas.

### 7.3 Dimensiones de variación

- **Algoritmo:** Cavity, Face, Edge u otro `AlgorithmSpec`.
- **Criterio:** policy tipada, sin llamada virtual dentro del hot path.
- **Workspace:** representación privada adaptada al algoritmo y backend.
- **Backend:** implementación CPU secuencial, CPU paralela o CUDA.
- **Selección de aplicaciones:** catálogo runtime con type erasure sólo en el
  borde; registra únicamente combinaciones compiladas.

No se fuerza una jerarquía universal sobre estados internos que sean esencialmente
distintos. El pipeline comparte semántica y contratos, no necesariamente una única
estructura de datos.

## 8. Paralelismo y frontera CUDA

### 8.1 CPU secuencial

Es la implementación de referencia: pequeña, legible y determinista. No es el único
oráculo; también se valida contra resultados manuales e invariantes.

### 8.2 CPU paralelo

- Conserva la semántica observable.
- Usa layouts y scheduling propios.
- Recibe el número de threads mediante configuración.
- No usa estado global mutable.
- Evita asignaciones después de `prepare` en los loops principales.
- Puede producir un orden interno diferente; la equivalencia se evalúa sobre una
  representación canonicalizada y sus invariantes.

### 8.3 CUDA

CUDA será un target opcional del módulo `algorithms`, no una dependencia del
modelo. Los headers públicos no incluyen CUDA ni exponen memoria de dispositivo.

Una ejecución CUDA:

1. Valida capacidades y configuración.
2. Construye el workspace de dispositivo.
3. Realiza las transferencias iniciales.
4. Mantiene los datos en dispositivo durante las tres fases cuando sea viable.
5. Materializa el resultado una sola vez.

Pedir CUDA de forma obligatoria y no tenerlo disponible produce un error explícito.
El fallback a CPU sólo ocurre si la configuración solicita `Auto`.

La primera migración incluye un spike vertical de CUDA, no una implementación
completa. Su finalidad es demostrar que una fase o un Cavity pequeño atraviesa la
frontera sin modificar la API ni el modelo.

## 9. Estadísticas

`statistics` es un eje independiente y no un conjunto de métodos anexados a
`Mesh`.

`statistics::core` contiene métricas y agregaciones que pueden implementarse sin
dependencias pesadas. Las integraciones con QuickHull o `polyhedron-kernel` viven
en targets opcionales bajo `statistics/adapters`.

Cada cálculo declara:

- Precondiciones geométricas.
- Política de tolerancia.
- Tratamiento de valores inválidos.
- Complejidad o costo relevante.
- Dependencias opcionales necesarias.

Los algoritmos sólo pueden depender de métricas pequeñas de `statistics::core`, no
de los adaptadores.

## 10. Formatos y adaptadores

TetGen, VisF, OFF y formatos futuros se implementan como adaptadores hacia el
modelo canónico. Un reader no conserva una malla mutable oculta después de leer.

```text
archivo/stream -> parser -> datos intermedios -> MeshBuilder -> TetrahedralMesh
```

Cada adaptador posee pruebas de:

- Archivo mínimo válido.
- Errores de sintaxis y referencias inválidas.
- Límites y casos degenerados.
- Equivalencia entre formatos para el mismo dominio.

Agregar un formato no modifica `TetrahedralMesh` ni ningún algoritmo.

## 11. Ejecutables

### 11.1 CLI

Responsabilidades:

- Interpretar argumentos.
- Seleccionar formatos, algoritmo, criterio y backend.
- Invocar casos de uso públicos.
- Escribir resultados y diagnósticos.

No calcula estadísticas manualmente ni contiene campañas experimentales.

### 11.2 Experiments

Ejecuta campañas reproducibles y registra dataset, hash, configuración, seed,
plataforma, compilador, backend, tiempos, memoria, warnings e invariantes. Produce
JSONL o CSV y un comando de reproducción.

No forma parte de CTest ni de la biblioteca instalada.

### 11.3 Demo

Se divide en:

```text
application -> casos de uso y estado de sesión
rendering   -> OpenGL, recursos, cámara y picking
ui          -> ImGui y controles
```

La demo visualiza entradas, estados diagnósticos y resultados. Sirve para manual
testing, pero no contiene lógica de construcción de poliedros.

## 12. Estructura objetivo

```text
3D-GPolylla/
|-- cmake/
|-- include/gpolylla/
|   |-- model/
|   |-- statistics/
|   |-- algorithms/
|   |-- adapters/io/
|   `-- gpolylla.hpp
|-- src/
|   |-- model/
|   |-- statistics/{core,adapters}/
|   |-- algorithms/{common,cavity,face,edge}/
|   `-- adapters/io/
|-- apps/{cli,demo,experiments}/
|-- test/{model,statistics,algorithms,adapters,contract,fixtures}/
|-- data/
`-- docs/
```

Los backends CPU y CUDA viven dentro del algoritmo correspondiente. No aparecen
como un cuarto eje público.

## 13. Build, portabilidad y dependencias

### 13.1 Targets

- `gpolylla::model`
- `gpolylla::statistics`
- `gpolylla::algorithms`
- Targets opcionales de formatos y estadísticas avanzadas.
- `gpolylla::gpolylla` como target paraguas, no como contenedor monolítico.

La instalación exporta targets y un `GPolyllaConfig.cmake`. Un consumidor puede
usar `find_package(GPolylla)` sin compilar aplicaciones.

### 13.2 Gestión de dependencias

- CMake moderno basado en targets.
- `vcpkg.json` con baseline fijado para desarrollo reproducible.
- El consumidor no queda obligado a usar vcpkg; el proyecto usa `find_package`.
- `FetchContent` se limita a un modo de bootstrap para desarrolladores.
- No se permiten referencias a `main`, `master` u otras ramas móviles.
- Eigen es `PRIVATE` de `algorithms` y `statistics`.
- QuickHull y `polyhedron-kernel` son privados y opcionales de estadísticas.
- OpenGL, GLFW, ImGui y GLM pertenecen exclusivamente a la demo.
- CUDA se configura sólo cuando el toolkit y el compilador son compatibles.

Compilar la biblioteca base no descarga ni compila la demo, CUDA o estadísticas
avanzadas.

### 13.3 Portabilidad

- C++20 común para GCC, Clang, AppleClang y MSVC.
- Presets sin rutas absolutas ni selección fija de `cl`.
- Código específico de plataforma encapsulado detrás de targets o archivos
  dedicados.
- CPU es la capacidad común en Linux, Windows y macOS.
- La ausencia de CUDA no degrada ni modifica la API CPU.

## 14. Guía de estilo

El proyecto adopta una guía propia basada en C++ Core Guidelines, Clean Code y
reglas de HPC. El formato se basa en LLVM:

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
BreakBeforeBraces: Attach
PointerAlignment: Left
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: Never
SortIncludes: CaseSensitive
```

### 14.1 Nombres

- Namespaces y archivos: `snake_case`.
- Tipos, conceptos y enums: `PascalCase`.
- Funciones, métodos y variables: `camelCase`.
- Miembros privados: `camelCase_`.
- Constantes: `kPascalCase`.
- Macros: `GPOLYLLA_UPPER_CASE`.
- Tests: `*_test.cpp` y fixtures `PascalCaseTest`.

Código, identificadores y documentación técnica se escriben en inglés.

### 14.2 Reglas de API y C++

- RAII y semántica de valores por defecto.
- Ningún raw pointer expresa ownership.
- `std::span` y `std::string_view` expresan vistas no poseedoras.
- No hay `using namespace` en headers.
- Headers autocontenidos con `#pragma once`.
- Parámetros booleanos ambiguos se sustituyen por enums o configuraciones tipadas.
- `const`, `[[nodiscard]]` y `noexcept` expresan contratos reales.
- No hay estado global mutable, singletons ni logging directo desde la biblioteca.
- Los comentarios explican decisiones, invariantes o riesgos, no traducen el código.
- No se conserva código muerto o comentado.

### 14.3 Reglas numéricas y de hot paths

- No existen epsilons mágicos.
- Conversiones destructivas son explícitas.
- `-ffast-math` está desactivado por defecto.
- No hay asignación dinámica en los loops principales después de `prepare`.
- No hay despacho virtual por elemento.
- Cambios de layout u optimizaciones complejas requieren evidencia o benchmark.

### 14.4 Enforcement

Se incorporan `.editorconfig`, `.clang-format`, `.clang-tidy`, módulos CMake para
warnings y herramientas, y pruebas arquitectónicas.

```bash
cmake --build --preset dev --target check
```

`check` valida formato, clang-tidy, warnings, tests, headers autocontenidos e
includes prohibidos. Warnings como errores se activan en
`GPOLYLLA_DEVELOPER_MODE`, pero nunca se propagan a consumidores o dependencias.

## 15. Estrategia de pruebas

```text
test/
|-- model/
|-- statistics/
|-- algorithms/phases/
|-- adapters/
|-- contract/
|-- metamorphic/
|-- fixtures/
`-- support/
```

### 15.1 Niveles

1. Modelo: IDs, conectividad, builders e invariantes.
2. Estadísticas: sólidos conocidos, tolerancias y degeneración.
3. Fases: micromallas y estados esperados de Label, Construct y Repair.
4. Criterios: suite contractual reutilizable.
5. Algoritmos: Cavity, Face y Edge con casos derivados manualmente.
6. Backends: equivalencia CPU secuencial, CPU paralelo y, cuando exista, CUDA.
7. Formatos: golden tests, entradas inválidas y equivalencia.

Las fases poseen seams de prueba internos; la API normal continúa ejecutando el
pipeline completo.

### 15.2 Invariantes

- Todo tetraedro tiene exactamente un propietario.
- No existen celdas huérfanas, ownership duplicado o índices inválidos.
- Las caras internas se cancelan correctamente.
- La frontera satisface las condiciones topológicas declaradas.
- El volumen se conserva dentro de una tolerancia definida.
- Repair cumple sus postcondiciones y termina dentro de límites explícitos.

### 15.3 Pruebas metamórficas y diferenciales

- Permutación de vértices y tetraedros.
- Traslación y rotación rígida.
- Escalamiento cuando el criterio sea invariante a escala.
- Repetición en modo determinista.
- Comparación de particiones canonicalizadas entre backends.

No se agrega inicialmente otra dependencia para property testing; los generadores
deterministas pueden vivir en `test/support` hasta que su complejidad justifique una
biblioteca dedicada.

## 16. Experimentos de robustez

`gpolylla-experiments` ejecuta, bajo demanda:

- Mallas válidas, degeneradas y casi degeneradas.
- Perturbaciones geométricas con seed.
- Diferentes órdenes de entrada.
- Criterios, parámetros, threads y backends.
- Escalas crecientes y corpus de fallos históricos.

Cada resultado registra commit, compilador, plataforma, backend, dataset hash,
configuración, seed, invariantes, warnings, tiempos y memoria por fase. Los
benchmarks no se convierten en gates rígidos sobre runners compartidos.

## 17. Automatización proporcional

No se implementa CD. La automatización inicial consiste en un workflow pequeño que
compila y ejecuta `check` en Linux. Windows y macOS se verifican manualmente antes de
entregas importantes o mediante un workflow manual si existen runners disponibles.

CUDA, sanitizers, fuzzing y experimentos largos son presets o tareas locales bajo
demanda. La prioridad es reproducibilidad local, no infraestructura empresarial.

## 18. Migración

### Etapa 0: caracterización

Registrar casos representativos, resultados actuales canonicalizados, inputs
inválidos y defectos conocidos.

### Etapa 1: fundación

Crear estructura, targets, build, guía de estilo y test harness nuevos sin reformar
masivamente código que será eliminado.

### Etapa 2: modelo canónico

Implementar IDs, buffers, builders, validación, `TetrahedralMesh` y
`PolyhedralMesh`.

### Etapa 3: primer corte vertical

Completar TetGen -> modelo -> Cavity CPU secuencial -> estadísticas -> CLI. Este
corte valida las fronteras antes de extender la familia.

### Etapa 4: familia de algoritmos

Extraer contratos comunes, criteria policies e implementar Face y Edge sin forzar
sus estados internos a compartir una representación.

### Etapa 5: CPU paralelo

Crear workspaces especializados, eliminar recorridos no paralelizables donde sea
necesario y ejecutar pruebas diferenciales con diferentes cantidades de threads.

### Etapa 6: aplicaciones

Migrar primero CLI, luego experiments y finalmente demo. La demo no dicta la API de
la biblioteca.

### Etapa 7: spike CUDA

Demostrar una ruta de dispositivo de extremo a extremo. Cualquier necesidad de
cambiar el modelo público se considera un defecto de la frontera.

### Etapa 8: cutover

Cambiar la fachada, eliminar la implementación anterior del árbol activo, actualizar
documentación y validar CPU en los tres sistemas.

No se mantiene una capa de compatibilidad de la API anterior.

## 19. Criterios de aceptación

| Atributo | Evidencia requerida |
|---|---|
| Formatos | Agregar un adaptador no modifica modelo ni algoritmos. |
| Criterios | Agregar una policy no modifica fases existentes. |
| Algoritmos | Agregar uno no modifica Cavity, Face o Edge. |
| Layouts | Cada algoritmo/backend posee su workspace privado. |
| Corrección | Resultados manuales, invariantes y comparación diferencial. |
| Paralelismo | CPU secuencial y paralelo producen particiones equivalentes. |
| GPU | El spike CUDA no cambia el modelo o la API observable. |
| Dependencias | La biblioteca base compila sin demo, CUDA o adaptadores avanzados. |
| Aplicaciones | CLI, experiments y demo no contienen lógica algorítmica. |
| Portabilidad | Build CPU validado en Linux, Windows y macOS. |
| Estilo | `check` valida reglas mecánicas y pruebas arquitectónicas. |
| Reproducibilidad | Un experimento se reconstruye desde dataset, configuración y seed. |

## 20. Riesgos y mitigaciones

### Sofisticación excesiva

La experiencia del mantenedor permite una arquitectura avanzada, pero cada
abstracción sigue necesitando una dimensión de variación real. El corte vertical de
Cavity impide construir una taxonomía completa antes de probarla.

### Explosión de combinaciones tipadas

Algoritmos, criterios y backends pueden aumentar compilación y binario. Las
aplicaciones registran sólo combinaciones soportadas; no se instancia el producto
cartesiano completo.

### Costo de transformar layouts

El modelo canónico puede requerir una conversión al workspace. La transformación
se realiza una vez en `prepare` y se amortiza durante las tres fases. Se mide antes
de introducir layouts más complejos.

### Divergencia entre backends

CPU paralelo y CUDA pueden cambiar orden de IDs o reducciones. Se ofrecen invariantes
y canonicalización, más un modo determinista cuando sea necesario.

### Frontera GPU teórica

Una arquitectura puede parecer compatible con GPU sin serlo. El spike CUDA antes
del cutover actúa como prueba de la frontera.

### Ruptura total de API

La falta de compatibilidad simplifica el diseño, pero exige un punto de cambio claro.
La versión actual permanece sólo como referencia hasta completar el corte vertical
y la evidencia de paridad.

## 21. Alternativas descartadas

### Jerarquía orientada a objetos completamente dinámica

Facilita selección runtime, pero introduce estado mutable y despacho virtual en
zonas sensibles. Se conserva type erasure únicamente en el borde de las
aplicaciones.

### Pipeline completamente templado y expuesto

Maximiza especialización, pero filtra detalles, aumenta tiempos de compilación y
convierte la API en una matriz de combinaciones. Se prefieren contratos públicos
pequeños y especialización privada.

### Eigen como representación pública

Ofrece ergonomía inmediata, pero cede control de layout, alineamiento, asignaciones
y ABI. Eigen queda como herramienta matemática privada.

### Un modelo universal óptimo para todos los algoritmos

Oculta diferencias esenciales entre Face, Edge, Cavity, CPU y GPU. Se adopta un
modelo canónico de intercambio y workspaces tailored.

### Plugins binarios desde el inicio

Agregan ABI, discovery y despliegue sin un consumidor que lo exija. La extensión
inicial es a nivel de código y targets compilados.

### CI/CD extensa

No corresponde al presupuesto ni al objetivo académico. Se conserva un check local
reproducible y automatización mínima.

## 22. Fundamento bibliográfico

- **Clean Architecture:** OCP, DIP, cohesión y acoplamiento de componentes,
  límites, plugins, Main y separación de detalles (caps. 8, 11, 13-17, 22, 24, 26,
  28 y 34).
- **Clean Code:** nombres, funciones, estructuras de datos, errores, fronteras,
  tests, clases, sistemas y concurrencia (caps. 2-11 y 13).
- **Fundamentals of Software Architecture:** modularidad, características,
  decisiones, componentes, estilos y trade-offs (caps. 3-8, 11, 12 y 18).
- **Building Evolutionary Architectures:** fitness functions, cambio incremental,
  acoplamiento y evolución guiada (caps. 2-4, 7 y 8).
- **Design Patterns:** Abstract Factory, Adapter, Bridge, Facade, Strategy y
  Template Method como vocabulario, aplicados sólo donde la variación lo requiere.

Las decisiones sobre workspaces, layouts, transferencias CUDA, equivalencia por
invariantes, control de Eigen y benchmarks corresponden además a criterios generales
de geometría computacional y HPC.
