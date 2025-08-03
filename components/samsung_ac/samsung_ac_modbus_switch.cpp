#include "samsung_ac_modbus_switch.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace samsung_ac
  {
    static const char *const TAG = "samsung_ac.modbus_switch";

    void Samsung_AC_Modbus_Switch::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up Samsung AC Modbus Switch...");
      
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

      ESP_LOGCONFIG(TAG, "Samsung AC Modbus Switch setup complete");
    }

    void Samsung_AC_Modbus_Switch::dump_config()
    {
      LOG_SWITCH("", "Samsung AC Modbus Switch", this);
      ESP_LOGCONFIG(TAG, "  Device Address: %s", config_.device_address.c_str());
      ESP_LOGCONFIG(TAG, "  Register Address: 0x%04X (NASA message)", config_.address);
      ESP_LOGCONFIG(TAG, "  Register Type: %d", static_cast<int>(config_.register_type));
      ESP_LOGCONFIG(TAG, "  Value Type: %d", static_cast<int>(config_.value_type));
      
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

    void Samsung_AC_Modbus_Switch::process_data(float value)
    {
      ESP_LOGD(TAG, "Processing data for switch '%s': %.2f", 
               this->get_name().c_str(), value);

      // Convert float value to boolean state
      bool state = (value > 0.5f);
      
      // Update the switch state without triggering a write
      this->publish_state(state);
      
      ESP_LOGD(TAG, "Published switch state: %s", state ? "ON" : "OFF");
    }

    void Samsung_AC_Modbus_Switch::write_data(float value)
    {
      ESP_LOGD(TAG, "Writing data for switch '%s': %.2f", 
               this->get_name().c_str(), value);

      if (modbus_controller_ == nullptr)
      {
        ESP_LOGE(TAG, "Cannot write data - modbus controller not set");
        return;
      }

      // Send the write request to the modbus controller
      modbus_controller_->write_register(config_.device_address, config_.address, value);
    }

    void Samsung_AC_Modbus_Switch::write_state(bool state)
    {
      ESP_LOGD(TAG, "Switch '%s' write_state called: %s", 
               this->get_name().c_str(), state ? "ON" : "OFF");

      // Convert boolean state to float value for modbus
      float value = state ? 1.0f : 0.0f;
      
      // Apply register configuration if needed
      if (modbus_controller_ != nullptr)
      {
        value = modbus_controller_->unapply_register_config(value, config_);
      }
      
      // Write the data
      write_data(value);
      
      // Optimistically update the state (will be corrected by feedback if wrong)
      this->publish_state(state);
    }

  } // namespace samsung_ac
} // namespace esphome 