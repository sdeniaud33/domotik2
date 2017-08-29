type DeviceTypeEnum = 'light' | 'switch' | 'tempSensor';
import { Room } from '../rooms/Room';

abstract class AbstractDevice {
	id: string;
	type: DeviceTypeEnum;
	label: string;
	room: Room;
	[key: string]: any;

	constructor(id: string, type: DeviceTypeEnum) {
		this.id = id;
		this.type = type;
	}

	abstract loadConfigurationNode(node: any): void;

	abstract toString(): string;

	abstract postConfiguration(): void;

}

export { AbstractDevice, DeviceTypeEnum};
