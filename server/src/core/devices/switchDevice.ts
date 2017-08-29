import { AbstractDevice, DeviceTypeEnum } from './abstractDevice';
import { LightDevice } from './lightDevice';
import { Controller } from '../controller';
import { DeviceManager } from './deviceManager';
import * as dbgModule from 'debug';

const debug = dbgModule('domotik:device:switch');

interface OnClickCallback { (): AbstractDevice[] };
interface OnDoubleClickCallback { (): AbstractDevice[] };
interface OnLongPressCallback { (): AbstractDevice[] };

class ClickAction {
	private devices: AbstractDevice[];
	private functionToInvoke: { (): boolean };

	resolve(): void {
		console.log("Resolve ", this.definition);
		this.devices = DeviceManager.getInstance().getDevices(this.definition.type, this.definition.id);
		if (!this.devices.length)
			throw new Error("Could not resolve devices " + JSON.stringify(this.definition));
		// Consider that all the devices are the same type. We can retrieve the function to invoke
		// from the first device.
		this.functionToInvoke = this.devices[0][this.definition.action];
		delete this.definition;
	}

	execute(): AbstractDevice[] {
		var updatedDevices: AbstractDevice[] = [];
		this.devices.forEach(device => {
			if (this.functionToInvoke.apply(device))
				updatedDevices.push(device);
		});
		return updatedDevices;
	}

	public definition: {
		type: DeviceTypeEnum;
		id: string;
		action: string;
	}
};

class SwitchDevice extends AbstractDevice {

	private _clickedCb: OnClickCallback;
	private _doubleClickedCb: OnDoubleClickCallback;
	private _longPressCb: OnLongPressCallback;
	private _singleClickActions: ClickAction[];
	private _doubleClickActions: ClickAction[];

	constructor(id: string) {
		super(id, 'switch');
		debug('Switch ' + id + ' created');
	}

	// -------------- Single click management --------------
	onSingleClick(cb: OnClickCallback): void {
		this._clickedCb = cb;
	}

	emitSingleClick(): AbstractDevice[] {
		debug(this.toString() + ': clicked');
		if (this._clickedCb)
			return this._clickedCb();
		else if (this._singleClickActions && this._singleClickActions.length) {
			return this._executeActions(this._singleClickActions);
		}
		else
			return [];
	}

	// -------------- Double click management --------------
	onDoubleClick(cb: OnDoubleClickCallback): void {
		this._doubleClickedCb = cb;
	}

	emitDoubleClick(): AbstractDevice[] {
		debug(this.toString() + ': double-clicked');
		if (this._doubleClickedCb)
			return this._doubleClickedCb();
		else if (this._doubleClickActions && this._doubleClickActions.length) {
			return this._executeActions(this._doubleClickActions);
		}
		else
			return [];
	}

	// -------------- Long press management --------------
	onLongPress(cb: OnLongPressCallback): void {
		this._longPressCb = cb;
	}

	emitLongPress(): AbstractDevice[] {
		debug(this.toString() + ': long pressed');
		if (this._longPressCb)
			return this._longPressCb();
		else
			return [];
	}

	// -------------- Configuration management --------------

	loadConfigurationNode(node: any): void {
		function _loadDefinitions(definitions: any): ClickAction[] {
			var actions: ClickAction[] = [];
			if (definitions) {
				definitions.forEach((def: any) => {
					var action = new ClickAction();
					action.definition = def;
					actions.push(action);
				});
			}
			return actions;
		}

		this._singleClickActions = _loadDefinitions(node.singleClickActions);
		this._doubleClickActions = _loadDefinitions(node.doubleClickActions);
	}

	postConfiguration(): void {
		this._singleClickActions.forEach(action => {
			action.resolve();
		});
		this._doubleClickActions.forEach(action => {
			action.resolve();
		});
	}

	// -------------- Other stuff --------------
	toString(): string {
		return 'switch@' + this.id;
	}
	private _executeActions(actions: ClickAction[]): AbstractDevice[] {
		var result: AbstractDevice[] = [];
		actions.forEach((action) => {
			result = result.concat(action.execute());
		});
		return result;
	}		

}

export { SwitchDevice, OnClickCallback, OnDoubleClickCallback, OnLongPressCallback };
