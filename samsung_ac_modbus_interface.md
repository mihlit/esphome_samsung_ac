# Samsung AC Modbus Controller Interface

This interface layer allows you to use Samsung AC devices as if they were modbus devices, where modbus register addresses map to NASA message addresses. This provides a familiar modbus interface for systems that already work with modbus controllers.

## Overview

The Samsung AC Modbus Controller creates a bridge between:
- **Standard ESPHome modbus components** (sensor, switch, number)
- **Samsung AC NASA protocol** (message-based communication)

### Key Features

- **Address Mapping**: Modbus register addresses directly map to NASA message numbers
- **Device Targeting**: Support for multiple Samsung AC devices using device addresses
- **Standard Interface**: Compatible with existing modbus tooling and configurations
- **Real-time Updates**: Automatic updates when NASA messages are received
- **Bi-directional**: Both reading and writing operations supported

## Architecture

```
┌─────────────────┐    ┌──────────────────────┐    ┌─────────────────┐
│ Modbus Sensor   │    │ Samsung AC Modbus    │    │ Samsung AC      │
│ Switch, Number  │◄──►│ Controller           │◄──►│ Protocol        │
│ (ESPHome)       │    │ (Bridge Layer)       │    │ (NASA Messages) │
└─────────────────┘    └──────────────────────┘    └─────────────────┘
        ▲                        ▲                          ▲
        │                        │                          │
 Modbus Register            Address Mapping            NASA Message
   Address                  & Type Conversion            Number
 (e.g., 0x4203)              (0x4203 → 0x4203)        (e.g., 0x4203)
```

## Components

### Samsung AC Modbus Controller

The main controller component that manages the bridge between modbus and Samsung AC protocols.

```yaml
samsung_ac_modbus:
  id: modbus_controller
  samsung_ac_id: samsung_ac_main
  update_interval: 30s
```

### Supported Components

#### Sensors (Read-only)
Read values from Samsung AC devices using NASA message addresses:

```yaml
sensor:
  - platform: samsung_ac_modbus
    name: "Room Temperature"
    samsung_ac_modbus_controller_id: modbus_controller
    register_address: 0x4203  # NASA message number
    device_address: "20.00.00"  # Samsung AC device address
    register_type: "read"
    value_type: "S_WORD"
    multiplier: 0.1
```

#### Switches (On/Off Control)
Control boolean values like power on/off:

```yaml
switch:
  - platform: samsung_ac_modbus
    name: "AC Power"
    samsung_ac_modbus_controller_id: modbus_controller
    register_address: 0x4000  # Power control
    device_address: "20.00.00"
    register_type: "coil"
    value_type: "U_WORD"
```

#### Numbers (Numeric Control)
Set numeric values like temperature or mode:

```yaml
number:
  - platform: samsung_ac_modbus
    name: "Target Temperature"
    samsung_ac_modbus_controller_id: modbus_controller
    register_address: 0x4201  # Target temperature
    device_address: "20.00.00"
    register_type: "holding"
    value_type: "S_WORD"
    multiplier: 10.0  # Convert to NASA format
    min_value: 16
    max_value: 30
```

## Configuration Reference

### Register Types

| Type | Description | Usage |
|------|-------------|-------|
| `coil` | 1-bit registers (ON/OFF) | Switch controls |
| `discrete_input` | Read-only coils | Status indicators |
| `holding` | 16-bit read/write registers | Most controls |
| `read` | 16-bit read-only registers | Most sensors |

### Value Types

| Type | Description | Range |
|------|-------------|-------|
| `U_WORD` | Unsigned 16-bit | 0 to 65535 |
| `S_WORD` | Signed 16-bit | -32768 to 32767 |
| `U_DWORD` | Unsigned 32-bit | 0 to 4294967295 |
| `S_DWORD` | Signed 32-bit | -2147483648 to 2147483647 |
| `FP32` | 32-bit floating point | IEEE 754 format |

### Common NASA Message Addresses

#### Device Control
| Address | Description | Type | Units |
|---------|-------------|------|-------|
| `0x4000` | Power On/Off | Bool | - |
| `0x4001` | Operation Mode | Enum | 0=Auto, 1=Cool, 2=Dry, 3=Fan, 4=Heat |
| `0x4006` | Fan Mode | Enum | Various speed settings |
| `0x4011` | Vertical Swing | Bool | - |
| `0x407e` | Horizontal Swing | Bool | - |
| `0x4060` | Alt Mode (Windfree, etc.) | Enum | Device specific |

#### Temperature Readings
| Address | Description | Type | Units |
|---------|-------------|------|-------|
| `0x4201` | Target Temperature | S_WORD | 0.1°C |
| `0x4203` | Room Temperature | S_WORD | 0.1°C |
| `0x4237` | Water Tank Temperature | S_WORD | 0.1°C |
| `0x8204` | Outdoor Temperature | S_WORD | 0.1°C |

#### Sensors
| Address | Description | Type | Units |
|---------|-------------|------|-------|
| `0x4038` | Humidity | U_WORD | % |
| `0x8413` | Power Consumption | U_WORD | W |
| `0x8414` | Energy Consumption | U_WORD | kWh |
| `0x4427` | Energy Produced | U_WORD | kWh |

### Device Addresses

Samsung AC devices use specific address formats:

- **Indoor Units**: `"20.00.00"`, `"20.00.01"`, `"20.00.02"`, etc.
- **Outdoor Unit**: `"10.00.00"`
- **Other Devices**: Various formats depending on device type

> **Note**: Check your ESPHome logs after startup to see discovered device addresses in the "Discovered devices:" section.

## Data Conversion

### Temperature Values
Most temperature values in NASA protocol use 0.1°C units:
- **Reading**: Use `multiplier: 0.1` to convert to Celsius
- **Writing**: Use `multiplier: 10.0` to convert from Celsius

### Boolean Values
- **0** = False/Off
- **1** = True/On
- Use bitmask if multiple boolean values are packed in one register

### Enum Values
Mode and other enumerated values use specific numeric codes. Refer to the Samsung AC documentation for specific mappings.

## Advanced Configuration

### Bitmask Support
Extract specific bits from register values:

```yaml
sensor:
  - platform: samsung_ac_modbus
    name: "Status Bit 3"
    register_address: 0x4100
    device_address: "20.00.00"
    bitmask: 0x0008  # Extract bit 3
```

### Offset and Scaling
Apply mathematical transformations:

```yaml
sensor:
  - platform: samsung_ac_modbus
    name: "Adjusted Temperature"
    register_address: 0x4203
    device_address: "20.00.00"
    multiplier: 0.1  # Scale
    offset: -2.0     # Adjust by -2°C
```

## Troubleshooting

### Common Issues

1. **Device not found**
   - Check device address in ESPHome logs
   - Ensure Samsung AC component is properly configured
   - Verify device is powered on and communicating

2. **Values not updating**
   - Check if NASA messages are being received (enable debug logging)
   - Verify register address is correct
   - Ensure device supports the requested message

3. **Incorrect values**
   - Check value type (signed vs unsigned)
   - Verify multiplier and offset settings
   - Confirm NASA message format for your device

### Debug Logging

Enable debug logging to see message flow:

```yaml
logger:
  level: DEBUG
  logs:
    samsung_ac.modbus_controller: DEBUG
    samsung_ac.modbus_sensor: DEBUG
```

### Testing

Use the number component to test write operations:

```yaml
number:
  - platform: samsung_ac_modbus
    name: "Test Register"
    register_address: 0x4000  # Safe register for testing
    device_address: "20.00.00"
    register_type: "holding"
    value_type: "U_WORD"
    min_value: 0
    max_value: 1
```

## Integration Examples

### Home Assistant
The modbus interface works seamlessly with Home Assistant's modbus integrations and dashboards.

### Node-RED
Use standard modbus nodes to interact with Samsung AC devices through ESPHome.

### SCADA Systems
Industrial systems can treat Samsung AC units as standard modbus devices.

## Limitations

1. **Write Operations**: Limited to common control messages (power, temperature, mode)
2. **Custom Messages**: May require adding specific support for proprietary message types
3. **Error Handling**: NASA protocol errors are not directly mapped to modbus errors
4. **Performance**: Updates depend on NASA message frequency from devices

## Contributing

To add support for additional NASA messages:

1. Add the message number to the write_register switch statement
2. Map to appropriate ProtocolRequest fields
3. Test with your specific Samsung AC model
4. Submit a pull request with documentation updates

---

For more information about the underlying Samsung AC protocol, see the [main project documentation](readme.md). 