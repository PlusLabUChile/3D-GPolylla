# Experimentos reproducibles

`GPolyllaExperiment` se compila junto a la CLI con `GPOLYLLA_EXE=ON`. Es una entrada experimental separada para conservar compatibilidad con el visor y los comandos existentes.

```
GPolyllaExperiment X.node X.ele cavity-ascending out/prefix 1
GPolyllaExperiment X.node X.ele face-incircle-repair out/prefix 1
GPolyllaExperiment X.node X.ele membership out/prefix membership.txt
GPolyllaExperiment X.node X.ele cavity-ascending out/flat 1 --flat
```

`GPolyllaMemory X.node X.ele inventory.csv` exporta capacidades por componente: entrada, copia común de salida, listas por celda y auxiliares retenidos. No mide metadatos del asignador ni sustituye el máximo RSS.

Exporta pertenencia por tetraedro y un CSV de tiempos en milisegundos. La última opción construye y mide una representación aplanada adicional; se usa solo en E6. Los tiempos de algoritmo incluyen construcción de salida y una validación de la partición en todas las variantes. Otra validación se registra por separado. La escritura es la del contrato de pertenencia, no VISF.

Variantes: `cavity-{ascending,descending,index,random}`, `face-{area,incircle}` con sufijo opcional `-repair`, `pairs` e `identity`. Las reparaciones inválidas se rechazan explícitamente. `pairs` limita a N/5 las uniones, cada una entre el mayor tetraedro restante y su vecino libre de cara más amplia.

Preparación Python:

```
python3 -m venv .venv
.venv/bin/pip install -r tools/requirements-experiments.txt
GPOLYLLA_EXE=/ruta/GPolyllaExe .venv/bin/python -m unittest discover -s tools/tests -v
```

- `gen_mesh.py`: familias Random, Poisson, Grid y Quality, calibración y hashes TetGen.
- `run_baseline.py`: oráculos Python Face/Edge desde una copia externa a `3D-Polylla`; parametriza criterios y reparación y registra SHA-256.
- `parity.py`: regresión de los puertos C++ contra los oráculos.
- `evaluate.py`: evaluación común de pertenencia, cuatro estados de kernel y métricas antes/después de unir coplanares. Los campos vacíos son indeterminados, errores o métricas no disponibles; no ceros.
- `prepare_real.py`: selección reproducible Thingi10K, filtros, licencias, tetraedrización y exclusiones.
- `campaign.py`: E1/E2/E4, repetición independiente, validación y checkpoints. Consultar `--help` para rutas y etapas. Ejecutar sobre copias congeladas de scripts y binarios.

El plan y las decisiones efectivas están en `../docs/plan-experimentos.md` y `../docs/ejecucion-experimentos-2026-11-03.md` del repositorio padre Tesis. El CSV de la CLI histórica sigue disponible; la campaña utiliza exclusivamente el evaluador común y conserva las métricas legadas con nombres explícitos.

`audit_kernels.py` verifica todas las ausencias primarias con certificados duales
y conserva los datos originales. `analyze.py` exige esa auditoría y aplica el
overlay derivado si alguna ausencia pasa a indeterminada. La versión corregida
de `evaluate.py` exige el mismo certificado. `evaluate_timed.py` mide por separado
lectura, estadísticas por celda y agregación/exportación; no modifica las métricas.
`validate_published.py` importa y verifica mallas del dataset publicado de P035;
`validate_scale.py` comprueba pertenencia bajo reescalamiento y traslación.

En E6, el campo histórico `flat_build_ms` abarca el ciclo de reservar, llenar y
liberar los contenedores aplanados. Se exporta también como `flat_cycle_ms` con
su alcance explícito; no se interpreta como una comparación de velocidad ni
como el pico RSS de una implementación que sustituya las listas actuales.
