# 📋 Auditoría y Reporte Técnico 1: Inconsistencias, Hacks y Refactorización de Arquitectura BunkerOS

**Proyecto:** BunkerOS - Novas Arquitecturas e Paradigmas IoT (2026/2027)  
**Fecha:** 22/11/2026  
**Objetivo:** Auditar la implementación actual de flujos en Node-RED frente a la consigna académica y la documentación oficial de la Wiki, identificando inconsistencias, lógica secuestrada, malas prácticas y definiendo la arquitectura objetivo desacoplada.

---

## 1. Diagnóstico: ¿Dónde reside la lógica del "Backend" hoy?

Actualmente **no existe un flujo ni una capa dedicada y delimitada de Backend**. Las reglas de negocio, validaciones y lógica de control están fragmentadas y secuestradas en tres lugares inadecuados:

1. **Adentro de la función de simulación del Frontend (`Telemetría + multialertas + simulación independiente` - `b37500000000000d`):**
   - Evalúa umbrales de alerta (`temperatura >= 30`, `humedad >= 70`, `nivelAgua <= 20`).
   - Contiene un generador de datos aleatorios con `Math.random()` (`jitter(...)`) que se ejecuta si recibe ticks de simulación.
   - Si no hay inyección de datos de demo, depende de que un nodo externo inyecte `bunkeros/live`. La lógica de la UI y del servidor están totalmente acopladas.
2. **Adentro del script Vue del componente `ui-template` (`b37500000000000e`):**
   - Evalúa estados de severidad (`severity`), resúmenes (`summary`), genera alertas locales artificiales (`localNotice`) y gestiona interbloqueos en el cliente.
   - El frontend asume responsabilidades que corresponden exclusivamente al servidor.
3. **En el nodo `Motor de control` (`51d14cd5c1d8b32b`) dentro de la pestaña de sensores virtuales:**
   - La lógica que decide si se enciende el extractor o se corta la válvula se encuentra mezclada entre los nodos que emulan sensores, devolviendo un payload con `simulated: true`.

---

## 2. Matriz de Inconsistencias Detectadas

| Componente / Regla | Especificación (Wiki / Consigna) | Implementación Actual en Node-RED | Severidad |
| :--- | :--- | :--- | :--- |
| **Simulación de Sensores** | La consigna exige sensores virtuales publicando a MQTT (Anexo). La Wiki define que los datos emulan ESP32 por MQTT. | La función y UI del Dashboard poseen un **"Modo Demo" local con `Math.random()`** que bypasses el broker MQTT si se activan los inyectores de la pestaña de control. | **Alta** |
| **Umbral de Reserva Hídrica** | En la Wiki: alerta cuando el nivel cae por debajo de **60%** (y apertura bajo 80% / cierre a 100%). | En `Motor de control` usa `< 80` y `>= 100`. En la función del Frontend usa `<= 20`. En el Front usa `60`. Hay tres umbrales contradictorios. | **Alta** |
| **Tópicos MQTT** | La Wiki define estándar: `bunker/g5/telemetria/...`, `bunker/g5/alerta/...`, `bunker/g5/control/...`. | Se hardcodearon prefijos arbitrarios y personales: `bunker/g5/test-ulvi-1882-7f3d/...` y `bunker/g5/test-fase1-1884-6c2f/...`. Cambiar un topic rompe las funciones. | **Alta** |
| **Correlación Clima + Escotilla** | Wiki: *Si la escotilla está abierta y OpenWeatherMap reporta lluvia o tormenta, emitir alerta preventiva prioritaria.* | OpenWeatherMap solo envía datos de texto para pintar la ciudad y temperatura. **No existe ninguna función de correlación de eventos**. | **Media** |
| **Confirmación de Actuadores** | Wiki: Comandos viajan por MQTT al actuador y este confirma el estado físico real tras la maniobra. | El backend responde en el mismo nodo emitiendo el estado con el flag `simulated: true`. | **Baja (Fase 1)** |
| **Persistencia de Estado** | Datos históricos y estados críticos deben preservarse ante reinicios. | Se usa `context.get(...)` en memoria RAM volátil sin contexto de archivo o base de datos. Si se reinicia el contenedor, el estado se resetea. | **Media** |

---

## 3. Código Hacks y Malas Prácticas Relevantes

1. **Hack de jitter aleatorio en UI:**
   ```javascript
   const jitter=(x,delta)=>Math.round((x+(Math.random()-.5)*delta)*10)/10;
   apply({temperatura:jitter(f.calor?34:23.5,.8),humedad:jitter(f.humedad?74:53,1.6),...});
   ```
2. **Números mágicos y umbrales dispersos:**
   Falta una configuración centralizada de umbrales ambientales y de seguridad.
3. **Contenedor Docker sin persistencia de volumen:**
   El contenedor `mynodered` fue iniciado sin `-v /ruta:/data`, por lo que cambios en la paleta o dependencias npm (`@flowfuse/node-red-dashboard`) se perderían al recrear el contenedor.

---

## 4. Arquitectura Objetivo Desacoplada

Para garantizar que en el futuro los sensores físicos reales (ESP32) reemplacen la simulación sin tocar una sola línea de lógica ni de frontend, la solución se estructurará en tres flujos independientes:

```
┌────────────────────────────────────────────────────────────────────────┐
│ FLOW 1: CAPA EDGE / SIMULACIÓN (TEMPORAL - SE ELIMINA EN FASE 2)       │
│  - Inyectores temporizados simulando hardware.                        │
│  - Formateo de payloads según esquema de la Wiki.                     │
│  - Publicación a topics oficiales `bunker/g5/telemetria/...`           │
│  - Receptor de comandos `bunker/g5/control/...` y emulador de FSM.    │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │
                                   ▼ (Broker MQTT: test.mosquitto.org)
┌────────────────────────────────────────────────────────────────────────┐
│ FLOW 2: CAPA BACKEND (LÓGICA DE NEGOCIO Y CORRELACIÓN) - PERMANENTE    │
│  - Suscripción centralizada a MQTT (`bunker/g5/#`).                   │
│  - Consumo y parseo de OpenWeatherMap API.                            │
│  - Motor de reglas de negocio:                                         │
│      * Evaluación de umbrales ambientales.                            │
│      * Correlación Clima (lluvia) + Escotilla abierta.                │
│      * Interbloqueo de seguridad (Safety Interlock).                   │
│  - Generación de objeto de estado unificado para el Dashboard.        │
│  - Enrutamiento y validación de comandos manuales hacia MQTT.          │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │
                                   ▼ (Link Nodes / WebSocket interno)
┌────────────────────────────────────────────────────────────────────────┐
│ FLOW 3: CAPA FRONTEND (DASHBOARD 2.0) - PERMANENTE Y AGNOSTICO        │
│  - CERO lógica de negocio y CERO generación de valores ficticios.      │
│  - Presentación reactiva de métricas, gráficos y alertas del Backend. │
│  - Emisión de intenciones de control (botones) hacia el Backend.       │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 5. Plan de Acción de Refactorización

1. **Fase 1 (Backend Centralizado):** Crear el flujo `P1 · Backend & Lógica de Negocio` conteniendo el motor de correlación, interbloqueos y consolidación de estado hacia el Frontend.
2. **Fase 2 (Saneamiento de Tópicos):** Homogeneizar todos los topics a la raíz `bunker/g5/` según especificación de la Wiki.
3. **Fase 3 (Depuración del Frontend):** Remover del `ui-template` y de sus funciones previas cualquier generación sintética de datos y reglas de alarma locales.
4. **Fase 4 (Aislamiento de la Simulación):** Confinar los sensores simulados estrictamente al Flow 1 (Capa Edge), garantizando que su desconexión no afecte la integridad del sistema.
