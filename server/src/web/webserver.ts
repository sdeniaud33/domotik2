import * as http from 'http';
import * as dbgModule from 'debug';
const debug = dbgModule('domotik:webserver');

import App from './App';

class WebServer {
	server: any;
	port: number | string | boolean;

	constructor() {

		this.server = http.createServer(App);

		this.port = this.normalizePort(process.env.PORT || 3000);
		App.set('port', this.port);

		this.server.listen(this.port);
		this.server.on('error', this.onError);
		this.server.on('listening', this.onListening);
		debug('ready');
	}

	normalizePort(val: number | string): number | string | boolean {
		let port: number = (typeof val === 'string') ? parseInt(val, 10) : val;
		if (isNaN(port)) return val;
		else if (port >= 0) return port;
		else return false;
	}

	onError(error: NodeJS.ErrnoException): void {
		if (error.syscall !== 'listen') throw error;
		let bind = (typeof this.port === 'string') ? 'Pipe ' + this.port : 'Port ' + this.port;
		switch (error.code) {
			case 'EACCES':
				console.error(`${bind} requires elevated privileges`);
				process.exit(1);
				break;
			case 'EADDRINUSE':
				console.error(`${bind} is already in use`);
				process.exit(1);
				break;
			default:
				throw error;
		}
	}

	onListening(): void {
		debug('listening on port ' + this.port);
	}
}

export default WebServer;