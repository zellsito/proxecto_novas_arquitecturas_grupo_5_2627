# proxecto_novas_arquitecturas_grupo_5_2627
 En palabras simples y directas, el Edge es el reflejo involuntario del sistema:

  │ La analogía del cuerpo humano:
  │ Si tocas una sartén caliente, tu médula espinal (Edge) saca la mano en 10 milisegundos sin esperar a
  que
  │ la orden vaya a tu cerebro y vuelva. Tu cerebro (Cloud) se entera un segundo después de lo que pasó.

  En tu búnker, el Edge (los ESP32) hace 3 cosas concretas:
  ──────
  ### 1. Actúa al instante ante el peligro (Cero Latencia)

  • Si hay agua en el suelo ➔ El Edge cierra la válvula (servomotor) en 5 milisegundos.
  • No puede esperar a enviar los datos a un servidor en internet, que el servidor lo procese y le devuelva
  la orden de cerrar. Para ese momento el búnker ya se inundó.

  ### 2. Garantiza la supervivencia sin Internet (Autonomía)

  • Si cortas el cable de WiFi o se cae la red ➔ Todo sigue funcionando igual:
      • Si la temperatura sube, el Edge prende el ventilador.
      • Si abren la escotilla, el Edge hace sonar la alarma.
      • Los ESP32 se coordinan entre ellos por radio (ESP-NOW) aunque no haya internet en todo el planeta.


  ### 3. Filtra la basura y ahorra datos

  • Los sensores físicos tienen ruido (lecturas falsas). El Edge limpia la señal (calcula promedios).
  • No satura internet: En vez de mandar 10 lecturas por segundo a la nube, el Edge las procesa localmente
  y a la nube solo le manda un mensaje: "Todo normal" o "¡Emergencia: fuga detectada!".

────────────────────────────────────────────────────────────
> mostrame la arq de nuevo

▸ Thought for 9s

  Aquí tienes el mapa completo y definitivo de la arquitectura BunkerOS, con los 3 ESP32, los 4 sensores,
  los 4 actuadores y las 3 capas:
  ──────
    ========================================================================================
                                         CAPA CLOUD (Nube)
       • Dashboard en el Celular / Web (Blynk / Adafruit IO) ➔ Gráficos y control remoto
       • API Externa (Open-Meteo) ➔ Consulta pronóstico de lluvias y tormentas
    ========================================================================================
                                               ▲
                                               │  Internet (WiFi / MQTT + HTTPS)
                                               ▼
    ========================================================================================
                       CAPA GATEWAY & EDGE DE SUPERFICIE (Garita / Exterior)
                               [ ESP32 #3: NODO HMI / SUPERVISIÓN ]
       • Actuador 1: Pantalla LCD 16x2 I2C  ➔ Muestra estado general y alertas a humanos
       • Actuador 2: Buzzer (Sirena)        ➔ Alarma sonora exterior ante peligro
       • Lógica Edge: Cruza pronóstico de lluvia (API) con el estado interior del búnker
    ========================================================================================
                                      ▲                    ▲
                                      │                    │
                  ESP-NOW             │                    │             ESP-NOW
                Directo P2P           │                    │           Directo P2P
            (Sin router / Sin IP)     │                    │       (Sin router / Sin IP)
                                      ▼                    ▼
    ========================================================================================
                        CAPA DESCENTRALIZADA & EDGE DISTRIBUIDO (Bajo Tierra)
                         (Se coordinan entre sí directamente con ESP-NOW)
    
        [ ESP32 #1: AMBIENTE Y ACCESO ]               [ ESP32 #2: GESTIÓN HÍDRICA ]
        (Sector Habitabilidad)                        (Sector Sala de Máquinas)
        --------------------------------              --------------------------------
        • Sensor 1: DHT11 (Temp / Humedad)            • Sensor 3: Ultrasonido HC-SR04
        • Sensor 2: Reed Switch (Escotilla)             (Nivel del tanque de agua)
        • Actuador 3: Relé (Ventilador)               • Sensor 4: Sensor de Agua
                                                        (Detección de fugas en suelo)
        • Decisión Edge Local:                        • Actuador 4: Servomotor
          Si Humedad > 75% ➔ Prende                     (Válvula de corte de agua)
          el ventilador de inmediato.
                                                      • Decisión Edge Local:
                                                        Si hay agua en el piso ➔ Cierra
                                                        la válvula en 5 milisegundos.
                                      ▲                    ▲
                                      └──── ESP-NOW P2P ───┘
                                       (Mensajes directos:
                                        ej. Fuga ➔ Apagar relé)
    ========================================================================================
  ──────
  ### Resumen Rápido de las 3 Capas

  1. Capa Descentralizada (P2P con ESP-NOW):
      • El ESP32 #1 y el ESP32 #2 están bajo tierra y se hablan por radio directa sin pasar por ningún
      servidor central ni router.
      • Si el Nodo 3 de la superficie se rompe o se queda sin energía, los 2 nodos subterráneos siguen
      vivos y coordinados.
  2. Capa Edge (En los 3 ESP32):
      • Nodo 1: Filtra lecturas del DHT11 y prende el ventilador automáticamente por histéresis.
      • Nodo 2: Detecta la fuga y cierra la válvula con el servomotor al instante (cero latencia).
      • Nodo 3: Calcula el índice de riesgo de anegamiento (cruce de API externa + estado del tanque).
  3. Capa Cloud (WiFi + MQTT):
      • El Nodo 3 sube los eventos a la nube (Adafruit IO / Blynk).
      • Tienes los gráficos en el celular y recibes alertas push a distancia estés donde estés.

