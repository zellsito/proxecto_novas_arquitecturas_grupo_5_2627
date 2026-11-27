#!/usr/bin/env bash
# ==============================================================================
# Funciones compartidas para build.sh, flash.sh y monitor.sh (arduino-cli)
# ==============================================================================

require_arduino_cli() {
    if ! command -v arduino-cli >/dev/null 2>&1; then
        echo "[!] No se encontró 'arduino-cli' en el PATH."
        echo "    Verificá que ~/.local/bin esté en tu PATH o instalalo con:"
        echo "    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh"
        exit 1
    fi
}

# Convierte alias cortos en FQBN completos de arduino-cli
resolve_fqbn() {
    local input="${1:-}"
    case "${input,,}" in
        "")           echo "" ;;
        uno)          echo "arduino:avr:uno" ;;
        nano)         echo "arduino:avr:nano" ;;
        nano-old)     echo "arduino:avr:nano:cpu=atmega328old" ;;
        mega)         echo "arduino:avr:mega" ;;
        leonardo|leo) echo "arduino:avr:leonardo" ;;
        micro)        echo "arduino:avr:micro" ;;
        esp32)        echo "esp32:esp32:esp32" ;;
        esp32s3)      echo "esp32:esp32:esp32s3" ;;
        esp32c3)      echo "esp32:esp32:esp32c3" ;;
        *)            echo "$input" ;;
    esac
}

# Resuelve la carpeta del sketch (.ino)
resolve_sketch() {
    local target="${1:-}"
    if [ -n "$target" ]; then
        if [ -f "$target" ]; then
            dirname "$target"
            return 0
        elif [ -d "$target" ]; then
            echo "$target"
            return 0
        else
            echo "[!] No se encontró el sketch: $target" >&2
            return 1
        fi
    fi

    # Si hay un .ino en el directorio actual, usar "."
    if compgen -G "*.ino" >/dev/null 2>&1; then
        echo "."
        return 0
    fi

    # Buscar hasta 2 niveles hacia abajo
    local found
    mapfile -t found < <(find . -maxdepth 2 -name "*.ino" 2>/dev/null | sort)
    if [ "${#found[@]}" -eq 1 ]; then
        dirname "${found[0]}"
        return 0
    elif [ "${#found[@]}" -gt 1 ]; then
        if [ -t 0 ]; then
            echo "[i] Se encontraron varios sketches:" >&2
            local i choice
            for i in "${!found[@]}"; do
                printf "  [%d] %s\n" "$((i + 1))" "${found[$i]}" >&2
            done
            read -rp "Elegí un número (1-${#found[@]}): " choice
            if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#found[@]}" ]; then
                dirname "${found[$((choice - 1))]}"
                return 0
            fi
            echo "[!] Selección inválida." >&2
            return 1
        else
            echo "[!] Hay múltiples sketches .ino. Especificá la ruta." >&2
            printf '    %s\n' "${found[@]}" >&2
            return 1
        fi
    fi

    echo "[!] No se encontró ningún archivo .ino en el directorio actual ni subcarpetas." >&2
    return 1
}

# Detecta el FQBN automáticamente si hay una placa conectada o usa default
detect_fqbn() {
    local sketch_dir="$1"
    local explicit="${2:-${ARDUINO_BOARD:-}}"

    if [ -n "$explicit" ]; then
        resolve_fqbn "$explicit"
        return 0
    fi

    # Si existe sketch.yaml con default_fqbn, dejar que arduino-cli lo use
    if [ -f "$sketch_dir/sketch.yaml" ] && grep -qE '^\s*default_fqbn:' "$sketch_dir/sketch.yaml"; then
        echo ""
        return 0
    fi

    # Intentar autodetectar desde la placa conectada
    local detected
    detected=$(arduino-cli board list 2>/dev/null | awk 'NR>1 && $1 ~ /^\/dev\/tty/ {for(i=1;i<=NF;i++) if ($i ~ /^[a-zA-Z0-9_-]+:[a-zA-Z0-9_-]+:[a-zA-Z0-9_-]+/) {print $i; exit}}' || true)
    if [ -n "$detected" ]; then
        echo "[i] Placa detectada automáticamente: $detected" >&2
        echo "$detected"
        return 0
    fi

    echo "[i] No se indicó placa (-b) ni se detectó FQBN; usando por defecto: arduino:avr:uno" >&2
    echo "arduino:avr:uno"
}

# Detecta puerto serie /dev/ttyACM* o /dev/ttyUSB*
detect_port() {
    local explicit="${1:-${ARDUINO_PORT:-}}"
    if [ -n "$explicit" ]; then
        if [ ! -e "$explicit" ]; then
            echo "[!] El puerto '$explicit' no existe." >&2
            return 1
        fi
        echo "$explicit"
        return 0
    fi

    local ports=()
    local p
    for p in /dev/ttyACM* /dev/ttyUSB*; do
        [ -e "$p" ] && ports+=("$p")
    done

    if [ "${#ports[@]}" -eq 0 ]; then
        echo "[!] No se encontró ningún puerto serie (/dev/ttyACM* o /dev/ttyUSB*)." >&2
        echo "    ¿Montaste el USB en WSL? Corré primero: mount.sh" >&2
        return 1
    elif [ "${#ports[@]}" -eq 1 ]; then
        echo "${ports[0]}"
        return 0
    elif [ -t 0 ]; then
        echo "[i] Se detectaron varios puertos serie:" >&2
        local i choice
        for i in "${!ports[@]}"; do
            printf "  [%d] %s\n" "$((i + 1))" "${ports[$i]}" >&2
        done
        read -rp "Elegí un puerto (1-${#ports[@]}): " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#ports[@]}" ]; then
            echo "${ports[$((choice - 1))]}"
            return 0
        fi
        echo "[!] Selección inválida." >&2
        return 1
    else
        echo "[!] Hay múltiples puertos serie. Especificá uno con -p:" >&2
        printf '    %s\n' "${ports[@]}" >&2
        return 1
    fi
}

check_port_permissions() {
    local port="$1"
    if [ ! -r "$port" ] || [ ! -w "$port" ]; then
        echo "[!] Sin permisos de lectura/escritura en $port." >&2
        echo "    Agregá tu usuario al grupo dialout y reiniciá WSL:" >&2
        echo "    sudo usermod -aG dialout $USER" >&2
        return 1
    fi
}
