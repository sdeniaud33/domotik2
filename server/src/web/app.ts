import * as path from 'path';
import * as express from 'express';
import * as logger from 'morgan';
import * as bodyParser from 'body-parser';
import * as exphbs from 'express-handlebars';
import { DeviceManager } from '../core/devices/deviceManager';
import { AbstractDevice } from '../core/devices/abstractDevice';
import { LocationManager } from '../core/locations/locationManager';
import { MySensorsLightDevice } from '../core/mySensors/mySensorsLightDevice';
import { MySensorsPushButtonDevice } from '../core/mySensors/mySensorsPushButtonDevice';

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
			var devicesToRender = DeviceManager.getInstance().getDevices().reduce((obj:any, device) => {
				obj[device.id] = device.getObjectToExposeViaRest();
				return obj;
			}, {});
			res.render('home', {
				devices: JSON.stringify(devicesToRender),
				locations: JSON.stringify(LocationManager.getInstance().getLocationsAsMap()),
			});
		});
		this.express.get('/devices', (req: express.Request, res: express.Response) => {
			res.json({
				devices: DeviceManager.getInstance().getDevices().map(
					device => {
						return device.getObjectToExposeViaRest();
					}),
			});
		});
		this.express.post('/device/command/:deviceId/:command', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.deviceId);
			device.runCommand(req.params.command);
			res.json({ devices: [] });
		});
		this.express.get('/device/:deviceId/status', (req: express.Request, res: express.Response) => {
			var device = DeviceManager.getInstance().getDevice(req.params.deviceId);
			if (device === undefined) {
				res.status(404);
				return;
			}
			console.log(">>>> DEVICE : ", device);
			var result: any = {
				device: device.getObjectToExposeViaRest(),
			};
			res.json(result);
		});
	}

}

export default new App().express;