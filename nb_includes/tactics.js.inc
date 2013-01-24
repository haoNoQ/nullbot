
/*
 * This file is responsible for moving combat unit around.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

function regroup(gr) {
	if (inPanic()) 
		return enumGroup(gr).filter(checkRepaired);
	var size = attackGroupSize();
	if (gr === miscGroup)
		size = personality.minMiscTanks;
	if (size < groupSize(gr) / 2)
		size = groupSize(gr) / 2;
	var ret = naiveFindClusters(enumGroup(gr).filter(checkRepaired), (baseScale / 3));
	if (ret.maxCount === 0)
		return [];
	for (var i = 0; i < ret.clusters.length; ++i)
		if (i !== ret.maxIdx) 
			for (var j = 0; j < ret.clusters[i].length; ++j) 
				orderDroidLoc(ret.clusters[i][j], DORDER_MOVE, ret.xav[ret.maxIdx], ret.yav[ret.maxIdx]);
	if (ret.maxCount < size) {
		for (var j = 0; j < ret.clusters[ret.maxIdx].length; ++j) {
			if (groupInDanger(gr))
				orderDroid(ret.clusters[ret.maxIdx][j], DORDER_RTB);
			else
				orderDroid(ret.clusters[ret.maxIdx][j], DORDER_STOP);
		}
		return [];
	}
	return ret.clusters[ret.maxIdx];
}

function weHaveRepair() {
	function uncached() {
		return enumStruct(me, REPAIR_FACILITY).length > 0;
	}
	return cached(uncached, 1000);
}

function checkRepaired(droid) {
	if (!weHaveRepair())
		return true;
	if (droid.order === DORDER_RTR)
		return false;
	if (droid.health < personality.repairAt) {
		orderDroid(droid, DORDER_RTR);
		return false;
	}
	return true;
}

function droidFree(droid) {
	if (droid.order === DORDER_SCOUT)
		return false;
	if (droid.order === DORDER_ATTACK)
		return false;
	if (droid.order === DORDER_RTR)
		return false;
	if (!checkRepaired(droid))
		return false;
	return true;
}

function attackTarget(droid) {
	var target = findTarget(droid.group);
	if (droid.group != miscGroup)
		if (!defined(target) || !droidCanReach(droid, target.x, target.y)) {
			groupDroid(droid);
			return;
		}
	if (defined(target))
		orderDroidLoc(droid, DORDER_SCOUT, target.x, target.y);
}

_global.fallBack = function(droid, threat) {
	if (droid.order === DORDER_MOVE)
		return;
	var x = droid.x - (baseScale / 7) * (threat.x - droid.x) / distBetweenTwoPoints(threat.x, threat.y, droid.x, droid.y);
	var y = droid.y - (baseScale / 7) * (threat.y - droid.y) / distBetweenTwoPoints(threat.x, threat.y, droid.x, droid.y);
	if (x < 1 || y < 1 || x > mapWidth - 2 || y > mapHeight - 2)
		return;
	orderDroidLoc(droid, DORDER_MOVE, x, y);
}

_global.checkAttack = function() {
	for (var i = 0; i <= MAX_GROUPS; ++i)
		if (!throttled(3000, i)) {
			regroup(i).forEach(attackTarget);
			break;
		}
}

_global.inPanic = function() {
	function uncached() {
		var badGuys = enumRange(baseLocation.x, baseLocation.y, baseScale, ENEMIES).length;
		var goodGuys = enumRange(baseLocation.x, baseLocation.y, baseScale, ALLIES).filter(function(object) {
			return object.type === DROID && (object.droidType === DROID_WEAPON || object.droidType === DROID_CYBORG);
		}).length;
		return 3 * badGuys > 2 * goodGuys;
	}
	return cached(uncached, 10000);
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
