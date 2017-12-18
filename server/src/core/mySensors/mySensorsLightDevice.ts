import { IMySensorsDevice } from './i_mySensorsDevice';
import * as dbgModule from 'debug';
import { GatewayController } from './gatewayController';
import { CommandEnum, TypeEnum, DeviceTypeEnum } from './mySensorsEnums';
import { AbstractDevice } from '../devices/abstractDevice';

const debug = dbgModule('domotik:device:light');

type LightStatus = "on" | "off" | "unknown";

class MySensorsLightDevice extends AbstractDevice implements IMySensorsDevice {
	node: number;
	sensor: number;
	public status: LightStatus = 'off';


	constructor(node: number, sensor: number) {
		super(GatewayController.buildDeviceId(node, sensor), 'light');
		this.node = node;
		this.sensor = sensor;
	}

	getStatus(): LightStatus {
		return this.status;
	}
/*
	on(): boolean {
		GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_STATUS, 1);
		return true;
	}

	off(): boolean {
		GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_STATUS, 0);
		return true;
	}

	switch(): boolean {
		console.log(">>>>> SWITCH <<<<<");
		GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_STATUS, 2);
		return true;
	}
*/
	getObjectToExposeViaRest(): any {
		var obj = super.getObjectToExposeViaRest();
		obj.status = this.status;
		return obj;
	}

	parseValue(type: TypeEnum, value: string): boolean {
		if (super.parseValue(type, value))
			return true;	
		console.log("PARSING " + value);
		if (value === "1")
			this.status = 'on';
		else if (value === "0")
			this.status = 'off';
		else {
			console.log("Unparsed value " + value);
			return false;
		}
		return true;
	}

}

export { MySensorsLightDevice };
