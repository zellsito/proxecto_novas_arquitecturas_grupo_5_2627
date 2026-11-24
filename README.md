# 🛡️ BunkerOS: Arquitectura IoT Resiliente (Edge-to-Cloud)

> **Materia:** Novas Arquitecturas e Paradigmas IoT  
> **Curso Académico:** 2026 / 2027  
> **Grupo de Trabajo:** Grupo 5  

Sistema de soporte vital, supervisión ambiental preventiva y gestión automática de incidencias para refugios subterráneos de emergencia. Implementa un modelo de degradación elegante y tolerancia a fallos conectando el Edge local (ESP32) con la Nube (**Node-RED en Nube**).

---

## 👥 Integrantes del Equipo y Roles

| Integrante | Rol | Responsabilidad Principal |
| :--- | :--- | :--- |
| **Luciano Ezequiel Salgado Bucco** | **Software Architect** | Diseño arquitectónico global, balance Edge-Cloud y especificación técnica |
| **Ulviyya Hasanova** | **UX / UI** | Diseño e implementación de Dashboards (Node-RED Dashboard 2.0 y WebServer local) |
| **Daniel Pérez Pérez** | **DevOps** | Entornos de despliegue, CI/CD y gestión de Brokers MQTT |
| **Rubén Fernández Bastos** | **Data Engineer** | Modelado de datos JSON, ingestión de telemetría y reglas de procesamiento |

---

## 📚 Documentación Oficial del Proyecto

Toda la especificación técnica, justificaciones de arquitectura, contratos MQTT y diseño de mitigaciones se encuentran centralizados en la **Wiki oficial del repositorio**:

👉 **[Acceder a la Wiki Oficial de BunkerOS](https://github.com/zellsito/proxecto_novas_arquitecturas_grupo_5_2627/wiki)**

---

## 📂 Estructura del Repositorio

```text
proxecto_novas_arquitecturas_grupo_5_2627/
├── README.md                     # Ficha técnica oficial del proyecto
├── flows.json                    # Flujos finales integrados de Node-RED (P1)
├── danielperez/                  # Módulos y entregables DevOps / Despliegue
├── lucianosalgado/               # Diagramas de arquitectura y especificación
├── rubenfernandez/               # Módulos Data Engineer / Modelado de telemetría
└── ulviyyahasanova/              # Módulos UX/UI y Dashboards
```

---
*Grupo 5 — Novas Arquitecturas e Paradigmas IoT (2026/2027)*
