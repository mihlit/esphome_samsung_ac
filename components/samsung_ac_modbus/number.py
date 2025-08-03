import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import *
from . import (
    Samsung_AC_Modbus_Number,
    Samsung_AC_Modbus_Controller,
    MODBUS_COMPONENT_BASE_SCHEMA,
    CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID,
    setup_modbus_component,
    modbus_register_config_struct,
)

CONFIG_SCHEMA = number.number_schema(Samsung_AC_Modbus_Number).extend(
    MODBUS_COMPONENT_BASE_SCHEMA
)


async def to_code(config):
    """Configure the Samsung AC Modbus Number"""
    # Create the register config
    register_config = modbus_register_config_struct(config)
    
    # Create the number with the register config
    var = cg.new_Pvariable(config[CONF_ID], register_config)
    await number.register_number(var, config)
    
    # Get the modbus controller
    controller = await cg.get_variable(config[CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID])
    
    # Setup the modbus component
    await setup_modbus_component(var, config, controller) 