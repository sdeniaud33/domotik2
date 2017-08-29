import { Controller } from './core/controller';

import WebServer from './web/webserver';

var controller = Controller.getInstance();

var webServer = new WebServer(controller);

console.log("Started ...");
