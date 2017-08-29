import * as path from 'path';
import * as express from 'express';
import * as logger from 'morgan';
import * as bodyParser from 'body-parser';
import * as exphbs from 'express-handlebars';
import { DeviceManager } from '../core/devices/deviceManager';
import { RoomManager } from '../core/rooms/roomManager';
import { AbstractDevice } from '../core/devices/abstractDevice';
import { LightDevice } from '../core/devices/lightDevice';
import { SwitchDevice } from '../core/devices/switchDevice';

// Creates and configures an ExpressJS web server.
class App {

	// ref to Express instance
	public express: express.Application;

	//Run configuration methods on the Express instance.
	constructor() {
		this.express = express();
		this.middleware();
		this.routes();
	}

	// Configure Express middleware.
	private middleware(): void {
//		this.express.use(logger('dev'));
		this.express.use(bodyParser.json());
		this.express.use(bodyParser.urlencoded({ extended: false }));
		this.express.use(express.static(path.join(__dirname, '/../../public')));
		this.express.engine('handlebars', exphbs({
			defaultLayout: 'main',
			layoutsDir: path.join(__dirname, '/../../views/layouts'),
		}));
		this.express.set('view engine', 'handlebars');
		this.express.set('views', path.join(__dirname, '/../../views'));
		this.express.use(function (err: Error, req: express.Request, res: express.Response, next: express.NextFunction) {
			console.log("An unhandled error occured :");
			console.error(err.stack);
			res.status(500).send(err.message || 'Something broke!');
		});
	}

	// Configure API endpoints.
	private routes(): void {
		this.express.get('/', (req: express.Request, res: express.Response, next: express.NextFunction) => {
			res.render('home', {
				devices: JSON.stringify(DeviceManager.getInstance().getDevicesAsMap()),
				rooms: JSON.stringify(RoomManager.getInstance().getRoomsAsMap()),
			});
		});
		this.express.get('/devices', (req: express.Request, res: express.Response) => {
			res.json({ devices: DeviceManager.getInstance().getDevices() });
		});
		this.express.post('/device/light/:lightId/switch', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.lightId, 'light');
			if (device === undefined) {
				res.status(404);
				return;
			}
			var light = device as LightDevice;
			light.switch();
			res.json({
				devices: [device]
			});
		});
		this.express.post('/device/switch/:switchId/singleClick', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.switchId, 'switch');
			if (device === undefined) {
				res.status(404);
				return;
			}
			var sw = device as SwitchDevice;
			var result = sw.emitSingleClick();
			res.json({ devices: result });
		});
		this.express.post('/device/switch/:switchId/doubleClick', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.switchId, 'switch');
			if (device === undefined) {
				res.status(404);
				return;
			}
			var sw = device as SwitchDevice;
			var result = sw.emitDoubleClick();
			res.json({ devices: result });
		});
		this.express.get('/device/:deviceId/status', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.deviceId);
			if (device === undefined) {
				res.status(404);
				return;
			}
			console.log(">>>> DEVICE : ", device);
			var result: any = {
				device: device,
			};
			if (device.hasOwnProperty('getStatus')) {
				console.log('>>>> . getStatus');
				result.status = (<LightDevice>device).getStatus();
			}
			res.json(result);
		});
	}

}

export default new App().express;