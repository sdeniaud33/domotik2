import { DeviceTypeEnum, TypeEnum } from './mySensorsEnums';

interface IMySensorsDevice {
	node: number;
	sensor: number;
	deviceType: DeviceTypeEnum;

	parseValue(type: TypeEnum, value: string): boolean;
}

export { IMySensorsDevice };
