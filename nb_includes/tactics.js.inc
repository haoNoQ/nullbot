
/*
 * This file is responsible for moving combat unit around.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.MAX_GROUPS = maxPlayers;
_global.miscGroup = MAX_GROUPS;
_global.vtolGroup = miscGroup + 1;
var vtolTarget;
var groupInfo = [];

function GroupInfo() {
	this.cachedTarget = undefined; // the actual target the group is pursuing
	this.lastAttacked = undefined; // gameTime at the moment of the last unit loss
}

function groupsBySize() {
	var ret = [];
	for (var i = 0; i < MAX_GROUPS; ++i)
		if (isEnemy(i))
			ret.push(i);
	ret.sort(function(one, two) { return groupSize(two) - groupSize(one); });
	return ret;
}

function findLargestGroupIn(list) {
	var sizes = []
	for (var i = 0; i < MAX_GROUPS; ++i)
		sizes[i] = 0;
	list.forEach(function(object) {
		if (!defined(object.group) || object.group < 0 && object.group >= MAX_GROUPS)
			return;
		++sizes[object.group];
	});
	var maxCount = 0, maxIdx = 0;
	for (var i = 0; i < sizes.length; ++i)
		if (sizes[i] > maxCount) {
			maxCount = sizes[i];
			maxIdx = i;
		}
	return maxIdx;
}

function findNearestGroup(x, y) {
	var ret = naiveFindClusters(enumDroid(me).filter(function(droid) {
		return !isVTOL(droid) && (droid.droidType === DROID_WEAPON || droid.droidType === DROID_CYBORG);
	}), baseScale / 3);
	if (ret.maxCount === 0)
		return undefined;
	var minDist = Infinity, minIdx;
	var gr = [];
	for (var i = 0; i < ret.clusters.length; ++i) {
		gr[i] = findLargestGroupIn(ret.clusters[i]);
		if (groupSize(gr[i]) > attackGroupSize()) {
			var dist = distBetweenTwoPoints(ret.xav[i], ret.yav[i], x, y);
			if (dist < minDist) {
				minDist = dist;
				minIdx = i;
			}
		}
	}
	if (defined(minIdx))
		return gr[minIdx];
	gr = groupsBySize();
	if (gr.length > 0)
		if (groupSize(gr[0]) > attackGroupSize())
			return gr[0];
}

function targetSuitableForHarass(object) {
	function uncached() {
		return enumRange(object.x, object.y, baseScale / 2, ENEMIES, false).length <= groupSize(miscGroup);
	}
	return cached(uncached, 60000, object.id);
}

function getGroupInfo(gr) {
	if (defined(groupInfo[gr]))
		return groupInfo[gr];
	groupInfo[gr] = new GroupInfo();
	return groupInfo[gr];
}

function findTarget(gr) {
	getGroupInfo(gr); // make sure everything is defined
	if (defined(groupInfo[gr].cachedTarget)) {
		var obj = objFromId(groupInfo[gr].cachedTarget);
		if (gr === miscGroup) {
			// harass group should switch targets when its target gets protected, otherwise targets are permanent
			if (throttled(10000) || targetSuitableForHarass(obj))
				return obj;
		} else
			return obj;
	}
	// find harass targets for the misc group
	if (gr === miscGroup) {
		var list = enumStructList(miscTargets, enumLivingPlayers().filter(isEnemy).random());
		powerUps.forEach(function(stat) { // pick up oil drums and artifacts
			list = list.concat(enumFeature(-1, stat));
		});
		list = list.filter(targetSuitableForHarass).sort(function(one, two) {
			return distanceToBase(one) - distanceToBase(two);
		});
		if (list.length > 0) {
			groupInfo[gr].cachedTarget = list[random(Math.max(3, list.length))];
			return groupInfo[gr].cachedTarget;
		}
	}
	// fund structure targets
	var list = enumStructList(targets, gr);
	if (list.length > 0)
		groupInfo[gr].cachedTarget = list.random();
	else {
		// find remaining droids
		list = enumDroid(gr);
		if (list.length > 0)
			groupInfo[gr].cachedTarget = list.random();
	}
	return groupInfo[gr].cachedTarget;
}

function groupInDanger(gr) {
	getGroupInfo(gr); // make sure everything is defined
	if (!defined(groupInfo[gr].lastAttacked))
		return false;
	return gameTime - groupInfo[gr].lastAttacked < 10000;
}

function regroup(gr) {
	if (inPanic()) 
		return enumGroup(gr).filter(checkRepaired);
	var size = attackGroupSize();
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
	if (droid.group !== miscGroup)
		if (!defined(target) || !droidCanReach(droid, target.x, target.y)) {
			groupDroid(droid);
			return;
		}
	if (defined(target))
		switch (target.type) {
			case STRUCTURE:
				orderDroidLoc(droid, DORDER_SCOUT, target.x, target.y);
				break;
			case DROID:
				orderDroidObj(droid, DORDER_ATTACK, target);
				break;
			case FEATURE:
				orderDroidObj(droid, DORDER_RECOVER, target);
				break;
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

_global.attackGroupSize = function() {
	var ret = personality.minTanks + (gameTime / 300000) * personality.becomeHarder;
	if (ret > personality.maxTanks)
		ret = personality.maxTanks;
	return ret;
}

_global.setTarget = function(object, group) {
	if (!defined(group)) {
		group = findNearestGroup(object.x, object.y);
		vtolTarget = object;
	}
	if (!defined(group))
		return false;
	getGroupInfo(group); // make sure everything is defined
	if (defined(groupInfo[group].cachedTarget))
		if (throttled(10000, group)) // don't switch targets too often
			return false;
	groupInfo[group].cachedTarget = object;
	return true;
}

_global.invalidateTarget = function(object) {
	groupInfo.forEach(function(info) {
		if (defined(info.cachedTarget))
			if (info.cachedTarget.id === object.id)
				info.cachedTarget = undefined;
	});
	if (defined(vtolTarget))
		if (vtolTarget.id === object.id)
			vtolTarget = undefined;
}

_global.groupDroid = function(droid) {
	if (droid.droidType === DROID_WEAPON || droid.droidType === DROID_CYBORG) {
		if (isVTOL(droid)) {
			groupAdd(vtolGroup, droid);
			return;
		}
		if (withChance(100 - groupSize(miscGroup) * 100 / personality.maxMiscTanks)) {
			groupAdd(miscGroup, droid);
			return;
		}
		var grp = groupsBySize().filter(function(i) {
			if (isAlly(i))
				return false;
			if (!defined(findTarget(i)))
				return false;
			if (!droidCanReach(droid, findTarget(i).x, findTarget(i).y))
				return false;
			return true;
		});
		var ret = grp.filter(function(i) {
			return groupSize(i) < attackGroupSize() * 2 && defined(findTarget(i));
		});
		if (ret.length === 0)
			ret = grp;
		if (ret.length === 0)
			ret = [ miscGroup ];
		groupAdd(ret[0], droid);
	}
}

_global.rebalanceGroups = function() {
	if (groupSize(miscGroup) > personality.maxMiscTanks && !throttled(5000)) {
		var list = enumGroup(miscGroup).shuffle();
		for (var i = personality.maxMiscTanks; i < personality.maxMiscTanks + 5 && i < list.length; ++i)
			groupDroid(list[i]);
	}
	if (throttled(5000))
		return;
	var ret = groupsBySize();
	if (ret.length > 0)
		if (ret[0] > 0 && ret[0] < attackGroupSize())
			for (var i = 1; i < ret.length; ++i) {
				var list = enumGroup(ret[i]);
				for (var j = 0; j < list.length; ++j)
					if (droidCanReach(list[i], findTarget(ret[0]).x, findTarget(ret[0]).y)) {
						groupAdd(ret[0], list[i]);
						return;
					}
			}
}

_global.touchGroup = function(gr) {
	getGroupInfo(gr); // make sure everything is defined
	groupInfo[gr].lastAttacked = gameTime;
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

_global.dangerLevel = function(loc) {
	return enumRange(loc.x, loc.y, baseScale / 2, ENEMIES, false).length;
}

_global.checkAttack = function() {
	for (var i = 0; i < MAX_GROUPS; ++i)
		if (!throttled(3000, i)) {
			regroup(i).forEach(attackTarget);
			break;
		}
	if (throttled(1000))
		return;
	enumGroup(miscGroup).filter(checkRepaired).forEach(attackTarget);
	var droids = enumGroup(vtolGroup).filter(vtolReady);
	var target = pickVtolTarget();
	if (!defined(target))
		return;
	var size = dangerLevel(target);
	if (size > attackGroupSize())
		size = attackGroupSize();
	if (droids.length > size)
		droids.forEach(function(droid) {
			orderDroidObj(droid, DORDER_ATTACK, target);
		});
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
