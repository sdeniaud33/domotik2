type DeviceCategoryEnum = 'light' | 'switch' | 'tempSensor' | 'shutter';
import { DeviceLocation } from '../locations/DeviceLocation';
import { LocationManager } from '../locations/locationManager';
import { DeviceTypeEnum, TypeEnum, CommandEnum } from '../mySensors/mySensorsEnums';
import { GatewayController } from '../mySensors/gatewayController';
import { DeviceManager } from './deviceManager';

class SlaveDevice {
	node: number;
	sensor: number;
	command: string;
}

class DeviceCommand {
	id: number;
	name: string;
	type: TypeEnum;
	value: any;
	description: string;
	script: string;
	slaveDevices = [] as SlaveDevice[];
}


abstract class AbstractDevice {
	id: string;
	category: DeviceCategoryEnum;
	label: string;
	location: DeviceLocation;
	[key: string]: any;
	private commandsById: {
		[key: number]: DeviceCommand;
	};
	private commandsByName: {
		[key: string]: DeviceCommand;
	};
	private commands = [] as DeviceCommand[];

	deviceType: DeviceTypeEnum;

	constructor(id: string, category: DeviceCategoryEnum) {
		this.id = id;
		this.category = category;
	}

	toString(): string {
		return this.category + '@' + this.id;
	}

	runCommand(cmdNameOrId: number | string): boolean {
		var cmd: DeviceCommand;
		if (typeof (cmdNameOrId) === 'string')
			cmd = this.commandsByName[cmdNameOrId];
		else
			cmd = this.commandsById[cmdNameOrId];
		if (!cmd)
			return false;
		if (cmd.type === TypeEnum.V_SCRIPT) {
			try {
				eval("const DeviceManager = require('./deviceManager').DeviceManager;\n" + cmd.script);
				return true;
			}
			catch (err) {
				console.log("ERROR : could not run script " + cmd.script, err);
				return false;
			}
		}
		else {
			if (cmd.slaveDevices.length > 0) {
				var ok = true;
				cmd.slaveDevices.forEach(slaveDevice => {
					var deviceId = GatewayController.buildDeviceId(slaveDevice.node, slaveDevice.sensor);
					var device = DeviceManager.getInstance().getDevice(deviceId);
					if (!device.runCommand(slaveDevice.command))
						ok = false;
				});
				return ok;
			}
			else {
				GatewayController.getInstance().emitCommand(
					this.node,
					this.sensor,
					CommandEnum.C_REQ,
					cmd.type,
					cmd.value);
			}
			return true;
		}
	}

	parseValue(type: TypeEnum, value: string): boolean {
		this.commands.forEach(cmd => {
			if (cmd.slaveDevices.length > 0) {
				console.log("PARSE VALUE " + TypeEnum[type] + " / " + value);
				if ((cmd.type === type) && (cmd.value == value)) {
					var ok = true;
					cmd.slaveDevices.forEach(slaveDevice => {
						var deviceId = GatewayController.buildDeviceId(slaveDevice.node, slaveDevice.sensor);						
						var device = DeviceManager.getInstance().getDevice(deviceId);
						if (!device.runCommand(slaveDevice.command))
							ok = false;	
					});
					return ok;
				}
			}
		});
		return false;
	};

	getObjectToExposeViaRest(): any {
		return {
			id: this.id,
			category: this.category,
			location: this.location.id,
			label: this.label,
			description: this.description,
			commands: this.commands.map((cmd) => {
				return {
					id: cmd.id,
					name: cmd.name,
					description: cmd.description,
				}
			}),
		};
	}
	

	/**
	 * Configures the device from a mongo document
	 * @param mongoDoc The mongo document to parse
	 */
	parseMongoDocument(mongoDoc: any): void {
		this.location = LocationManager.getInstance().getLocation(mongoDoc.location);
		this.deviceType = (<any>DeviceTypeEnum)[mongoDoc.deviceType];
		this.label = mongoDoc.label;
		var _this = this;
		this.commandsById = {};
		this.commandsByName = {};
		if (mongoDoc.commands) {
			mongoDoc.commands.forEach((mongoCmd:any) => {
				var command = new DeviceCommand();
				command.id = mongoCmd.id;
				command.name = mongoCmd.name;
				command.description = mongoCmd.description;
				command.type = (<any>TypeEnum)[mongoCmd.type];
				if (command.type === TypeEnum.V_SCRIPT) {
					command.script = mongoCmd.script;
				}
				else {
					command.value = mongoCmd.value;
					command.slaveDevices = [];
					(mongoCmd.slaveDevices || []).forEach((mongoSlave: any) => {
						var slaveDevice = new SlaveDevice();
						slaveDevice.node = mongoSlave.node;
						slaveDevice.sensor = mongoSlave.sensor;
						slaveDevice.command = mongoSlave.command;
						command.slaveDevices.push(slaveDevice);
					});
				}	
				_this.commandsById[command.id] = command;
				_this.commandsByName[command.name] = command;	
				_this.commands.push(command);
			});
		}
	}
}

export { AbstractDevice, DeviceCategoryEnum };
