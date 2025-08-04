#include "samsung_ac_modbus_number.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace samsung_ac
  {
    static const char *const MODBUS_NUMBER_TAG = "samsung_ac.modbus_number";

    void Samsung_AC_Modbus_Number::setup()
    {
      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "Setting up Samsung AC Modbus Number...");
      
      if (modbus_controller_ == nullptr)
      {
        ESP_LOGE(MODBUS_NUMBER_TAG, "Modbus controller not set!");
        return;
      }

      // Validate configuration
      if (config_.device_address.empty())
      {
        ESP_LOGE(MODBUS_NUMBER_TAG, "Device address not set!");
        return;
      }

      if (!modbus_controller_->validate_device_address(config_.device_address))
      {
        ESP_LOGE(MODBUS_NUMBER_TAG, "Invalid device address: %s", config_.device_address.c_str());
        return;
      }

      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "Samsung AC Modbus Number setup complete");
    }

    void Samsung_AC_Modbus_Number::dump_config()
    {
      LOG_NUMBER("", "Samsung AC Modbus Number", this);
      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Device Address: %s", config_.device_address.c_str());
      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Register Address: 0x%04X (NASA message)", config_.address);

      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Value Type: %d", static_cast<int>(config_.value_type));
      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Multiplier: %.3f", config_.multiplier);
      ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Offset: %.3f", config_.offset);
      
      if (config_.bitmask != 0xFFFFFFFF)
      {
        ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Bitmask: 0x%08X", config_.bitmask);
      }

      if (modbus_controller_ == nullptr)
      {
        ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Modbus Controller: NOT SET");
      }
      else
      {
        ESP_LOGCONFIG(MODBUS_NUMBER_TAG, "  Modbus Controller: Connected");
      }
    }

    void Samsung_AC_Modbus_Number::process_data(float value)
    {
      ESP_LOGD(MODBUS_NUMBER_TAG, "Processing data for number '%s': %.2f", 
               this->get_name().c_str(), value);

      // Check if the value is valid
      if (std::isnan(value))
      {
        ESP_LOGW(MODBUS_NUMBER_TAG, "Received NaN value for number '%s'", this->get_name().c_str());
        return;
      }

      // Publish the processed value to Home Assistant
      this->publish_state(value);
      
      ESP_LOGD(MODBUS_NUMBER_TAG, "Published number state: %.2f", value);
    }

    void Samsung_AC_Modbus_Number::write_data(float value)
    {
      ESP_LOGD(MODBUS_NUMBER_TAG, "Writing data for number '%s': %.2f", 
               this->get_name().c_str(), value);

      if (modbus_controller_ == nullptr)
      {
        ESP_LOGE(MODBUS_NUMBER_TAG, "Cannot write data - modbus controller not set");
        return;
      }

      // Send the write request to the modbus controller
      modbus_controller_->write_register(config_.device_address, config_.address, value);
    }

    void Samsung_AC_Modbus_Number::control(float value)
    {
      ESP_LOGD(MODBUS_NUMBER_TAG, "Number '%s' control called: %.2f", 
               this->get_name().c_str(), value);

      // Apply register configuration if needed (reverse transform)
      float adjusted_value = value;
      if (modbus_controller_ != nullptr)
      {
        adjusted_value = modbus_controller_->unapply_register_config(value, config_);
      }
      
      // Write the data
      write_data(adjusted_value);
      
      // Optimistically update the state (will be corrected by feedback if wrong)
      this->publish_state(value);
    }

  } // namespace samsung_ac
} // namespace esphome 