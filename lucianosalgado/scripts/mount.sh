#!/usr/bin/env bash
# ==============================================================================
# Script genérico para montar / desmontar placas USB (Arduino, nRF, ESP32, etc.)
# en WSL usando usbipd-win.
# ==============================================================================
set -euo pipefail

DEFAULT_PATTERN="1366:|J-Link|nRF|2341:|2a03:|Arduino|Genuino|1a86:|CH340|CH341|10c4:|CP210|0403:|FTDI|FT232|067b:|PL2303|303a:|Espressif|0483:|ST-Link|USB[- ]?Serial|UART|COM[0-9]+"

ACTION="attach"
AUTO_ATTACH=true
TARGET=""

usage() {
    cat <<'EOF'
Uso: mount.sh [opciones] [busid|filtro]

Por defecto monta con auto-reconexión (--auto-attach) y queda en primer plano
reconectando automáticamente si la placa se reinicia o se desconecta.

Opciones:
  -1, --once     Monta una sola vez y termina (sin quedar reconectando)
  -a, --auto     Usa auto-reconexión (comportamiento por defecto)
  -d, --detach   Desmonta el dispositivo de WSL y lo devuelve a Windows
  -l, --list     Lista los dispositivos USB conectados en Windows
  -h, --help     Muestra esta ayuda

Ejemplos:
  mount.sh              # Detecta automáticamente y queda reconectando
  mount.sh -1           # Monta una sola vez y vuelve a la terminal
  mount.sh arduino      # Filtra por nombre o VID:PID
  mount.sh 1-4          # Monta directamente el BUSID 1-4
  mount.sh -d           # Desmonta el dispositivo detectado
  mount.sh -d 1-4       # Desmonta el BUSID 1-4
EOF
}

show_ports() {
    echo "Puertos serie disponibles:"
    shopt -s nullglob
    local ports=(/dev/ttyACM* /dev/ttyUSB*)
    shopt -u nullglob
    if [ "${#ports[@]}" -gt 0 ]; then
        ls -l "${ports[@]}"
    else
        echo "Aún no asignó /dev/ttyACM* o /dev/ttyUSB* (dale un par de segundos)."
    fi
}

check_dialout() {
    if ! id -nG | grep -qw dialout; then
        echo ""
        echo "[!] Tu usuario ($USER) no está en el grupo 'dialout'. Para evitar errores de permisos corré:"
        echo "    sudo usermod -aG dialout $USER"
        echo "    (y reiniciá la sesión de WSL)"
    fi
}

while [ $# -gt 0 ]; do
    case "$1" in
        detach|-d|--detach)
            ACTION="detach"
            ;;
        -a|--auto)
            AUTO_ATTACH=true
            ;;
        -1|--once)
            AUTO_ATTACH=false
            ;;
        list|-l|--list)
            ACTION="list"
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
            TARGET="$1"
            ;;
    esac
    shift
done

if ! command -v usbipd.exe >/dev/null 2>&1; then
    echo "[!] No se encontró 'usbipd.exe'. Instalá usbipd-win en Windows primero."
    exit 1
fi

if [ "$ACTION" = "list" ]; then
    usbipd.exe list < /dev/null | tr -d '\r'
    exit 0
fi

# Obtener solo la sección de dispositivos conectados (líneas que empiezan con BUSID tipo 1-4)
CONNECTED=$(usbipd.exe list < /dev/null 2>/dev/null | tr -d '\r' | grep -E '^[0-9]+-[0-9]+' || true)

if [ -z "$CONNECTED" ]; then
    echo "[!] No se detectaron dispositivos USB conectados en Windows."
    exit 1
fi

BUSID=""
MATCH_LINE=""

if [[ "$TARGET" =~ ^[0-9]+-[0-9]+(\.[0-9]+)*$ ]]; then
    BUSID="$TARGET"
    MATCH_LINE=$(printf '%s\n' "$CONNECTED" | grep -E "^${BUSID}[[:space:]]" | head -n 1 || true)
    if [ -z "$MATCH_LINE" ]; then
        echo "[!] No se encontró el BUSID '$BUSID' entre los dispositivos conectados."
        echo "$CONNECTED"
        exit 1
    fi
else
    PATTERN="${TARGET:-$DEFAULT_PATTERN}"
    MATCHES=$(printf '%s\n' "$CONNECTED" | grep -iE -- "$PATTERN" || true)

    if [ -z "$MATCHES" ]; then
        echo "[!] No se detectó ningún dispositivo que coincida con: $PATTERN"
        echo ""
        echo "Dispositivos conectados en Windows:"
        usbipd.exe list < /dev/null | tr -d '\r'
        echo ""
        echo "Tip: pasá el BUSID (ej: $0 1-4) o una palabra clave (ej: $0 arduino)."
        exit 1
    fi

    mapfile -t LINES <<< "$MATCHES"

    if [ "${#LINES[@]}" -eq 1 ]; then
        MATCH_LINE="${LINES[0]}"
    elif [ -t 0 ]; then
        echo "[i] Se detectaron varios dispositivos:"
        for i in "${!LINES[@]}"; do
            printf "  [%d] %s\n" "$((i + 1))" "${LINES[$i]}"
        done
        read -rp "Elegí un número (1-${#LINES[@]}): " CHOICE
        if ! [[ "$CHOICE" =~ ^[0-9]+$ ]] || [ "$CHOICE" -lt 1 ] || [ "$CHOICE" -gt "${#LINES[@]}" ]; then
            echo "[!] Selección inválida."
            exit 1
        fi
        MATCH_LINE="${LINES[$((CHOICE - 1))]}"
    else
        echo "[!] Hay múltiples dispositivos coincidentes. Especificá el BUSID o un filtro más preciso:"
        printf '%s\n' "${LINES[@]}"
        exit 1
    fi

    BUSID=$(awk '{print $1}' <<< "$MATCH_LINE")
fi

echo "[i] Dispositivo seleccionado: $MATCH_LINE"

if [ "$ACTION" = "detach" ]; then
    echo "[*] Desconectando $BUSID de WSL..."
    usbipd.exe detach --busid "$BUSID" < /dev/null || true
    echo "[✓] Dispositivo devuelto a Windows."
    exit 0
fi

if [[ "$MATCH_LINE" =~ [[:space:]]Attached[[:space:]]*$ ]]; then
    if [ "$AUTO_ATTACH" = true ]; then
        echo "[*] El dispositivo $BUSID ya estaba montado; reenganchando en modo auto-reconexión..."
        usbipd.exe detach --busid "$BUSID" < /dev/null || true
        sleep 0.5
    else
        echo "[✓] El dispositivo $BUSID ya estaba montado en WSL."
        show_ports
        check_dialout
        exit 0
    fi
fi

if [[ "$MATCH_LINE" =~ [[:space:]]Not\ shared[[:space:]]*$ ]]; then
    if command -v powershell.exe >/dev/null 2>&1; then
        echo "[*] El dispositivo $BUSID no está compartido. Solicitando elevación (UAC) en Windows para ejecutar 'usbipd bind'..."
        powershell.exe -NoProfile -Command "Start-Process usbipd -ArgumentList 'bind --busid ${BUSID}' -Verb RunAs -Wait" < /dev/null || true
    else
        echo "[!] El dispositivo $BUSID está en estado 'Not shared'. Abrí PowerShell como Administrador en Windows y corré:"
        echo "    usbipd bind --busid $BUSID"
        exit 1
    fi
fi

if [ "$AUTO_ATTACH" = true ]; then
    check_dialout
    echo "[*] Montando $BUSID a WSL con auto-reconexión (Ctrl+C para detener)..."
    (sleep 2 && show_ports) &
    PORTS_PID=$!
    cleanup_auto() {
        kill "$PORTS_PID" 2>/dev/null || true
        echo ""
        echo "[i] Auto-reconexión detenida. Usá '$0 -d' si querés devolver el dispositivo a Windows."
        exit 0
    }
    trap cleanup_auto INT TERM

    set +e
    usbipd.exe attach --wsl --busid "$BUSID" --auto-attach < /dev/null
    STATUS=$?
    set -e

    kill "$PORTS_PID" 2>/dev/null || true
    trap - INT TERM

    if [ "$STATUS" -ne 0 ]; then
        echo ""
        echo "[!] No se pudo montar el dispositivo $BUSID."
        echo "    Si rechazaste el UAC o sigue en 'Not shared', corré en PowerShell como Administrador:"
        echo "    usbipd bind --busid $BUSID"
        exit "$STATUS"
    fi
    exit 0
fi

echo "[*] Montando $BUSID a WSL..."
if ! usbipd.exe attach --wsl --busid "$BUSID" < /dev/null; then
    echo ""
    echo "[!] No se pudo montar el dispositivo $BUSID."
    echo "    Si rechazaste el UAC o sigue en 'Not shared', corré en PowerShell como Administrador:"
    echo "    usbipd bind --busid $BUSID"
    exit 1
fi

echo "[✓] Dispositivo montado correctamente en WSL."
sleep 1
show_ports
check_dialout
