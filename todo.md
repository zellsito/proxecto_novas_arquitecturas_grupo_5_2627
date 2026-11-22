

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


FALTA ACTUALIZAR LA WIKI CON ESTOS LINKS
wiki-bunker.lucho.gal
repo-bunker.lucho.gal
codespace-bunker.lucho.gal
nodered-bunker.lucho.gal
dash-bunker.lucho.gal

Podriamos agregar la logica de simular el uso del agua algo mas copado. nunca llega a 100 y corta por ejemplo..
o bajar progresivo como que esta en uso

el foot del dash no me gusta: BUNKEROS / NAPIoT / PRÁCTICA 1 Cloud · Node-RED Dashboard 2.0 · Datos virtuales / Prototipo académico
Umbral de Prueba cambiar solo por umbral en el front. de humedad.
y los graficos que solo digan umbral
este msj no me gusta:" Inyección directa al módulo Edge/Sensores · Propagación por MQTT en tiempo real.

No le veo sentido al menu de la izq
que se llame v3 el panel no me gusta, volaria toda la barra de arriba.
cuando encendes la humedad alta tiene un bug

seguir testeando todo....  buscar bugs, codesmell, codigo feos, comentarios innecesarios, inconsistencias con la docu y la consigna, etc.



hay un bug que encontre. si esto corre local, envia eventos al mismo topic que la nube, lo que ocasiona que todo funcione mal. se podria solucioanr con una variable de prefijo en produ ya la configure le puse MQTT_BASE_TOPIC= bunker/g5/prod
lo que habria que hacer es hacer que se use, yo corri en produ 
 pm2 start node-red --name "bunkeros" -- flows.json
 2. ¿Cómo se detiene?

  Con pm2:

  • Para pausarlo/detenerlo temporalmente:
    pm2 stop bunkeros

  • Para reanudarlo:
    pm2 restart bunkeros

  • Para eliminarlo del todo del segundo plano:
    pm2 delete bunkeros

si es esta la solucion, documentarlo correctamente en wiki y no olvidarse de configurar la pertinente variable en caso de desarrollar en local (algo que la practica no lo admite)