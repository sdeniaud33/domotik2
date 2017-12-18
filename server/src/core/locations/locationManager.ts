import { DeviceLocation } from './deviceLocation';
import * as dbgModule from 'debug';

import * as fs from 'fs';
import * as mongodb from 'mongodb';	

const debug = dbgModule('domotik:locationManager');

interface ILocationsMap {
	[index: string]: DeviceLocation;
}

class LocationManager {

	private static _instance: LocationManager;
	private mongoDb: mongodb.Db;

	locationsById = {} as ILocationsMap;
	private _locations = [] as DeviceLocation[];

	constructor() {
		if (LocationManager._instance)
			throw new Error("Singleton violation : LocationManager");
		var _this = this;
		mongodb.MongoClient.connect("mongodb://localhost:27017", function (err: any, client: mongodb.Db) {
			_this.mongoDb = client.db('domotik');
			console.log("Connected");
			_this.loadConfigurationFromDb();
		});
		debug('Ready');
	}

	public static getInstance(): LocationManager {
		if (!LocationManager._instance) {
			LocationManager._instance = new LocationManager();
		}
		return LocationManager._instance;
	}

	private registerLocation(location: DeviceLocation): DeviceLocation {
		this.locationsById[location.id] = location;
		this._locations.push(location);
		// if (device instanceof IStatusHandler)
		// 	console.log("XXXXXXXXXX", (device as IStatusHandler).getStatus());
		debug('Registered location ' + location);
		return location;
	}

	getLocation(id: string): DeviceLocation {
		var location = this.locationsById[id];
		if (!location)
			throw new Error("Unknown location : " + id);
		return location;
	}

	getLocationsAsMap(): ILocationsMap {
		return this.locationsById;
	}

	private createLocation(mongoDoc: any): DeviceLocation {
		let location = new DeviceLocation(mongoDoc.id);
		location.label = mongoDoc.label;
		return location;
	}

	private loadConfigurationFromDb(): void {
		var cln = this.mongoDb.collection('locations');
		var _this = this;
		cln.find({}).toArray(function (err: any, data: any) {
			data.forEach((mongoDoc:any) => {
				_this.registerLocation(_this.createLocation(mongoDoc));
			});
			console.log("XXXXX", data);
		});
	}

}

export { LocationManager };