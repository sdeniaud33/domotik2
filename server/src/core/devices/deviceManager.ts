import { AbstractDevice, DeviceTypeEnum } from './abstractDevice';
import * as dbgModule from 'debug';
import { LightDevice } from './lightDevice';
import { SwitchDevice } from './switchDevice';
import { TempSensorDevice } from './tempSensorDevice';
import { Room } from '../rooms/Room';
import { RoomManager } from '../rooms/roomManager';

import * as fs from 'fs';

const debug = dbgModule('domotik:deviceManager');

interface IDevicesMap {
	[index: string]: AbstractDevice;
}

class DeviceManager {

	private static _instance: DeviceManager;

	devicesById = {} as IDevicesMap;
	private _devices = [] as AbstractDevice[];

	constructor() {
		if (DeviceManager._instance)
			throw new Error("Singleton violation : DeviceManager");
		debug('Ready');
	}

	public static getInstance(): DeviceManager {
		if (!DeviceManager._instance) {
			DeviceManager._instance = new DeviceManager();
			DeviceManager._instance.loadConfigurationFile();
		}
		return DeviceManager._instance;
	}

	private registerDevice(device: AbstractDevice): void {
		if (this.devicesById[device.id])
			throw new Error("Duplicate device : " + device.id);
		this.devicesById[device.id] = device;
		this._devices.push(device);
		debug('Registered device ' + device);
	}

	getDevice(id: string, deviceTypeToCheck?: DeviceTypeEnum): AbstractDevice {
		var device = this.devicesById[id];
		if (!device)
			throw new Error("Unknown device : " + id);
		if (deviceTypeToCheck && device.type != deviceTypeToCheck)
			throw new Error("Invalid device : " + device.id + " was expected to be " + deviceTypeToCheck + ", but was " + device.type);
		return device;
	}

	getDevicesAsMap(): IDevicesMap {
		return this.devicesById;
	}

	getDevices(typeFilter?: DeviceTypeEnum, idFilter?: string): AbstractDevice[] {
		if (!typeFilter && !idFilter)
			return this._devices;
		
		if (typeof (idFilter) === 'string') {
			return [this.getDevice(idFilter, typeFilter)];
		}
		
		return this._devices.filter((device) => {
			if (typeFilter && device.type !== typeFilter)
				return false;
			if (idFilter && (idFilter !== '*') && (device.id !== idFilter)) {
				return false;
			}
			return true;
		});
	}

	private createDevice(deviceType: string, deviceId: string): AbstractDevice {
		let device;
		switch (deviceType) {
			case 'light':
				device = new LightDevice(deviceId);
				break;
			case 'switch':
				device = new SwitchDevice(deviceId);
				break;
			case 'tempSensor':
				device = new TempSensorDevice(deviceId);
				break;
			default:
				throw new Error("Invalid device type : " + deviceType);
		}
		return device;
	}

	private loadConfigurationFile(): void {
		var definitions = JSON.parse(fs.readFileSync('./config/devices.json', 'utf8'));
		// load devices
		definitions.devices.forEach((deviceDef: any) => {
			var device = this.createDevice(deviceDef.type, deviceDef.id);
			device.label = deviceDef.label;
			if (!deviceDef.roomId)
				throw new Error("Device " + deviceDef.id + " has no room");
			device.room = RoomManager.getInstance().getRoom(deviceDef.roomId);
			device.loadConfigurationNode(deviceDef);
			this.registerDevice(device);
		});

		this._devices.forEach((device) => {
			device.postConfiguration();
		});
	}

}

export { DeviceManager };