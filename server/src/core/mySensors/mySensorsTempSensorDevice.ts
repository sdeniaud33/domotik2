import { IMySensorsDevice } from './i_mySensorsDevice';
import * as dbgModule from 'debug';
import { DeviceTypeEnum, TypeEnum } from './mySensorsEnums';
import { GatewayController } from './gatewayController';
import { AbstractDevice } from '../devices/abstractDevice';

const debug = dbgModule('domotik:device:light');

class MySensorsTempSensorDevice extends AbstractDevice implements IMySensorsDevice {
	node: number;
	sensor: number;
	public temperature: number;

	constructor(node: number, sensor: number) {
		super(GatewayController.buildDeviceId(node, sensor), 'tempSensor');
		this.node = node;
		this.sensor = sensor;
	}

	parseValue(type: TypeEnum, value: string): boolean {
		if (super.parseValue(type, value))
			return true;	
		this.temperature = parseFloat(value);
		return true;
	}

	getObjectToExposeViaRest(): any {
		var obj = super.getObjectToExposeViaRest();
		obj.temperature = this.temperature;
		return obj;
	}

	getValue(): number {
		return this.temperature;
	}

}

export { MySensorsTempSensorDevice };
