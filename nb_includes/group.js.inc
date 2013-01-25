
/*
 * This file is responsible for arranging droids into combat groups.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.MAX_GROUPS = maxPlayers;
_global.miscGroup = MAX_GROUPS;
_global.vtolGroup = miscGroup + 1;
var groupInfo = [];

function GroupInfo() {
	this.cachedTarget = undefined; // the actual target the group is pursuing
	this.lastAttacked = undefined; // gameTime at the moment of the last unit loss
}

_global.attackGroupSize = function() {
	var ret = personality.minTanks + (gameTime / 300000) * personality.becomeHarder;
	if (ret > personality.maxTanks) 
		ret = personality.maxTanks;
	return ret;
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
	return cached(uncached, 60000);
}

_global.getGroupInfo = function(gr) {
	if (defined(groupInfo[gr]))
		return groupInfo[gr];
	groupInfo[gr] = new GroupInfo();
	return groupInfo[gr];
}

_global.setTarget = function(object, group) {
	if (!defined(group))
		group = findNearestGroup(object.x, object.y);
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
}

_global.findTarget = function(gr) {
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

_global.groupDroid = function(droid) {
	if (droid.droidType === DROID_WEAPON || droid.droidType === DROID_CYBORG) {
		if (isVTOL(droid)) {
			groupAdd(vtolGroup, droid);
			return;
		}
		if (withChance(20) && groupSize(miscGroup) < personality.maxMiscTanks) {
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

_global.touchGroup = function(gr) {
	getGroupInfo(gr); // make sure everything is defined
	groupInfo[gr].lastAttacked = gameTime;
}

_global.groupInDanger = function(gr) {
	getGroupInfo(gr); // make sure everything is defined
	if (!defined(groupInfo[gr].lastAttacked))
		return false;
	return gameTime - groupInfo[gr].lastAttacked < 10000;
}

_global.rebalanceGroups = function() {
	if (groupSize(miscGroup) > personality.maxMiscTanks && !throttled(5000)) {
		var list = enumGroup(miscGroup).shuffle();
		for (var i = personality.maxMiscTanks; i < personality.maxMiscTanks + 5 && i < list.length; ++i) 
			groupDroid(list[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
