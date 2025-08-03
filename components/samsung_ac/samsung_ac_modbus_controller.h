#pragma once

#include <map>
#include <vector>
#include <functional>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "samsung_ac.h"
#include "samsung_ac_device.h"

namespace esphome
{
  namespace samsung_ac
  {
    class Samsung_AC;
    class Samsung_AC_Device;

    // Forward declarations for modbus components
    class Samsung_AC_Modbus_Sensor;
    class Samsung_AC_Modbus_Switch;
    class Samsung_AC_Modbus_Number;
    // class Samsung_AC_Modbus_Select; // TODO: Implement when needed

    enum class ModbusRegisterType : uint8_t
    {
      COIL = 1,
      DISCRETE_INPUT = 2, 
      HOLDING = 3,
      READ = 4
    };

    enum class ModbusValueType : uint8_t
    {
      U_WORD = 1,
      S_WORD = 2,
      U_DWORD = 3,
      S_DWORD = 4,
      U_DWORD_R = 5,
      S_DWORD_R = 6,
      FP32 = 7,
      FP32_R = 8
    };

    struct ModbusRegisterConfig
    {
      uint16_t address;                    // Modbus register address (maps to NASA message number)
      ModbusRegisterType register_type;
      ModbusValueType value_type;
      std::string device_address;          // Samsung AC device address (e.g., "20.00.00")
      float multiplier = 1.0;
      float offset = 0.0;
      uint32_t bitmask = 0xFFFFFFFF;
    };

    // Base class for modbus register items
    class ModbusRegisterItem
    {
    public:
      ModbusRegisterItem(ModbusRegisterConfig config) : config_(config) {}
      virtual ~ModbusRegisterItem() = default;
      
      virtual void process_data(float value) = 0;
      virtual void write_data(float value) {}  // Default implementation for read-only items
      
      const ModbusRegisterConfig& get_config() const { return config_; }
      uint16_t get_nasa_address() const { return config_.address; }
      const std::string& get_device_address() const { return config_.device_address; }

    protected:
      ModbusRegisterConfig config_;
    };

    /**
     * Samsung AC Modbus Controller - Bridge between ESPHome modbus interface and Samsung AC protocol
     * 
     * This class allows using Samsung AC devices as if they were modbus devices, where:
     * - Modbus register addresses map to NASA message numbers
     * - Different register types (holding, read, coil, discrete_input) are supported
     * - Device addressing allows targeting specific Samsung AC units
     * - Standard ESPHome modbus components (sensor, switch, number) can be used
     */
    class Samsung_AC_Modbus_Controller : public PollingComponent
    {
    public:
      Samsung_AC_Modbus_Controller() = default;
      
      void setup() override;
      void update() override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::DATA; }

      // Set the Samsung AC component to bridge to
      void set_samsung_ac(Samsung_AC *samsung_ac) { samsung_ac_ = samsung_ac; }

      // Register modbus components
      void register_sensor(Samsung_AC_Modbus_Sensor *sensor);
      void register_switch(Samsung_AC_Modbus_Switch *switch_);
      void register_number(Samsung_AC_Modbus_Number *number);
      // void register_select(Samsung_AC_Modbus_Select *select); // TODO: Implement when needed

      // Handle incoming data from Samsung AC
      void on_nasa_message(const std::string& device_address, uint16_t message_number, float value);
      
      // Send data to Samsung AC device
      void write_register(const std::string& device_address, uint16_t message_number, float value);

      // Validation helpers
      bool validate_device_address(const std::string& address);
      Samsung_AC_Device* get_device(const std::string& address);

    protected:
      Samsung_AC *samsung_ac_{nullptr};
      
      // Map from NASA message number + device address to modbus components
      std::map<std::pair<std::string, uint16_t>, std::vector<ModbusRegisterItem*>> register_map_;
      
      // Component lists
      std::vector<Samsung_AC_Modbus_Sensor*> sensors_;
      std::vector<Samsung_AC_Modbus_Switch*> switches_;
      std::vector<Samsung_AC_Modbus_Number*> numbers_;
      // std::vector<Samsung_AC_Modbus_Select*> selects_; // TODO: Implement when needed

      // Convert between modbus and internal formats
      float convert_from_modbus(uint32_t raw_value, ModbusValueType value_type);
      uint32_t convert_to_modbus(float value, ModbusValueType value_type);
      
      // Apply register configuration transformations
      float apply_register_config(float raw_value, const ModbusRegisterConfig& config);
      
    public:
      // Public interface for modbus components
      float unapply_register_config(float processed_value, const ModbusRegisterConfig& config);
    };

  } // namespace samsung_ac
} // namespace esphome 