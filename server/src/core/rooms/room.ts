class Room {
	id: string;
	label: string;

	constructor(id: string) {
		this.id = id;
	}

	toString(): string {
		return this.label;
	}

}

export { Room };
