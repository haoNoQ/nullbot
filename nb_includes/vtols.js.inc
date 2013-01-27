
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
	if (defined(vtolTarget)) {
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
		list = enumStructList(miscTargets, enemy);
		if (list.length > 0) return list.random();
		list = enumStructList(targets, enemy);
		if (list.length > 0) return list.random();
		list = enumDroid(enemy);
		if (list.length > 0) return list.random();
	}
	if (defined(vtolTarget))
		return objFromId(vtolTarget);
	vtolTarget = uncached();
	return vtolTarget;
}

function vtolReady(droid) {
	for (var i = 0; i < droid.weapons.length; ++i)
		if (droid.weapons[i].armed > 0)
			return true;
	return false;
}

_global.checkVtols = function() {
	var droids = enumGroup(vtolGroup).filter(vtolReady);
	if (droids.length > personality.minTanks) 
		droids.forEach(function(droid) { orderDroidObj(droid, DORDER_ATTACK, pickVtolTarget()); });
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
