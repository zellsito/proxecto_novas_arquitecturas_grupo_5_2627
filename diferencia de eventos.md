# Diferencia de Eventos: Especificación Wiki vs. Implementación BunkerOS

Este documento detalla la comparativa formal entre los eventos definidos en la documentación técnica de la cátedra ([Protocolos-y-Topicos-MQTT.md](wiki_repo/Protocolos-y-Topicos-MQTT.md)) y los eventos procesados por el motor de BunkerOS.

---

## 1. Matriz de Correspondencia de Eventos Oficiales

| Tópico Oficial MQTT | Evento en la Wiki | Evento en BunkerOS | Semántica en Registro Histórico | Estado |
| :--- | :--- | :--- | :--- | :---: |
| `bunker/g5/evento/hidrico` | Cierre al alcanzar el 100% | `VALVE_CLOSED` | Válvula de agua cerrada (Depósito lleno) | **Idéntico** |
| `bunker/g5/evento/hidrico` | Apertura por debajo del 80% | `VALVE_OPENED` | Válvula de agua abierta (Relleno automático) | **Idéntico** |
| `bunker/g5/evento/ventilacion` | Encendido del ventilador | `FAN_ON` | Extractor de aire encendido | **Idéntico** |
| `bunker/g5/evento/ventilacion` | Apagado del ventilador | `FAN_OFF` | Extractor de aire apagado | **Idéntico** |
| `bunker/g5/alerta/fuga` | Alerta inmediata de agua en suelo | `WATER_LEAK_TRIGGERED` | ¡Alerta crítica! Fuga de agua detectada en suelo | **Idéntico** |
| `bunker/g5/alerta/hidrico` | Alerta por nivel bajo (< 60%) | `LOW_WATER_LEVEL` | Aviso: Reserva hídrica baja (< 60 %) | **Idéntico** |
| `bunker/g5/alerta/escotilla` | Apertura no autorizada | `UNAUTHORIZED_HATCH_OPEN` | ¡Alerta de intrusión! Escotilla abierta sin autorización | **Idéntico** |

---

## 2. Eventos Adicionales de Robustez Implementados en el Sistema

Para cumplir con las buenas prácticas de arquitectura *Event-Driven* y garantizar que el log histórico de auditoría sea verídico y refleje el ciclo de vida completo de cada incidencia, se incorporaron los siguientes eventos complementarios:

### A. Eventos de Normalización / Recuperación (`CLEARED`)
*En la Wiki únicamente se definió el disparo de la alarma; en sistemas industriales se requiere constancia del cese de la emergencia.*
* `WATER_LEAK_CLEARED`: Se emite cuando el sensor de suelo se seca y cesa la presencia de agua.
* `HATCH_ALERT_CLEARED`: Se emite cuando la escotilla vuelve a cerrarse y el perímetro queda asegurado.
* `WATER_LEVEL_RECOVERED`: Se emite cuando la reserva hídrica supera nuevamente el umbral seguro.

### B. Eventos de Interbloqueo de Seguridad Físico (`REJECTED`)
*Definidos conceptualmente en [Edge-vs-Cloud-Logica.md](wiki_repo/Edge-vs-Cloud-Logica.md) como prioridad absoluta del hardware sobre la nube.*
* `REJECTED_INTERLOCK`: El Coordinador Edge rechaza la apertura remota de la válvula si existe una fuga activa en el suelo.
* `REJECTED_SAFETY_OVERRIDE`: El Coordinador Edge rechaza apagar el extractor de aire si la temperatura ambiente supera el umbral crítico (30 °C).

### C. Eventos de Seguridad Perimetral (`ARM` / `DISARM`)
*Permiten discriminar entre accesos autorizados y aperturas no autorizadas.*
* `ACCEPTED_ARM`: Perímetro armado en Modo Sellado (toda apertura dispara intrusión).
* `ACCEPTED_DISARM`: Perímetro desarmado en Modo Acceso Autorizado (la apertura se reporta como operación normal sin generar alarma de intrusión).

---

## 3. Resumen Arquitectónico
El sistema respeta al 100% la convención de tópicos bajo el prefijo `bunker/g5/`, garantiza que ningún comando manual de la nube viole los interbloqueos de seguridad físicos en el Edge, y registra cada evento una única vez de manera idempotente a partir de la confirmación MQTT.
