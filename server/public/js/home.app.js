var app = angular.module('myApp', ['ngSanitize']);

app.config(function ($interpolateProvider) {
	// We have to configure start/end delimiters for Angular to avoid conflicts with handlebars
	// HandleBars and Angular both use the same notation {{xxx}}
	// So, now {{xxxx}} will be interpreted by handleBars and {[{xxxxx}]} by Angular.
	$interpolateProvider.startSymbol('{[{').endSymbol('}]}');
});

