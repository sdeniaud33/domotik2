import { IMySensorsDevice } from './i_mySensorsDevice';
import * as dbgModule from 'debug';
import { GatewayController } from './gatewayController';
import { CommandEnum, TypeEnum, DeviceTypeEnum } from './mySensorsEnums';
import { DeviceManager } from '../devices/deviceManager';
import { MySensorsLightDevice } from './mySensorsLightDevice';
import { AbstractDevice } from '../devices/abstractDevice';

const debug = dbgModule('domotik:device:light');

class Action {
	deviceId: string;
	command: string;
};

class MySensorsPushButtonDevice extends AbstractDevice implements IMySensorsDevice {
	node: number;
	sensor: number;
	private actions = [] as Action[];

	constructor(node: number, sensor: number) {
		super(GatewayController.buildDeviceId(node, sensor), 'switch');
		this.node = node;
		this.sensor = sensor;
	}


	parseValue(type: TypeEnum, value: string): boolean {
		if (super.parseValue(type, value))
			return true;	
		this.runCommand(parseInt(value));
		return true;
	}
	// /**
	//  * Invoked when the push button is single clicked
	//  */
	// singleClick(): void {
	// 	console.log("SINGLE CLICK");
	// 	this.actions.forEach((action) => {
	// 		DeviceManager.getInstance().getDevice(action.deviceId).runCommand(action.command);
	// 	});
	// }

	/**
	 * Configures the device from a mongo document
	 * @param mongoDoc The mongo document to parse
     */
	parseMongoDocument(mongoDoc: any): void {
		super.parseMongoDocument(mongoDoc);
		this.actions = (mongoDoc.actions || []).reduce((obj: any, action: any) => {
			delete (action._id);
			obj.push(action);
			return obj;
		}, []);
	}

}

export { MySensorsPushButtonDevice };
