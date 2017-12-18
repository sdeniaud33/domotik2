class DeviceLocation {
	id: string;
	label: string;

	constructor(id: string) {
		this.id = id;
	}

	toString(): string {
		return this.label;
	}

}

export { DeviceLocation };
