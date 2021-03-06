app.controller('myCtrl', function ($scope, $timeout) {
	$scope.devices = devices;
	$scope.locations = locations;

	setInterval(() => {
		$scope.loadDevices();
	}, 2000);

	$scope.launchAjaxRequest = function (context) {
		$.ajax({
			type: context.method || 'GET',
			contentType: 'application/json',
			url: context.url,
			data: context.data ? JSON.stringify(context.data) : undefined,
			// beforeSend: function (request) {
			// 	if (context.scope.requestData.clientId) {
			// 		request.setRequestHeader("client-id", context.scope.requestData.clientId);
			// 	}
			// },
			success: function (serverData, textStatus, request) {
				context.callback && context.callback(undefined, serverData);
			},
			error: function (err) {
				context.callback && context.callback(err);
			},
		});
	}

	$scope.loadDevices = function () {
		$scope.launchAjaxRequest({
			method: 'GET',
			url: '/devices',
			callback: function (err, data) {
				if (data)
					$scope.updateDevices(data);
			}
		})
	}

	$scope.updateDevices = function (data) {
		if (!data.devices)
			return;
		$scope.$apply(function () {
			data.devices.forEach((device) => {
				if (device.category === 'light') {
					$scope.devices[device.id].status = device.status;
				}
				else if (device.category === 'tempSensor') {
					$scope.devices[device.id].temperature = device.temperature;
				}
				else if (device.category === 'shutter') {
					$scope.devices[device.id].position = device.position;
					$scope.devices[device.id].positionBy10 = device.positionBy10;
				}
			});
		});
	}

	$scope.emitCommand = function (deviceId, command) {
		$scope.launchAjaxRequest({
			method: 'POST',
			url: '/device/command/' + deviceId + '/' + command,
			callback: function (err, data) {
				if (err)
					throw err;
				$scope.updateDevices(data);
			}
		});
	}
});