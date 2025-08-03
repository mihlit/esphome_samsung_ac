#include "samsung_ac_modbus_sensor.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace samsung_ac
  {
    static const char *const MODBUS_SENSOR_TAG = "samsung_ac.modbus_sensor";

    void Samsung_AC_Modbus_Sensor::setup()
    {
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "Setting up Samsung AC Modbus Sensor...");
      
      if (modbus_controller_ == nullptr)
      {
        ESP_LOGE(MODBUS_SENSOR_TAG, "Modbus controller not set!");
        return;
      }

      // Validate configuration
      if (config_.device_address.empty())
      {
        ESP_LOGE(MODBUS_SENSOR_TAG, "Device address not set!");
        return;
      }

      if (!modbus_controller_->validate_device_address(config_.device_address))
      {
        ESP_LOGE(MODBUS_SENSOR_TAG, "Invalid device address: %s", config_.device_address.c_str());
        return;
      }

      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "Samsung AC Modbus Sensor setup complete");
    }

    void Samsung_AC_Modbus_Sensor::dump_config()
    {
      LOG_SENSOR("", "Samsung AC Modbus Sensor", this);
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Device Address: %s", config_.device_address.c_str());
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Register Address: 0x%04X (NASA message)", config_.address);
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Register Type: %d", static_cast<int>(config_.register_type));
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Value Type: %d", static_cast<int>(config_.value_type));
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Multiplier: %.3f", config_.multiplier);
      ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Offset: %.3f", config_.offset);
      
      if (config_.bitmask != 0xFFFFFFFF)
      {
        ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Bitmask: 0x%08X", config_.bitmask);
      }

      if (modbus_controller_ == nullptr)
      {
        ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Modbus Controller: NOT SET");
      }
      else
      {
        ESP_LOGCONFIG(MODBUS_SENSOR_TAG, "  Modbus Controller: Connected");
      }
    }

    void Samsung_AC_Modbus_Sensor::process_data(float value)
    {
      ESP_LOGD(MODBUS_SENSOR_TAG, "Processing data for sensor '%s': %.2f", 
               this->get_name().c_str(), value);

      // Check if the value is valid
      if (std::isnan(value))
      {
        ESP_LOGW(MODBUS_SENSOR_TAG, "Received NaN value for sensor '%s'", this->get_name().c_str());
        return;
      }

      // Publish the processed value to Home Assistant
      this->publish_state(value);
      
      ESP_LOGD(MODBUS_SENSOR_TAG, "Published sensor state: %.2f", value);
    }

  } // namespace samsung_ac
} // namespace esphome 