# 🛡️ BunkerOS: Arquitectura IoT Resiliente (Edge-to-Cloud)

> **Materia:** Novas Arquitecturas e Paradigmas IoT  
> **Curso Académico:** 2026 / 2027  
> **Grupo de Trabajo:** Grupo 5  

Sistema de soporte vital, supervisión ambiental preventiva y gestión automática de incidencias para refugios subterráneos de emergencia. Implementa un modelo de degradación elegante y tolerancia a fallos conectando el Edge local (ESP32) con la Nube (**Node-RED en Nube**).

---

## 👥 Integrantes del Equipo y Roles

| Integrante | Rol | Responsabilidad Principal |
| :--- | :--- | :--- |
| **Luciano Ezequiel Salgado Bucco** (DNI: 43944385X) | **Software Architect** | Diseño arquitectónico global, balance Edge-Cloud y especificación técnica |
| **Ulviyya Hasanova** | **UX / UI** | Diseño e implementación de Dashboards (Node-RED Dashboard 2.0 y WebServer local) |
| **Daniel Pérez Pérez** (DNI: 35666868D) | **DevOps** | Entornos de despliegue, Docker Compose, CI/CD y gestión de Brokers MQTT |
| **Rubén Fernández Bastos** (DNI: 39494438B) | **Data Engineer** | Modelado de datos JSON, ingestión de telemetría y reglas de procesamiento |

---

## 📚 Documentación Oficial del Proyecto

Toda la especificación técnica, justificaciones de arquitectura y protocolos se encuentran centralizados en la **Wiki oficial del repositorio**:

👉 **[Acceder a la Wiki Oficial de BunkerOS](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki)**

### Secciones Principales en la Wiki:
* 📐 **[1. Arquitectura General](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Arquitectura-General)**: Diagrama completo de bloques, capas y componentes.
* ⚖️ **[2. Edge vs Cloud: Reparto de Lógica](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Edge-vs-Cloud-Logica)**: Matriz anti-colisión y jerarquía de seguridad (*Safety Interlock*).
* 📡 **[3. Protocolos y Tópicos MQTT](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Protocolos-y-Topicos-MQTT)**: Estándar `bunker/g5/...` y payloads JSON.
* 📶 **[4. Modo Offline y Acceso Local](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Modo-Offline-Acceso-Local)**: Operación sin Internet y servidor web HTTP local del ESP32.
* 🚀 **[5. Evolución por Fases](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Evolucion-por-Fases)**: Transición Fase 1 (Cloud) $\rightarrow$ Fase 2 (Edge) $\rightarrow$ Fase 3 (P2P).
* 🛡️ **[6. Defensa y Resiliencia](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki/Defensa-y-Resiliencia)**: Matriz ante fallos catastróficos y justificación para la defensa del proyecto.

---

## 🛠️ Stack Tecnológico Resumen

| Capa | Tecnologías | Propósito |
| :--- | :--- | :--- |
| **Cloud** | Node-RED (en Nube) + Dashboard 2.0 | Supervisión remota, reglas contextuales, histórico |
| **Servicios Externos** | OpenWeatherMap API | Contexto meteorológico exterior (lluvia, tormenta) |
| **Broker** | MQTT | Mensajería asíncrona y desacoplada |
| **Edge Gateway** | ESP32 (Wi-Fi + WebServer HTTP + ESP-NOW) | Puente Cloud/P2P e interfaz de emergencia local |
| **Edge Subterráneo** | ESP32 + Sensores/Actuadores físicos | Reflejos de latencia cero (< 5 ms) mediante ESP-NOW |

---

## 📂 Estructura del Repositorio

```text
proxecto_novas_arquitecturas_grupo_5_2627/
├── README.md                 # Esta ficha técnica del proyecto
├── lucianosalgado/           # Diagramas draw.io y recursos gráficos
│   ├── arq.svg
│   ├── arquitectura.drawio
│   └── README.md
└── ...                       # Código fuente de ESP32 y flujos Node-RED
```

---
*Grupo 5 — Novas Arquitecturas e Paradigmas IoT (2026/2027)*
