enum CommandEnum {
	C_PRESENTATION = 0,
	C_SET,
	C_REQ,
	C_INTERNAL,
	C_STREAM,
}

enum TypeEnum {
	V_TEMP = 0,
	V_HUM,
	V_STATUS,
	V_PERCENTAGE,
	V_PRESSURE,
	V_FORECAST,
	V_RAIN,
	V_RAINRATE,
	V_WIND,
	V_GUST,
	V_DIRECTION,
	V_UV,
	V_WEIGHT,
	V_DISTANCE,
	V_IMPEDANCE,
	V_ARMED,
	V_TRIPPED,
	V_WATT,
	V_KWH,
	V_SCENE_ON,
	V_SCENE_OFF,
	V_HVAC_FLOW_STATE,
	V_HVAC_SPEED,
	V_LIGHT_LEVEL,
	V_VAR1,
	V_VAR2,
	V_VAR3,
	V_VAR4,
	V_VAR5,
	V_UP,
	V_DOWN,
	V_STOP,
	V_IR_SEND,
	V_IR_RECEIVE,
	V_FLOW,
	V_VOLUME,
	V_LOCK_STATUS,
	V_LEVEL,
	V_VOLTAGE,
	V_CURRENT,
	V_RGB,
	V_RGBW,
	V_ID,
	V_UNIT_PREFIX,
	V_HVAC_SETPOINT_COOL,
	V_HVAC_SETPOINT_HEAT,
	V_HVAC_FLOW_MODE,
	V_TEXT,
	V_CUSTOM,
	V_POSITION,
	V_IR_RECORD,
	V_PH,
	V_ORP,
	V_EC,
	V_VAR,
	V_VA,
	V_POWER_FACTOR,	
	V_SCRIPT = 255
}


enum InternalEnum {
	I_BATTERY_LEVEL = 0,
	I_TIME,
	I_VERSION,
	I_ID_REQUEST,
	I_ID_RESPONSE,
	I_INCLUSION_MODE,
	I_CONFIG,
	I_FIND_PARENT,
	I_FIND_PARENT_RESPONSE,
	I_LOG_MESSAGE,
	I_CHILDREN,
	I_SKETCH_NAME,
	I_SKETCH_VERSION,
	I_REBOOT,
	I_GATEWAY_READY,
	I_SIGNING_PRESENTATION,
	I_NONCE_REQUEST,
	I_NONCE_RESPONSE,
	I_HEARTBEAT_REQUEST,
	I_PRESENTATION,
	I_DISCOVER_REQUEST,
	I_DISCOVER_RESPONSE,
	I_HEARTBEAT_RESPONSE,
	I_LOCKED,
	I_PING,
	I_PONG,
	I_REGISTRATION_REQUEST,
	I_REGISTRATION_RESPONSE,
	I_DEBUG,
}

enum DeviceTypeEnum {
	S_DOOR,
	S_MOTION,
	S_SMOKE,
	S_BINARY,
	S_DIMMER,
	S_COVER,
	S_TEMP,
	S_HUM,
	S_BARO,
	S_WIND,
	S_RAIN,
	S_UV,
	S_WEIGHT,
	S_POWER,
	S_HEATER,
	S_DISTANCE,
	S_LIGHT_LEVEL,
	S_ARDUINO_NODE,
	S_ARDUINO_REPEATER_NODE,
	S_LOCK,
	S_IR,
	S_WATER,
	S_AIR_QUALITY,
	S_CUSTOM,
	S_DUST,
	S_SCENE_CONTROLLER,
	S_RGB_LIGHT,
	S_RGBW_LIGHT,
	S_COLOR_SENSOR,
	S_HVAC,
	S_MULTIMETER,
	S_SPRINKLER,
	S_WATER_LEAK,
	S_SOUND,
	S_VIBRATION,
	S_MOISTURE,
	S_INFO,
	S_GAS,
	S_GPS,
	S_WATER_QUALITY
}

enum ValueTypeEnum {
	P_STRING = 0,
	P_BYTE,
	P_INT16,
	P_UINT16,
	P_LONG32,
	P_ULONG32,
	P_CUSTOM,
}

export { CommandEnum, TypeEnum, InternalEnum, DeviceTypeEnum, ValueTypeEnum };
