# Presentación web autónoma del rediseño de 3D-GPolylla

**Fecha:** 2026-07-28

**Estado:** diseño aprobado; especificación para revisión final

**Documento fuente:**
`docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md`

## 1. Propósito

Crear una presentación web simple, directa y autocontenida que permita comprender y
digerir el documento completo de rediseño sin una exposición oral. No sustituye al
documento técnico: lo convierte en una narrativa de lectura guiada con resúmenes,
diagramas y relaciones visuales.

## 2. Audiencia y modo de uso

La audiencia principal es técnica y puede comprender C++, arquitectura de software,
geometría computacional y paralelismo. La presentación se consume de manera
autónoma, mediante scroll vertical, en escritorio o móvil.

El lector debe poder:

- Entender el problema actual antes de ver la solución.
- Reconocer las decisiones principales sin leer cada detalle.
- Profundizar en riesgos, fuentes y consecuencias cuando lo necesite.
- Volver directamente a cualquier capítulo mediante enlaces estables.
- Completar la lectura sin controles ocultos o instrucciones especiales.

## 3. Enfoque seleccionado

Se implementará una narrativa vertical guiada de una sola página. Se descartan un
deck horizontal de diapositivas y un mapa arquitectónico no lineal porque ambos
dificultan la lectura autónoma y la consulta completa.

Cada capítulo contiene:

1. Número y título.
2. Una frase que resume la decisión.
3. Un diagrama o relación visual simple cuando aporta comprensión.
4. Entre tres y seis ideas esenciales.
5. Detalles complementarios sólo cuando son necesarios.

## 4. Recorrido de contenido

### 1. Portada y north star

Presenta el repositorio como consolidación de una familia de algoritmos de
generación de poliedros. Expone en una frase el cambio desde una implementación
acoplada hacia una plataforma algorítmica extensible.

### 2. Por qué rediseñar

Resume las señales observadas en el código actual: modelo mutable, herencia de
Eigen, algoritmo stateful, renderer monolítico, CLI mezclado con experimentos y
dependencias filtradas. Una composición “actual -> propuesto” evita convertir esta
sección en una crítica extensa.

### 3. Objetivos y prioridades

Ordena corrección, extensibilidad, control de rendimiento, portabilidad,
reproducibilidad y usabilidad. Distingue objetivos de no objetivos.

### 4. Decisión arquitectónica

Explica el microkernel lógico con pipelines tipados y la razón para no adoptar una
jerarquía completamente dinámica ni un pipeline público completamente templado.

### 5. Los tres ejes

Presenta `model`, `statistics` y `algorithms`, sus responsabilidades y la regla de
dependencias. Adaptadores y aplicaciones aparecen en la periferia.

### 6. Modelo y memoria

Muestra IDs fuertes, buffers propios y separación de geometría, topología y
atributos. Incluye una franja visual del layout canónico.

### 7. El papel de Eigen

Resume la regla “Eigen calcula; GPolylla controla memoria, flujo y concurrencia”.
Diferencia buffers del dominio de `Eigen::Map` y operaciones privadas.

### 8. Pipeline algorítmico

Representa `Prepare -> Label -> Construct -> Repair -> Materialize`, diferenciando
las tres fases semánticas de las fronteras técnicas.

### 9. Matriz de extensibilidad

Relaciona cada cambio con su punto de extensión: formato, criterio, algoritmo,
workspace, backend, métrica y visualización.

### 10. CPU, paralelismo y CUDA

Compara CPU secuencial, CPU paralela y CUDA. Ilustra una transferencia inicial,
permanencia en el backend y materialización final.

### 11. Estadísticas y adaptadores

Separa métricas centrales, estadísticas avanzadas y formatos de entrada. Explica
qué dependencias son opcionales y privadas.

### 12. Ejecutables

Compara CLI, experiments y demo. La demo se divide en aplicación, rendering y UI;
ningún ejecutable contiene lógica algorítmica.

### 13. Build y portabilidad

Resume targets CMake, `find_package`, vcpkg para desarrollo reproducible y opciones
independientes para demo, estadísticas avanzadas y CUDA. CPU es la capacidad común
de Linux, Windows y macOS.

### 14. Pruebas y robustez

Expone unit tests, suites contractuales, invariantes, pruebas metamórficas,
comparación de backends y experiments reproducible. Separa corrección, robustez y
rendimiento.

### 15. Guía de estilo

Presenta convenciones de nombres, ownership, errores, reglas numéricas, hot paths y
el target local `check`. No reproduce la guía completa.

### 16. Migración

Muestra las etapas desde caracterización hasta cutover, destacando el primer corte
vertical de Cavity y el spike CUDA.

### 17. Criterios de éxito, riesgos y fuentes

Cierra con una checklist de aceptación, los riesgos principales y la bibliografía
que respalda el diseño. Incluye un enlace al documento técnico completo.

## 5. Diseño visual

La apariencia debe ser técnica, académica y sobria:

- Fondo marfil muy claro.
- Texto azul petróleo casi negro.
- Cobre para decisiones y cambios importantes.
- Verde azulado para módulos y estados saludables.
- Tipografía del sistema; no se descargan fuentes externas.
- Monoespaciada para código, targets y nombres de módulos.
- Ancho de lectura controlado y separación vertical generosa.
- Bordes y fondos planos; sin gradientes decorativos ni glassmorphism.

Las ilustraciones se construyen con HTML y CSS. No se requieren fotografías,
imágenes generadas ni SVGs dibujados manualmente.

## 6. Sistema de ilustraciones

Se usarán composiciones consistentes:

- Tarjetas conectadas para arquitectura y dependencias.
- Franjas segmentadas para layouts de memoria.
- Pasos numerados para el pipeline.
- Tabla compacta para dimensiones de extensibilidad.
- Tres carriles para CPU secuencial, CPU paralela y CUDA.
- Capas apiladas para niveles de pruebas.
- Timeline para migración.
- Checklist para criterios de aceptación.

El color nunca será la única forma de comunicar estado o dependencia.

## 7. Navegación e interacción

- Scroll vertical nativo.
- Índice lateral fijo en escritorio.
- Barra superior compacta en pantallas pequeñas.
- Progreso de lectura visible.
- Enlaces con anchors estables para los 17 capítulos.
- Resaltado de la sección actual mediante observación de intersección.
- Bloques expandibles sólo para fuentes, riesgos y detalles secundarios.
- Todo el contenido esencial es visible sin interacción.
- Focus visible y navegación completa mediante teclado.
- El movimiento respeta `prefers-reduced-motion`.

No habrá carrusel, paginación horizontal, búsqueda, autenticación, persistencia,
backend ni estado remoto.

## 8. Responsive y accesibilidad

- Escritorio: índice lateral y contenido central.
- Tablet: índice compacto y diagramas reordenados.
- Móvil: una columna, barra de navegación superior y tablas convertidas en tarjetas
  cuando sea necesario.
- Estructura semántica con `header`, `nav`, `main`, `section` y `footer`.
- Orden de headings consistente.
- Contraste suficiente, controles con labels y targets táctiles adecuados.
- La página sigue siendo comprensible con JavaScript deshabilitado; sólo se pierde
  el resaltado automático y el progreso dinámico.

## 9. Arquitectura de la presentación

La presentación vive como un sitio independiente de una sola ruta. La primera
versión utiliza un componente de página y un stylesheet principal. Los datos de las
secciones pueden declararse localmente cuando eviten repetición, sin construir un
CMS o sistema genérico de slides.

```text
presentation/
|-- app/
|   |-- page.tsx
|   |-- layout.tsx
|   `-- globals.css
|-- public/
|-- package.json
`-- .openai/hosting.json
```

La presentación no forma parte del build CMake de la biblioteca y no añade
dependencias a 3D-GPolylla.

## 10. Manejo de contenido y errores

El contenido es estático y se obtiene del documento aprobado. No existen llamadas de
red ni estados de carga. Si JavaScript falla, anchors, contenido y elementos
`details` continúan funcionando mediante HTML.

El documento técnico permanece como fuente de verdad. La presentación enlaza a su
versión en el repositorio:
`https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md`.
No intenta sincronizarlo automáticamente ni parsear Markdown en runtime.

## 11. Validación

Antes de entregar:

- El build de producción debe completar sin errores.
- Los 17 capítulos deben estar presentes y en el orden aprobado.
- Cada decisión central del documento debe aparecer al menos una vez.
- No debe haber texto placeholder, contenido starter o metadatos genéricos.
- Los anchors deben ser únicos y legibles.
- La navegación debe funcionar con teclado.
- Los layouts deben ser legibles en anchos de escritorio y móvil mediante CSS
  responsive.
- La página no debe depender de servicios externos para renderizar contenido.

No se requiere una suite de tests automatizados para esta presentación estática. El
build, una revisión estructural del HTML y la comprobación del contenido son
suficientes para la primera versión.

## 12. Entrega

La entrega incluye:

- Código fuente dentro de `presentation/`.
- Build de producción validado.
- URL desplegada mediante Sites, salvo que el usuario solicite mantenerla local.
- Enlace al documento técnico completo desde la sección final.

## 13. Criterios de aceptación

La presentación se considera completa cuando un lector técnico puede:

1. Explicar el north star y el problema actual.
2. Nombrar los tres ejes y su dirección de dependencias.
3. Comprender el control de memoria y el papel privado de Eigen.
4. Describir las tres fases y las dimensiones de variación.
5. Explicar cómo se incorporará CUDA sin contaminar el modelo.
6. Distinguir responsabilidades de CLI, experiments y demo.
7. Resumir testing, estilo y estrategia de migración.
8. Identificar riesgos, trade-offs y condiciones de éxito.

La experiencia debe sentirse como una lectura técnica guiada, no como una landing
page comercial ni como una copia del documento Markdown.
