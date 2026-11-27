#!/usr/bin/env bash
# ==============================================================================
# Compila y flashea un sketch Arduino (.ino) usando arduino-cli
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=_common.sh
source "$SCRIPT_DIR/_common.sh"

BOARD=""
PORT=""
SKETCH=""
NO_BUILD=false
OPEN_MONITOR=false
BAUD="115200"

usage() {
    cat <<'EOF'
Uso: flash.sh [opciones] [ruta_sketch]

Opciones:
  -b, --board <fqbn|alias>  Placa destino (uno, nano, nano-old, mega, esp32, esp32s3, esp32c3 o FQBN completo)
  -p, --port <puerto>       Puerto serie (ej: /dev/ttyACM0, /dev/ttyUSB0). Si se omite, se autodetecta.
  -n, --no-build            Solo sube sin recompilar antes
  -m, --monitor             Abre el monitor serie automáticamente después de flashear
  -B, --baud <velocidad>    Velocidad del monitor si se usa -m (por defecto: 115200)
  -h, --help                Muestra esta ayuda

Variables de entorno opcionales:
  ARDUINO_BOARD             Placa por defecto
  ARDUINO_PORT              Puerto por defecto
  ARDUINO_BAUD              Velocidad por defecto del monitor

Ejemplos:
  flash.sh                          # Compila y sube el .ino actual al puerto detectado
  flash.sh -b uno -m -B 9600        # Compila, sube a Arduino Uno y abre monitor a 9600
  flash.sh -b esp32 -p /dev/ttyUSB0 ./mi_sketch
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -b|--board)
            BOARD="${2:-}"
            shift 2
            ;;
        -p|--port)
            PORT="${2:-}"
            shift 2
            ;;
        -n|--no-build)
            NO_BUILD=true
            shift
            ;;
        -m|--monitor)
            OPEN_MONITOR=true
            shift
            ;;
        -B|--baud)
            BAUD="${2:-}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            echo "[!] Opción desconocida: $1"
            usage
            exit 1
            ;;
        *)
            SKETCH="$1"
            shift
            ;;
    esac
done

require_arduino_cli

SKETCH_DIR=$(resolve_sketch "$SKETCH")
FQBN=$(detect_fqbn "$SKETCH_DIR" "$BOARD")
PORT=$(detect_port "$PORT")
check_port_permissions "$PORT"

if [ "$NO_BUILD" = false ]; then
    COMPILE_CMD=(arduino-cli compile)
    [ -n "$FQBN" ] && COMPILE_CMD+=(--fqbn "$FQBN")
    COMPILE_CMD+=("$SKETCH_DIR")
    echo "[*] Compilando '$SKETCH_DIR'${FQBN:+ (placa: $FQBN)}..."
    "${COMPILE_CMD[@]}"
fi

UPLOAD_CMD=(arduino-cli upload -p "$PORT")
[ -n "$FQBN" ] && UPLOAD_CMD+=(--fqbn "$FQBN")
UPLOAD_CMD+=("$SKETCH_DIR")

echo "[*] Flasheando en $PORT..."
"${UPLOAD_CMD[@]}"
echo "[✓] Flasheo completado."

if [ "$OPEN_MONITOR" = true ]; then
    exec "$SCRIPT_DIR/monitor.sh" -p "$PORT" -b "$BAUD"
fi
