import { IMySensorsDevice } from './i_mySensorsDevice';
import * as dbgModule from 'debug';
import { GatewayController } from './gatewayController';
import { CommandEnum, TypeEnum, DeviceTypeEnum } from './mySensorsEnums';
import { AbstractDevice } from '../devices/abstractDevice';

const debug = dbgModule('domotik:device:shutter');

class MySensorsRollingShutterDevice extends AbstractDevice implements IMySensorsDevice {
	node: number;
	sensor: number;
	public position: number;

	constructor(node: number, sensor: number) {
		super(GatewayController.buildDeviceId(node, sensor), 'shutter');
		this.node = node;
		this.sensor = sensor;
	}

	getStatus(): number {
		return this.position;
	}

	// up(): boolean {
	// 	GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_UP, 1);
	// 	return true;
	// }

	// down(): boolean {
	// 	GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_DOWN, 1);
	// 	return true;
	// }

	// stop(): boolean {
	// 	GatewayController.getInstance().emitCommand(this.node, this.sensor, CommandEnum.C_REQ, TypeEnum.V_STOP, 1);
	// 	return true;
	// }

	parseValue(type: TypeEnum, value: string): boolean {
		if (super.parseValue(type, value))
			return true;
		console.log("PARSING POSITION " + value);
		this.position = parseInt(value);
		return true;
	}

	getObjectToExposeViaRest(): any {
		var obj = super.getObjectToExposeViaRest();
		obj.position = this.position;
		obj.positionBy10 = Math.floor(this.position / 10) * 10;
		return obj;
	}
}

export { MySensorsRollingShutterDevice };
