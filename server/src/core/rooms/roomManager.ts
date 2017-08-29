import { Room } from './room';
import * as dbgModule from 'debug';

import * as fs from 'fs';

const debug = dbgModule('domotik:roomManager');

interface IRoomsMap {
	[index: string]: Room;
}

class RoomManager {

	private static _instance: RoomManager;

	roomsById = {} as IRoomsMap;
	private _rooms = [] as Room[];

	constructor() {
		if (RoomManager._instance)
			throw new Error("Singleton violation : RoomManager");
		debug('Ready');
	}

	public static getInstance(): RoomManager {
		if (!RoomManager._instance) {
			RoomManager._instance = new RoomManager();
			RoomManager._instance.loadConfigurationFile();
		}
		return RoomManager._instance;
	}

	private registerRoom(room: Room): void {
		this.roomsById[room.id] = room;
		this._rooms.push(room);
		// if (device instanceof IStatusHandler)
		// 	console.log("XXXXXXXXXX", (device as IStatusHandler).getStatus());
		debug('Registered room ' + room);
	}

	getRoom(id: string): Room {
		var room = this.roomsById[id];
		if (!room)
			throw new Error("Unknown room : " + id);
		return room;
	}

	getRoomsAsMap(): IRoomsMap {
		return this.roomsById;
	}

	private createDevice(roomId: string): Room {
		let room = new Room(roomId);
		return room;
	}

	private loadConfigurationFile(): void {
		var definitions = JSON.parse(fs.readFileSync('./config/devices.json', 'utf8'));
		// load rooms
		definitions.rooms.forEach((roomDef: any) => {
			var room = this.createDevice(roomDef.id);
			room.label = roomDef.label;
			this.registerRoom(room);
		});
	}

}

export { RoomManager };