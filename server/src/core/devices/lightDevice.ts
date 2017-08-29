import { AbstractDevice, DeviceTypeEnum } from './abstractDevice';
import * as dbgModule from 'debug';

const debug = dbgModule('domotik:device:light');

type LightStatus = "on" | "off" | "unknown";

class LightDevice extends AbstractDevice {

	public status: LightStatus = 'off';

	constructor(id: string) {
		super(id, 'light');
		debug('Light ' + id + ' created');
	}
			
	toString(): string {
		return 'light@' + this.id;
	}

	on(): boolean {
		if (this.status === 'on')
			return false;
		this.status = 'on';
		debug(this.toString() + ' : ' + this.status);
		return true;
	}

	off(): boolean {
		if (this.status === 'off')
			return false;
		this.status = 'off';
		debug(this.toString() + ' : ' + this.status);
		return true;
	}

	getStatus(): LightStatus {
		return this.status;
	}

	switch(): boolean {
		if (this.status === 'on')
			this.off();
		else if (this.status === 'off')
			this.on();
		else
			this.on();
		return true;
	}

	loadConfigurationNode(node: any): void {

	}	

	postConfiguration(): void { }
	
}

export { LightDevice, LightStatus };
