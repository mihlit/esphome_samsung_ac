#pragma once

#include "esphome/components/sensor/sensor.h"
#include "samsung_ac_modbus_controller.h"

namespace esphome
{
  namespace samsung_ac
  {
    class Samsung_AC_Modbus_Controller;

    /**
     * Samsung AC Modbus Sensor
     * 
     * This sensor reads values from Samsung AC devices using modbus register addresses
     * that map to NASA message numbers. It provides the same interface as standard
     * ESPHome modbus sensors but works with Samsung AC protocol underneath.
     * 
     * Configuration:
     * - register_address: Modbus register address (maps to NASA message number)
     * - device_address: Samsung AC device address (e.g., "20.00.00")
     * - register_type: Type of modbus register (holding, read, coil, discrete_input)
     * - value_type: Data type (U_WORD, S_WORD, U_DWORD, S_DWORD, FP32, etc.)
     * - multiplier: Optional multiplier for scaling
     * - offset: Optional offset for adjustment
     * - bitmask: Optional bitmask for extracting bits
     */
    class Samsung_AC_Modbus_Sensor : public sensor::Sensor, public ModbusRegisterItem
    {
    public:
      Samsung_AC_Modbus_Sensor(ModbusRegisterConfig config) 
        : ModbusRegisterItem(config) {}

      void setup() override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::DATA; }

      // Set the modbus controller that manages this sensor
      void set_modbus_controller(Samsung_AC_Modbus_Controller *controller) 
      { 
        modbus_controller_ = controller; 
      }

      // ModbusRegisterItem implementation
      void process_data(float value) override;

      // Configuration getters
      uint16_t get_register_address() const { return config_.address; }
      const std::string& get_device_address() const { return config_.device_address; }
      ModbusRegisterType get_register_type() const { return config_.register_type; }
      ModbusValueType get_value_type() const { return config_.value_type; }

      // Configuration setters for YAML parsing
      void set_register_address(uint16_t address) { config_.address = address; }
      void set_device_address(const std::string& address) { config_.device_address = address; }
      void set_register_type(ModbusRegisterType type) { config_.register_type = type; }
      void set_value_type(ModbusValueType type) { config_.value_type = type; }
      void set_multiplier(float multiplier) { config_.multiplier = multiplier; }
      void set_offset(float offset) { config_.offset = offset; }
      void set_bitmask(uint32_t bitmask) { config_.bitmask = bitmask; }

    protected:
      Samsung_AC_Modbus_Controller *modbus_controller_{nullptr};
    };

  } // namespace samsung_ac
} // namespace esphome 