#include "samsung_ac_modbus_sensor.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace samsung_ac
  {
    static const char *const TAG = "samsung_ac.modbus_sensor";

    void Samsung_AC_Modbus_Sensor::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up Samsung AC Modbus Sensor...");
      
      if (modbus_controller_ == nullptr)
      {
        ESP_LOGE(TAG, "Modbus controller not set!");
        this->mark_failed();
        return;
      }

      // Validate configuration
      if (config_.device_address.empty())
      {
        ESP_LOGE(TAG, "Device address not set!");
        this->mark_failed();
        return;
      }

      if (!modbus_controller_->validate_device_address(config_.device_address))
      {
        ESP_LOGE(TAG, "Invalid device address: %s", config_.device_address.c_str());
        this->mark_failed();
        return;
      }

      ESP_LOGCONFIG(TAG, "Samsung AC Modbus Sensor setup complete");
    }

    void Samsung_AC_Modbus_Sensor::dump_config()
    {
      LOG_SENSOR("", "Samsung AC Modbus Sensor", this);
      ESP_LOGCONFIG(TAG, "  Device Address: %s", config_.device_address.c_str());
      ESP_LOGCONFIG(TAG, "  Register Address: 0x%04X (NASA message)", config_.address);
      ESP_LOGCONFIG(TAG, "  Register Type: %d", static_cast<int>(config_.register_type));
      ESP_LOGCONFIG(TAG, "  Value Type: %d", static_cast<int>(config_.value_type));
      ESP_LOGCONFIG(TAG, "  Multiplier: %.3f", config_.multiplier);
      ESP_LOGCONFIG(TAG, "  Offset: %.3f", config_.offset);
      
      if (config_.bitmask != 0xFFFFFFFF)
      {
        ESP_LOGCONFIG(TAG, "  Bitmask: 0x%08X", config_.bitmask);
      }

      if (modbus_controller_ == nullptr)
      {
        ESP_LOGCONFIG(TAG, "  Modbus Controller: NOT SET");
      }
      else
      {
        ESP_LOGCONFIG(TAG, "  Modbus Controller: Connected");
      }
    }

    void Samsung_AC_Modbus_Sensor::process_data(float value)
    {
      ESP_LOGD(TAG, "Processing data for sensor '%s': %.2f", 
               this->get_name().c_str(), value);

      // Check if the value is valid
      if (std::isnan(value))
      {
        ESP_LOGW(TAG, "Received NaN value for sensor '%s'", this->get_name().c_str());
        return;
      }

      // Publish the processed value to Home Assistant
      this->publish_state(value);
      
      ESP_LOGD(TAG, "Published sensor state: %.2f", value);
    }

  } // namespace samsung_ac
} // namespace esphome 