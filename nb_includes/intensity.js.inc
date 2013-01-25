
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
// NOTE: it won't work if the function repeatedly dies and gets created again, eg.
// a function defined inside forEach(function(...){...}) can't be throttled

_global.throttled = function(interval, notes) {
	if (!defined(arguments.callee.caller.throttleTimes))
		arguments.callee.caller.throttleTimes = {};
	if (!defined(arguments.callee.caller.throttleTimes[notes])) {
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
// NOTE: it won't work if the function repeatedly dies and gets created again, eg.
// a function defined inside forEach(function(...){...}) can't have caching inside

_global.cached = function(whatToCall, interval, notes) {
	if (!defined(arguments.callee.caller.cachedTimes)) {
		arguments.callee.caller.cachedTimes = {};
		arguments.callee.caller.cachedValues = {};
	}
	var t = arguments.callee.caller.cachedTimes[notes];
	if (!defined(t) || gameTime - t >= interval) {
		arguments.callee.caller.cachedValues[notes] = whatToCall();
		arguments.callee.caller.cachedTimes[notes] = gameTime;
	}
	return arguments.callee.caller.cachedValues[notes];
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
