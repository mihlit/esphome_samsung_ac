import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import *
from . import (
    Samsung_AC_Modbus_Switch,
    Samsung_AC_Modbus_Controller,
    MODBUS_COMPONENT_BASE_SCHEMA,
    CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID,
    setup_modbus_component_config,
    modbus_register_config_struct,
)

CONFIG_SCHEMA = switch.switch_schema(Samsung_AC_Modbus_Switch).extend(
    MODBUS_COMPONENT_BASE_SCHEMA
)


async def to_code(config):
    """Configure the Samsung AC Modbus Switch"""
    # Create the register config
    register_config = modbus_register_config_struct(config)
    
    # Create the switch with the register config
    var = cg.new_Pvariable(config[CONF_ID], register_config)
    await switch.register_switch(var, config)
    
    # Get the modbus controller
    controller = await cg.get_variable(config[CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID])
    
    # Setup the modbus component configuration
    await setup_modbus_component_config(var, config)
    
    # Register this switch with the controller
    cg.add(controller.register_switch(var)) 