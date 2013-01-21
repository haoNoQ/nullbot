
/*
 * This file includes generic functions for improving CPU intensity,
 * like caching and throttling mechanisms.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

// to make sure some function is not called more often than every 2 seconds, 
// put something like that at start the of its body:
//		if (throttled(2000)) return;
// if your function requires several throttling cases, you can use the optional
// notes value:
//		if (throttled(60000, player)) return;


_global.throttled = function(interval, notes) {
	if (typeof(arguments.callee.caller.throttleTimes) === "undefined")
		arguments.callee.caller.throttleTimes = {};
	if (typeof(arguments.callee.caller.throttleTimes[notes]) === "undefined") {
		arguments.callee.caller.throttleTimes[notes] = gameTime;
		return false;
	}
	if (gameTime - arguments.callee.caller.throttleTimes[notes] < interval)
		return true;
	arguments.callee.caller.throttleTimes[notes] = gameTime;
	return false;
}

// to cache a function's output value and make sure it's not re-calculated too often,
// use the following trick:
//		function calculateValue(params) {
//			function uncached(params) {
//				// do heavy calculations
//			}
//			return cached(uncached, 10000);
//		}
// add necessary notes to the cached() call if necessary, similar to how you do it
// for throttled().

_global.cached = function(whatToCall, interval, notes) {
	if (typeof(arguments.callee.caller.cachedTimes) === "undefined") {
		arguments.callee.caller.cachedTimes = {};
		arguments.callee.caller.cachedValues = {};
	}
	var t = arguments.callee.caller.cachedTimes[notes];
	if (typeof(t) === "undefined" || gameTime - t >= interval) {
		arguments.callee.caller.cachedValues[notes] = whatToCall();
		arguments.callee.caller.cachedTimes[notes] = gameTime;
	}
	return arguments.callee.caller.cachedValues[notes];
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
