import { AbstractDevice, DeviceCategoryEnum } from './abstractDevice';
import * as dbgModule from 'debug';
import { DeviceLocation } from '../locations/deviceLocation';
import { LocationManager } from '../locations/locationManager';
import * as mongodb from 'mongodb';	

import * as fs from 'fs';
import { MySensorsLightDevice } from '../mySensors/mySensorsLightDevice';
import { MySensorsPushButtonDevice } from '../mySensors/mySensorsPushButtonDevice';
import { MySensorsTempSensorDevice } from '../mySensors/mySensorsTempSensorDevice';
import { MySensorsRollingShutterDevice } from '../mySensors/mySensorsRollingShutterDevice';
import { DeviceTypeEnum } from '../mySensors/mySensorsEnums';

const debug = dbgModule('domotik:deviceManager');

interface IDevicesMap {
	[index: string]: AbstractDevice;
}

class DeviceManager {

	private static _instance: DeviceManager;

	devicesById = {} as IDevicesMap;
	private _devices = [] as AbstractDevice[];
	private mongoDb: mongodb.Db;

	constructor() {
		if (DeviceManager._instance)
			throw new Error("Singleton violation : DeviceManager");
		debug('Ready');
		var _this = this;
		mongodb.MongoClient.connect("mongodb://localhost:27017", function (err: any, client: mongodb.Db) {
			_this.mongoDb = client.db('domotik');
			console.log("Connected");
			_this.loadConfigurationFile();
		});
	}

	public static getInstance(): DeviceManager {
		if (!DeviceManager._instance) {
			DeviceManager._instance = new DeviceManager();
		}
		return DeviceManager._instance;
	}

	public registerDevice(device: AbstractDevice): AbstractDevice {
		if (this.devicesById[device.id])
			throw new Error("Duplicate device : " + device.id);
		this.devicesById[device.id] = device;
		this._devices.push(device);
		debug('Registered device ' + device);
		// var cln = this.mongoDb.collection('devices');
		// cln.insertMany([device], function (err:any, result:any) {
			
		// });
		return device;
	}

	hasDevice(id: string) {
		return this.devicesById[id] !== undefined;
	}

	getDevice(id: string, deviceCategoryToCheck?: DeviceCategoryEnum): AbstractDevice {
		var device = this.devicesById[id];
		if (!device)
			throw new Error("Unknown device : " + id);
		if (deviceCategoryToCheck && device.category != deviceCategoryToCheck)
			throw new Error("Invalid device : " + device.id + " was expected to be " + deviceCategoryToCheck + ", but was " + device.category);
		return device;
	}

	getDevicesAsMap(): IDevicesMap {
		return this.devicesById;
	}

	getDevices(categoryFilter?: DeviceCategoryEnum, idFilter?: string): AbstractDevice[] {
		if (!categoryFilter && !idFilter)
			return this._devices;
		
		if (typeof (idFilter) === 'string') {
			return [this.getDevice(idFilter, categoryFilter)];
		}
		
		return this._devices.filter((device) => {
			if (categoryFilter && device.category !== categoryFilter)
				return false;
			if (idFilter && (idFilter !== '*') && (device.id !== idFilter)) {
				return false;
			}
			return true;
		});
	}

	private createDevice(mongoDoc:any): AbstractDevice {
		let device;
		switch (mongoDoc.category) {
			case 'light':
				device = new MySensorsLightDevice(mongoDoc.node, mongoDoc.sensor);
				break;
			case 'switch':
				device = new MySensorsPushButtonDevice(mongoDoc.node, mongoDoc.sensor);
				break;
			case 'tempSensor':
				device = new MySensorsTempSensorDevice(mongoDoc.node, mongoDoc.sensor);
				break;
			case 'shutter':
				device = new MySensorsRollingShutterDevice(mongoDoc.node, mongoDoc.sensor);
				break;
			default:
				throw new Error("Invalid device type : " + mongoDoc.category);
		}
		device.parseMongoDocument(mongoDoc);
		return device;
	}

	private loadConfigurationFile(): void {
		var t: DeviceTypeEnum;
		var cln = this.mongoDb.collection('devices');
		var _this = this;
		cln.find({}).toArray(function (err: any, data: any) {
			data.forEach((mongoDoc:any) => {
				_this.registerDevice(_this.createDevice(mongoDoc));
			});
			console.log(">>>>", require("util").inspect(_this._devices, false, null));
		});

	}

}

export { DeviceManager };