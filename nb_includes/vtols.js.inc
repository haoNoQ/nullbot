
/*
 * This file controls VTOL behaviour in a way similar to how
 * tactics.js.inc controls tank and cyborg behaviour.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

var vtolTarget;

_global.setVtolTarget = function(object) {
	vtolTarget = object;
}

_global.invalidateVtolTarget = function(object) {
	if (typeof(vtolTarget) !== "undefined") {
		if (vtolTarget.id === object.id)
			vtolTarget = undefined;
	}
}

function pickVtolTarget() {
	function uncached() {
		var enemy = enumLivingPlayers().filter(isEnemy).random();
		var list;
		list = enumDroid(enemy, DROID_WEAPON).filterProperty("canHitAir", true);
		if (list.length > 0) return list.random();
		list = enumDroid(enemy, DROID_CYBORG).filterProperty("canHitAir", true);
		if (list.length > 0) return list.random();
		list = enumStruct(enemy, DEFENSE).filterProperty("canHitAir", true);
		if (list.length > 0) return list.random();
		list = enumStructList(enemy, miscTargets);
		if (list.length > 0) return list.random();
		list = enumStructList(enemy, targets);
		if (list.length > 0) return list.random();
		list = enumDroid(enemy);
		if (list.length > 0) return list.random();
	}
	if (typeof(vtolTarget) !== "undefined")
		return objFromId(vtolTarget);
	vtolTarget = uncached();
	return vtolTarget;
}

function vtolReady(droid) {
	return droid.armed > 0;
}

_global.checkVtols = function() {
	var droids = enumGroup(vtolGroup).filter(vtolReady);
	if (droids.length > personality.minTanks) 
		droids.forEach(function(droid) { orderDroidObj(droid, DORDER_ATTACK, pickVtolTarget()); });
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
