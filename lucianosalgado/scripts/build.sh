#!/usr/bin/env bash
# ==============================================================================
# Compila un sketch Arduino (.ino) usando arduino-cli
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=_common.sh
source "$SCRIPT_DIR/_common.sh"

BOARD=""
SKETCH=""
CLEAN=false
EXPORT=false
EXTRA_ARGS=()

usage() {
    cat <<'EOF'
Uso: build.sh [opciones] [ruta_sketch] [-- args_extra_arduino_cli]

Opciones:
  -b, --board <fqbn|alias>  Placa destino (ej: uno, nano, nano-old, mega, esp32, esp32s3, esp32c3 o FQBN completo)
  -c, --clean               Fuerza compilación limpia (--clean)
  -e, --export              Exporta los binarios compilados en ./build dentro del sketch
  -h, --help                Muestra esta ayuda

Variables de entorno opcionales:
  ARDUINO_BOARD             Placa por defecto si no se pasa -b

Ejemplos:
  build.sh                           # Compila el .ino actual (detecta placa o usa uno)
  build.sh -b esp32 ./mi_sketch      # Compila ./mi_sketch para ESP32
  build.sh -b nano-old -c            # Compilación limpia para Arduino Nano (old bootloader)
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -b|--board)
            BOARD="${2:-}"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -e|--export)
            EXPORT=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            EXTRA_ARGS+=("$@")
            break
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

CMD=(arduino-cli compile)
[ -n "$FQBN" ] && CMD+=(--fqbn "$FQBN")
[ "$CLEAN" = true ] && CMD+=(--clean)
[ "$EXPORT" = true ] && CMD+=(--export-binaries)
[ "${#EXTRA_ARGS[@]}" -gt 0 ] && CMD+=("${EXTRA_ARGS[@]}")
CMD+=("$SKETCH_DIR")

echo "[*] Compilando '$SKETCH_DIR'${FQBN:+ (placa: $FQBN)}..."
"${CMD[@]}"
echo "[✓] Compilación exitosa."
