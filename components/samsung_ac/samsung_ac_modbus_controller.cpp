#include "samsung_ac_modbus_controller.h"
#include "samsung_ac_modbus_sensor.h"
#include "samsung_ac_modbus_switch.h"
#include "samsung_ac_modbus_number.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include <cmath>

namespace esphome
{
  namespace samsung_ac
  {
    static const char *const MODBUS_TAG = "samsung_ac.modbus_controller";

    void Samsung_AC_Modbus_Controller::setup()
    {
      ESP_LOGCONFIG(MODBUS_TAG, "Setting up Samsung AC Modbus Controller...");
      ESP_LOGCONFIG(MODBUS_TAG, "  Initial component counts - Sensors: %d, Switches: %d, Numbers: %d", 
                    sensors_.size(), switches_.size(), numbers_.size());
      
      if (samsung_ac_ == nullptr)
      {
        ESP_LOGE(MODBUS_TAG, "Samsung AC component not set! Please configure samsung_ac_id.");
        return;
      }

      // Register all modbus components with the register map
      for (auto *sensor : sensors_)
      {
        auto key = std::make_pair(sensor->get_device_address(), sensor->get_nasa_address());
        register_map_[key].push_back(sensor);
        ESP_LOGCONFIG(MODBUS_TAG, "Setup: Registered sensor for device %s, NASA address 0x%04X", 
                     sensor->get_device_address().c_str(), sensor->get_nasa_address());
      }

      for (auto *switch_ : switches_)
      {
        auto key = std::make_pair(switch_->get_device_address(), switch_->get_nasa_address());
        register_map_[key].push_back(switch_);
        ESP_LOGCONFIG(MODBUS_TAG, "Setup: Registered switch for device %s, NASA address 0x%04X", 
                     switch_->get_device_address().c_str(), switch_->get_nasa_address());
      }

      for (auto *number : numbers_)
      {
        auto key = std::make_pair(number->get_device_address(), number->get_nasa_address());
        register_map_[key].push_back(number);
        ESP_LOGCONFIG(MODBUS_TAG, "Setup: Registered number for device %s, NASA address 0x%04X", 
                     number->get_device_address().c_str(), number->get_nasa_address());
      }

      // Register this controller with the Samsung AC component
      samsung_ac_->register_modbus_controller(this);

      ESP_LOGCONFIG(MODBUS_TAG, "Samsung AC Modbus Controller setup complete. Registered %d components.", 
                    register_map_.size());
    }

    void Samsung_AC_Modbus_Controller::update()
    {
      // The actual data updates come from the Samsung AC protocol processing
      // This method is called periodically but the real work happens in on_nasa_message
      ESP_LOGVV(MODBUS_TAG, "Modbus controller update - waiting for NASA messages");
    }

    void Samsung_AC_Modbus_Controller::dump_config()
    {
      ESP_LOGCONFIG(MODBUS_TAG, "Samsung AC Modbus Controller:");
      ESP_LOGCONFIG(MODBUS_TAG, "  Registered components: %d", register_map_.size());
      ESP_LOGCONFIG(MODBUS_TAG, "  Sensors: %d", sensors_.size());
      ESP_LOGCONFIG(MODBUS_TAG, "  Switches: %d", switches_.size());
      ESP_LOGCONFIG(MODBUS_TAG, "  Numbers: %d", numbers_.size());
      
      if (samsung_ac_ == nullptr)
      {
        ESP_LOGCONFIG(MODBUS_TAG, "  Samsung AC: NOT SET");
      }
      else
      {
        ESP_LOGCONFIG(MODBUS_TAG, "  Samsung AC: Connected");
      }
    }

    void Samsung_AC_Modbus_Controller::register_sensor(Samsung_AC_Modbus_Sensor *sensor)
    {
      sensors_.push_back(sensor);
      sensor->set_modbus_controller(this);
      ESP_LOGCONFIG(MODBUS_TAG, "REGISTRATION: Added sensor to controller - total sensors: %d", sensors_.size());
      ESP_LOGD(MODBUS_TAG, "Registered sensor: %s (device: %s, address: 0x%04X)", 
               sensor->get_name().c_str(), 
               sensor->get_device_address().c_str(), 
               sensor->get_register_address());
    }

    void Samsung_AC_Modbus_Controller::register_switch(Samsung_AC_Modbus_Switch *switch_)
    {
      switches_.push_back(switch_);
      switch_->set_modbus_controller(this);
      ESP_LOGCONFIG(MODBUS_TAG, "REGISTRATION: Added switch to controller - total switches: %d", switches_.size());
      ESP_LOGD(MODBUS_TAG, "Registered switch: %s (device: %s, address: 0x%04X)", 
               switch_->get_name().c_str(), 
               switch_->get_device_address().c_str(), 
               switch_->get_register_address());
    }

    void Samsung_AC_Modbus_Controller::register_number(Samsung_AC_Modbus_Number *number)
    {
      numbers_.push_back(number);
      number->set_modbus_controller(this);
      ESP_LOGCONFIG(MODBUS_TAG, "REGISTRATION: Added number to controller - total numbers: %d", numbers_.size());
      ESP_LOGD(MODBUS_TAG, "Registered number: %s (device: %s, address: 0x%04X)", 
               number->get_name().c_str(), 
               number->get_device_address().c_str(), 
               number->get_register_address());
    }

    // TODO: Implement Samsung_AC_Modbus_Select when needed
    // void Samsung_AC_Modbus_Controller::register_select(Samsung_AC_Modbus_Select *select)
    // {
    //   selects_.push_back(select);
    //   select->set_modbus_controller(this);
    // }

    void Samsung_AC_Modbus_Controller::on_nasa_message(const std::string& device_address, 
                                                       uint16_t message_number, 
                                                       float value)
    {
      auto key = std::make_pair(device_address, message_number);
      auto it = register_map_.find(key);
      
      if (it == register_map_.end())
      {
        ESP_LOGVV(MODBUS_TAG, "No modbus components registered for device %s, NASA address 0x%04X", 
                  device_address.c_str(), message_number);
        return;
      }

      ESP_LOGD(MODBUS_TAG, "Processing NASA message: device=%s, address=0x%04X, value=%.2f", 
               device_address.c_str(), message_number, value);

      // Process the message for all registered components
      for (auto *item : it->second)
      {
        float processed_value = apply_register_config(value, item->get_config());
        item->process_data(processed_value);
        ESP_LOGVV(MODBUS_TAG, "Updated component: raw=%.2f, processed=%.2f", value, processed_value);
      }
    }

    void Samsung_AC_Modbus_Controller::write_register(const std::string& device_address, 
                                                      uint16_t message_number, 
                                                      float value)
    {
      if (samsung_ac_ == nullptr)
      {
        ESP_LOGE(MODBUS_TAG, "Cannot write register - Samsung AC component not available");
        return;
      }

      Samsung_AC_Device *device = get_device(device_address);
      if (device == nullptr)
      {
        ESP_LOGE(MODBUS_TAG, "Cannot write register - device %s not found", device_address.c_str());
        return;
      }

      ESP_LOGD(MODBUS_TAG, "Writing register: device=%s, address=0x%04X, value=%.2f", 
               device_address.c_str(), message_number, value);

      // Create a protocol request to send the value
      ProtocolRequest request;
      
      // Map common NASA message numbers to protocol request fields
      switch (message_number)
      {
        case 0x4000: // Power
          request.power = (value > 0.5f);
          break;
        case 0x4001: // Mode  
          request.mode = static_cast<Mode>(static_cast<int>(value));
          break;
        case 0x4201: // Target temperature
          request.target_temp = value;
          break;
        case 0x4006: // Fan mode
          request.fan_mode = static_cast<FanMode>(static_cast<int>(value));
          break;
        default:
          // For custom addresses, we need to use a different approach
          ESP_LOGW(MODBUS_TAG, "Direct register write for address 0x%04X not implemented, value=%.2f", 
                   message_number, value);
          return;
      }

      device->publish_request(request);
      ESP_LOGD(MODBUS_TAG, "Register write request sent");
    }

    bool Samsung_AC_Modbus_Controller::validate_device_address(const std::string& address)
    {
      return get_device(address) != nullptr;
    }

    Samsung_AC_Device* Samsung_AC_Modbus_Controller::get_device(const std::string& address)
    {
      if (samsung_ac_ == nullptr)
        return nullptr;
      
      return samsung_ac_->find_device(address);
    }

    float Samsung_AC_Modbus_Controller::convert_from_modbus(uint32_t raw_value, ModbusValueType value_type)
    {
      switch (value_type)
      {
        case ModbusValueType::U_WORD:
          return static_cast<float>(static_cast<uint16_t>(raw_value));
        case ModbusValueType::S_WORD:
          return static_cast<float>(static_cast<int16_t>(raw_value));
        case ModbusValueType::U_DWORD:
          return static_cast<float>(raw_value);
        case ModbusValueType::S_DWORD:
          return static_cast<float>(static_cast<int32_t>(raw_value));
        case ModbusValueType::FP32:
        {
          float result;
          memcpy(&result, &raw_value, sizeof(float));
          return result;
        }
        default:
          ESP_LOGW(MODBUS_TAG, "Unsupported value type: %d", static_cast<int>(value_type));
          return static_cast<float>(raw_value);
      }
    }

    uint32_t Samsung_AC_Modbus_Controller::convert_to_modbus(float value, ModbusValueType value_type)
    {
      switch (value_type)
      {
        case ModbusValueType::U_WORD:
          return static_cast<uint32_t>(static_cast<uint16_t>(value));
        case ModbusValueType::S_WORD:
          return static_cast<uint32_t>(static_cast<uint16_t>(static_cast<int16_t>(value)));
        case ModbusValueType::U_DWORD:
          return static_cast<uint32_t>(value);
        case ModbusValueType::S_DWORD:
          return static_cast<uint32_t>(static_cast<int32_t>(value));
        case ModbusValueType::FP32:
        {
          uint32_t result;
          memcpy(&result, &value, sizeof(uint32_t));
          return result;
        }
        default:
          ESP_LOGW(MODBUS_TAG, "Unsupported value type: %d", static_cast<int>(value_type));
          return static_cast<uint32_t>(value);
      }
    }

    float Samsung_AC_Modbus_Controller::apply_register_config(float raw_value, const ModbusRegisterConfig& config)
    {
      float result = raw_value;
      
      // Apply bitmask if specified
      if (config.bitmask != 0xFFFFFFFF)
      {
        uint32_t int_value = static_cast<uint32_t>(raw_value);
        int_value &= config.bitmask;
        result = static_cast<float>(int_value);
      }
      
      // Apply multiplier and offset
      result = (result * config.multiplier) + config.offset;
      
      return result;
    }

    float Samsung_AC_Modbus_Controller::unapply_register_config(float processed_value, const ModbusRegisterConfig& config)
    {
      // Reverse the transformations applied in apply_register_config
      float result = (processed_value - config.offset) / config.multiplier;
      
      // Bitmask is not reversed since it's typically applied only on read
      
      return result;
    }

  } // namespace samsung_ac
} // namespace esphome 