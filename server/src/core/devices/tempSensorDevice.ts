import { AbstractDevice, DeviceTypeEnum } from './abstractDevice';
import * as dbgModule from 'debug';

const debug = dbgModule('domotik:device:tempSensor');

class TempSensorDevice extends AbstractDevice {

	public temperature: number;

	constructor(id: string) {
		super(id, 'tempSensor');
		debug('Temp sensor ' + id + ' created');
		setInterval(() => {
			this.temperature = 20 + parseFloat((Math.random() * 1000 / 100).toFixed(2));
		}, 1000);
	}

	toString(): string {
		return 'tempSensor@' + this.id;
	}

	getValue(): number {
		return this.temperature;
	}

	loadConfigurationNode(node: any): void {

	}	

	postConfiguration(): void { }
	
}

export { TempSensorDevice };
