"use strict";

import { CommandEnum, TypeEnum, InternalEnum, DeviceTypeEnum, ValueTypeEnum } from './mySensorsEnums';
import { DeviceManager } from '../devices/deviceManager';
import { AbstractDevice } from '../devices/abstractDevice';
import { MySensorsLightDevice } from './mySensorsLightDevice';
import { MySensorsTempSensorDevice } from './mySensorsTempSensorDevice';
import { MySensorsPushButtonDevice } from './mySensorsPushButtonDevice';

import { IMySensorsDevice } from './i_mySensorsDevice';
import { LocationManager } from '../locations/locationManager';

var SerialPort = require('serialport');

class GatewayController {
	private static _instance: GatewayController;
	comCtrl: any;
	pendingCmd: string = "";
	deviceManager: DeviceManager;

	constructor() {
		if (GatewayController._instance)
			throw new Error("Singleton violation : GatewayController");
		this.deviceManager = DeviceManager.getInstance();
	}

	public static getInstance(): GatewayController {
		if (!GatewayController._instance) {
			GatewayController._instance = new GatewayController();
			GatewayController._instance.connect();
		}
		return GatewayController._instance;
	}

	connect() {
		console.log(">>.1");
		// this.comCtrl = new SerialPort(
		// 	'COM14', {
		// 		baudrate: 115200,
		// 		dataBits: 8,
		// 		parity: 'none',
		// 		stopBits: 1,
		// 		flowControl: false
		// 	});
		this.comCtrl = new SerialPort(
			'COM14', {
				baudRate: 115200,
			});
		var _this = this;
		this.comCtrl.on('data', function (data: any) {
			_this.parseCommand(data.toString());
		});
	}

	public emitCommand(node: number, sensor: number, command: CommandEnum, type: TypeEnum, value: number) {
		var cmd = [
			node,
			sensor,
			command,
			0, // No ACK
			type,
			value
		].join(";")
		console.log("SEND : " + cmd);

		this.comCtrl.write(cmd + '\n');
	}

	public static buildDeviceId(node: number, sensor: number) {
		return "mySensors-" + node + "-" + sensor;
	}

	private processCommand(cmd: string) {
//		console.log("Parsing " + cmd);
		var parts = cmd.split(';');
		var nodeId = parseInt(parts[0]);
		if (nodeId === 0)
			return;
		var parsed = {
			node: nodeId,
			sensor: parseInt(parts[1]),
			command: parseInt(parts[2]), // Get / Set / ...
			ack: parseInt(parts[3]),
			type: parseInt(parts[4]), // S_DOOR, S_LIGHT, ...
			value: parts[5],
		};

		var device: AbstractDevice;
		switch (parsed.command) {
			case CommandEnum.C_SET: 
				var deviceId = GatewayController.buildDeviceId(parsed.node, parsed.sensor);
				console.log("Value for sensor #" + deviceId + " : " + parsed.value);
				if (!this.deviceManager.hasDevice(deviceId))
					console.log("Unknown device .... " + deviceId);
				else {
					device = this.deviceManager.getDevice(deviceId);
					if ('parseValue' in device) {
						try {
							device['parseValue'](parsed.type, parsed.value);
						} catch (err) {
							console.log("ERROR ", err);
						}
					}
				}	
				break;
			case CommandEnum.C_PRESENTATION:
				var deviceId = GatewayController.buildDeviceId(parsed.node, parsed.sensor);
				if (this.deviceManager.hasDevice(deviceId))
					return;	

				if (parsed.type === DeviceTypeEnum.S_BINARY) {
					device = this.deviceManager.registerDevice(new MySensorsLightDevice(parsed.node, parsed.sensor));					
					device.location = LocationManager.getInstance().getLocation("mysensors");
					console.log("Created device " + device.toString());
				}	
				else if (parsed.type === DeviceTypeEnum.S_TEMP) {
					device = this.deviceManager.registerDevice(new MySensorsTempSensorDevice(parsed.node, parsed.sensor));
					device.room = LocationManager.getInstance().getLocation("mysensors");
					console.log("Created device " + device.toString());
				}	
				else if (parsed.type === DeviceTypeEnum.S_DOOR) {
					device = this.deviceManager.registerDevice(new MySensorsPushButtonDevice(parsed.node, parsed.sensor));
					device.room = LocationManager.getInstance().getLocation("mysensors");
					console.log("Created device " + device.toString());
				}	
				else {
					console.log("Unknown device type ", parsed);
				}
				break;
		}
		//		console.log(">>>>", parsed);
		return parsed;
	}

	private parseCommand(cmd: string) {
		var lastIdx = cmd.lastIndexOf('\n');
		// console.log(">>>> cmd : " + lastIdx, cmd);
		// console.log(">>>> pending : ", this.pendingCmd);
		if (lastIdx === -1) {
			this.pendingCmd += cmd;
			return;
		}
		this.pendingCmd += cmd.substr(0, lastIdx);
		//		console.log(">>>> pending.2 : ", this.pendingCmd);
		var cmds = this.pendingCmd.split('\n');
		this.pendingCmd = cmd.substr(lastIdx + 1);
		cmds.forEach((cmd) => this.processCommand(cmd));
	}
}

export { GatewayController };