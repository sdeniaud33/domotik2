app.controller('myCtrl', function ($scope, $timeout) {
	$scope.devices = devices;
	$scope.rooms = rooms;

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
				$scope.updateDevices(data);
			}
		})
	}

	$scope.updateDevices = function (data) {
		if (!data.devices)
			return;
		$scope.$apply(function () {
			data.devices.forEach((device) => {
				if (device.type === 'light') {
					$scope.devices[device.id].status = device.status;
				}
				else if (device.type === 'tempSensor') {
					$scope.devices[device.id].temperature = device.temperature;
				}
			});
		});
	}

	$scope.lightSwitch = function (lightId) {
		$scope.launchAjaxRequest({
			method: 'POST',
			url: '/device/light/' + lightId + '/switch',
			callback: function (err, data) {
				if (err)
					throw err;
				$scope.updateDevices(data);
			}
		})
	}

	$scope.switchSingleClick = function (switchId) {
		$scope.launchAjaxRequest({
			method: 'POST',
			url: '/device/switch/' + switchId + '/singleClick',
			callback: function (err, data) {
				if (err)
					throw err;
				$scope.updateDevices(data);
			}
		})
	}

	$scope.switchDoubleClick = function (switchId) {
		$scope.launchAjaxRequest({
			method: 'POST',
			url: '/device/switch/' + switchId + '/doubleClick',
			callback: function (err, data) {
				if (err)
					throw err;
				$scope.updateDevices(data);
			}
		})
	}
});