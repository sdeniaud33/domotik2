import { GatewayController } from './core/mySensors/gatewayController';

import WebServer from './web/webserver';
import { DeviceManager } from './core/devices/deviceManager';
import { LocationManager } from './core/locations/locationManager';

var webServer = new WebServer();

LocationManager.getInstance();
DeviceManager.getInstance();
GatewayController.getInstance();

console.log("Started ...");
