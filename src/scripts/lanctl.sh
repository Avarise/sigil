#!/bin/bash

# Variables
WIFI_INTERFACE="wlan0"            # WiFi interface
ETH_INTERFACE="eno1"              # Ethernet interface
IP_RANGE_START="10.0.0.2"         # DHCP range start
IP_RANGE_END="10.0.0.254"         # DHCP range end
GATEWAY_IP="10.0.0.1"             # Gateway IP
RESERVED_MACS=(                   # Reserved MAC addresses
    "d4:93:90:17:c1:6b;10.0.0.100"
    "11:22:33:44:55:66;10.0.0.169"
)

# Function to install dependencies
install_dependencies() {
    echo "Installing dependencies..."
    sudo pacman -Syyu dnsmasq iptables
    sudo systemctl start dnsmasq
    sudo systemctl start iptables
    echo "Dependencies installed and enabled."
}

# Function to configure NAT passthrough
setup_passthrough() {
    echo "Setting up NAT passthrough..."
    sudo sysctl -w net.ipv4.ip_forward=1
    sudo iptables -t nat -A POSTROUTING -o "$WIFI_INTERFACE" -j MASQUERADE
    sudo iptables -A FORWARD -i "$ETH_INTERFACE" -o "$WIFI_INTERFACE" -j ACCEPT
    sudo iptables -A FORWARD -i "$WIFI_INTERFACE" -o "$ETH_INTERFACE" -j ACCEPT
    sudo iptables-save > /etc/iptables/iptables.rules
    echo "NAT passthrough configured."
}

# Function to configure direct LAN
setup_direct() {
    echo "Setting up direct LAN configuration..."

    # Configure static IP for the Ethernet interface
    sudo ip addr flush dev "$ETH_INTERFACE"
    sudo ip addr add "$GATEWAY_IP/24" dev "$ETH_INTERFACE"
    sudo ip link set "$ETH_INTERFACE" up

    # Configure dnsmasq
    echo "interface=$ETH_INTERFACE" > /etc/dnsmasq.conf
    echo "bind-interfaces" >> /etc/dnsmasq.conf
    echo "dhcp-range=$IP_RANGE_START,$IP_RANGE_END,24h" >> /etc/dnsmasq.conf
    for entry in "${RESERVED_MACS[@]}"; do
        IFS=';' read -r mac addr <<< "$entry"
        echo "dhcp-host=$mac,$addr" >> /etc/dnsmasq.conf
    done

    sudo systemctl restart dnsmasq
    echo "Direct LAN configuration completed."
}

# Display help
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --install-dependencies    Install necessary packages and enable services"
    echo "  --passthrough             Enable NAT passthrough for Ethernet"
    echo "  --direct                  Configure direct LAN with DHCP"
    echo "  --help                    Display this help message"
}

# Parse command-line arguments
case "$1" in
    --install-dependencies)
        install_dependencies
        ;;
    --passthrough)
        setup_passthrough
        ;;
    --direct)
        setup_direct
        ;;
    --help|-h)
        show_help
        ;;
    *)
        echo "Invalid option. Use --help for usage information."
        exit 1
        ;;
esac

exit 0
