import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, switch, number
from esphome.const import *

CODEOWNERS = ["@your-github-username"]
DEPENDENCIES = ["samsung_ac"]
AUTO_LOAD = ["sensor", "switch", "number"]

CONF_SAMSUNG_AC_ID = "samsung_ac_id"
CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID = "samsung_ac_modbus_controller_id"
CONF_REGISTER_ADDRESS = "register_address"
CONF_DEVICE_ADDRESS = "device_address"
CONF_REGISTER_TYPE = "register_type"
CONF_VALUE_TYPE = "value_type"
CONF_MULTIPLIER = "multiplier"
CONF_OFFSET = "offset"
CONF_BITMASK = "bitmask"

# Import samsung_ac namespace
samsung_ac_ns = cg.esphome_ns.namespace("samsung_ac")

# Controller class
Samsung_AC_Modbus_Controller = samsung_ac_ns.class_("Samsung_AC_Modbus_Controller", cg.PollingComponent)

# Component classes
Samsung_AC_Modbus_Sensor = samsung_ac_ns.class_("Samsung_AC_Modbus_Sensor", sensor.Sensor)
Samsung_AC_Modbus_Switch = samsung_ac_ns.class_("Samsung_AC_Modbus_Switch", switch.Switch)
Samsung_AC_Modbus_Number = samsung_ac_ns.class_("Samsung_AC_Modbus_Number", number.Number)

# Enums
ModbusRegisterType = samsung_ac_ns.enum("ModbusRegisterType")
REGISTER_TYPES = {
    "coil": ModbusRegisterType.COIL,
    "discrete_input": ModbusRegisterType.DISCRETE_INPUT,
    "holding": ModbusRegisterType.HOLDING,
    "read": ModbusRegisterType.READ,
}

ModbusValueType = samsung_ac_ns.enum("ModbusValueType")
VALUE_TYPES = {
    "U_WORD": ModbusValueType.U_WORD,
    "S_WORD": ModbusValueType.S_WORD,
    "U_DWORD": ModbusValueType.U_DWORD,
    "S_DWORD": ModbusValueType.S_DWORD,
    "U_DWORD_R": ModbusValueType.U_DWORD_R,
    "S_DWORD_R": ModbusValueType.S_DWORD_R,
    "FP32": ModbusValueType.FP32,
    "FP32_R": ModbusValueType.FP32_R,
}

# Helper function to create modbus register config
def modbus_register_config_struct(conf):
    """Create a ModbusRegisterConfig struct from YAML configuration"""
    config_struct = cg.StructInitializer(
        samsung_ac_ns.struct("ModbusRegisterConfig"),
        ("address", conf[CONF_REGISTER_ADDRESS]),
        ("register_type", REGISTER_TYPES[conf[CONF_REGISTER_TYPE]]),
        ("value_type", VALUE_TYPES[conf[CONF_VALUE_TYPE]]),
        ("device_address", conf[CONF_DEVICE_ADDRESS]),
        ("multiplier", conf.get(CONF_MULTIPLIER, 1.0)),
        ("offset", conf.get(CONF_OFFSET, 0.0)),
        ("bitmask", conf.get(CONF_BITMASK, 0xFFFFFFFF)),
    )
    return config_struct

# Base schema for modbus components
MODBUS_COMPONENT_BASE_SCHEMA = cv.Schema({
    cv.Required(CONF_REGISTER_ADDRESS): cv.hex_int,
    cv.Required(CONF_DEVICE_ADDRESS): cv.string,
    cv.Optional(CONF_REGISTER_TYPE, default="holding"): cv.enum(REGISTER_TYPES, lower=True),
    cv.Optional(CONF_VALUE_TYPE, default="U_WORD"): cv.enum(VALUE_TYPES, upper=True),
    cv.Optional(CONF_MULTIPLIER, default=1.0): cv.float_,
    cv.Optional(CONF_OFFSET, default=0.0): cv.float_,
    cv.Optional(CONF_BITMASK, default=0xFFFFFFFF): cv.hex_int,
    cv.Required(CONF_SAMSUNG_AC_MODBUS_CONTROLLER_ID): cv.use_id(Samsung_AC_Modbus_Controller),
})

# Controller configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Samsung_AC_Modbus_Controller),
    cv.Required(CONF_SAMSUNG_AC_ID): cv.use_id(samsung_ac_ns.class_("Samsung_AC")),
}).extend(cv.polling_component_schema("30s"))


async def to_code(config):
    """Configure the Samsung AC Modbus Controller"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Get the Samsung AC component
    samsung_ac = await cg.get_variable(config[CONF_SAMSUNG_AC_ID])
    cg.add(var.set_samsung_ac(samsung_ac))
    
    # Register the modbus controller with the Samsung AC component to receive NASA messages
    cg.add(samsung_ac.register_modbus_controller(var))


# Helper function for component setup
async def setup_modbus_component(var, config, controller_var):
    """Common setup for modbus components"""
    # Initialize the component with the config
    cg.add(var.set_register_address(config[CONF_REGISTER_ADDRESS]))
    cg.add(var.set_device_address(config[CONF_DEVICE_ADDRESS]))
    cg.add(var.set_register_type(REGISTER_TYPES[config[CONF_REGISTER_TYPE]]))
    cg.add(var.set_value_type(VALUE_TYPES[config[CONF_VALUE_TYPE]]))
    cg.add(var.set_multiplier(config[CONF_MULTIPLIER]))
    cg.add(var.set_offset(config[CONF_OFFSET]))
    cg.add(var.set_bitmask(config[CONF_BITMASK]))
    
    # Register with the controller - use direct type checking
    if var.type == Samsung_AC_Modbus_Sensor:
        cg.add(controller_var.register_sensor(var))
    elif var.type == Samsung_AC_Modbus_Switch:
        cg.add(controller_var.register_switch(var))
    elif var.type == Samsung_AC_Modbus_Number:
        cg.add(controller_var.register_number(var)) 