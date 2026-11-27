#!/usr/bin/env bash
# ==============================================================================
# Abre el monitor serie de Arduino en consola usando arduino-cli
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=_common.sh
source "$SCRIPT_DIR/_common.sh"

PORT=""
BAUD="${ARDUINO_BAUD:-115200}"

usage() {
    cat <<'EOF'
Uso: monitor.sh [opciones] [baudrate]

Opciones:
  -p, --port <puerto>       Puerto serie (ej: /dev/ttyACM0). Si se omite, se autodetecta.
  -b, --baud <velocidad>    Velocidad en baudios (por defecto: 115200 o ARDUINO_BAUD)
  -h, --help                Muestra esta ayuda

Ejemplos:
  monitor.sh                # Abre el puerto detectado a 115200 baudios
  monitor.sh 9600           # Abre el puerto detectado a 9600 baudios
  monitor.sh -p /dev/ttyUSB0 -b 115200
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -p|--port)
            PORT="${2:-}"
            shift 2
            ;;
        -b|--baud)
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
            if [[ "$1" =~ ^[0-9]+$ ]]; then
                BAUD="$1"
            else
                PORT="$1"
            fi
            shift
            ;;
    esac
done

require_arduino_cli

PORT=$(detect_port "$PORT")
check_port_permissions "$PORT"

echo "[*] Abriendo monitor serie en $PORT a $BAUD baudios (Ctrl+C para salir)..."
exec arduino-cli monitor -p "$PORT" --config "baudrate=$BAUD"
