"use strict";

var SerialPort = require('serialport');

function RS485Connector(port) {
	this.port = port;
}

RS485Connector.prototype = {
    connect: function() {
        var _this = this;
		this.serialPort = new SerialPort('COM7', {
			baudrate: 9600,
			dataBits: 8,
			parity: 'none',
			stopBits: 1,
			flowControl: false
		});
		this.serialPort.on('data', function(data) {
            console.log(data.toString());
		});
        
    }
}

module.exports.RS485Connector = RS485Connector;