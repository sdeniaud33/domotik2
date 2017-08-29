import * as dbgModule from 'debug';
import { DeviceManager } from './devices/deviceManager';
import { DeviceTypeEnum } from './devices/abstractDevice';

const debug = dbgModule('domotik:controller');

interface RS485Message {
	crc: number;
	counter: number;
	moduleId: number;
	deviceId: number;
	deviceType: DeviceTypeEnum;
	payload: string;
}


class Controller {

	private static _instance: Controller = new Controller();

	name: string;

	constructor() {
		if (Controller._instance)
			throw new Error("Singleton violation : Controller");
		debug('ready');
		DeviceManager.getInstance();
	}

	public static getInstance(): Controller {
		return Controller._instance;
	}

	parseRS485(message: string): void {

	}
}

export { Controller, RS485Message };