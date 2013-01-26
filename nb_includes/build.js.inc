
/*
 * This file describes building construction procedures.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.baseLocation = startPositions[me];

_global.setBaseLocation = function(loc) {
	baseLocation = loc;
}

_global.enumTrucks = function() {
	return enumDroid(me, DROID_CONSTRUCT);
}

_global.distanceToBase = function(loc) {
	return distBetweenTwoPoints(loc.x, loc.y, baseLocation.x, baseLocation.y);
}

// a shortened version of isStructureAvailable() for convenience
function isAvailable(stat) {
	return isStructureAvailable(stat, me);
}

// a function for checking the presence of enemy units at the construction site
function safeSpot() {
	// currently does nothing
	return true; 
}

function truckFree(truck) {
	if (truck.droidType !== DROID_CONSTRUCT)
		return false;
	if (truck.order === DORDER_BUILD)
		return false;
	if (truck.order === DORDER_HELPBUILD)
		return false;
	if (truck.order === DORDER_LINEBUILD)
		return false;
	return true;
}

// returns one or two free trucks
function getTwoFreeTrucks() {
	var trucks = enumTrucks().filter(truckFree);
	if (trucks.length > 2) {
		var ret = naiveFindClusters(trucks, baseScale / 2);
		if (ret.maxCount >= 2)
			trucks = ret.clusters[ret.maxIdx];
	}
	if (trucks.length > 2)
		trucks.length = 2;
	return trucks;
}

function getFreeTruckAround(x, y) {
	var list = enumTrucks().filter(truckFree).filter(function(droid) { 
		return droidCanReach(droid, x, y); 
	}).sort(function(one, two) {
		return distBetweenTwoPoints(one.x, one.y, x, y) - distBetweenTwoPoints(two.x, two.y, x, y);
	});
	if (list.length > 0)
		return list[0];
}

function buildModule(struct) {
	trucks = getTwoFreeTrucks();
	if (trucks.length <= 0)
		return BUILDRET.FAILURE;
	var moduleInfo = modules.filter(function(item) { return isAvailable(item.module) && item.base === struct.stattype; }).last();
	if (!defined(moduleInfo))
		return BUILDRET.UNAVAILABLE;
	if (struct.modules >= moduleInfo.count)
		return BUILDRET.UNAVAILABLE;
	var success = false;
	for (var i = 0; i < trucks.length; ++i) 
		success = orderDroidBuild(trucks[i], DORDER_BUILD, moduleInfo.module, struct.x, struct.y) || success;
	if (success)
		return BUILDRET.SUCCESS;
	return BUILDRET.FAILURE;
}

function buildBasicStructure(statlist, importance) {
	if (throttled(5000, statlist[0])) return BUILDRET.FAILURE;
	// by default, don't try building things in dangerous locations
	if (!defined(importance))
		importance = IMPORTANCE.PEACETIME;
	trucks = getTwoFreeTrucks();
	if (trucks.length <= 0)
		return BUILDRET.FAILURE;
	// choose structure type (out of the statlist), 
	// together with suitable location
	var idx, loc, avail = false;
	for (var i = 0; i < statlist.length; ++i)
		if (isAvailable(statlist[i])) {
			avail = true;
			if (distanceToBase(trucks[0]) <= baseScale)
				loc = pickStructLocation(trucks[0], statlist[i], trucks[0].x, trucks[0].y);
			else 
				loc = pickStructLocation(trucks[0], statlist[i], baseLocation.x, baseLocation.y);
			idx = i;
			break;
		}
	if (!avail)
		return BUILDRET.UNAVAILABLE;
	if (!defined(loc))
		return BUILDRET.FAILURE;
	if (importance === IMPORTANCE.PEACETIME && !safeSpot(loc.x, loc.y))
		return BUILDRET.FAILURE;
	// now actually build
	var success = false;
	for (var i = 0; i < trucks.length; ++i) 
		success = orderDroidBuild(trucks[i], DORDER_BUILD, statlist[idx], loc.x, loc.y) || success;
	if (success)
		return BUILDRET.SUCCESS;
	return BUILDRET.FAILURE;
}

function finishStructures() {
	var success = false;
	var list = enumStruct(me).filterProperty("status", BEING_BUILT);
	for (var i = 0; i < list.length; ++i) {
		if (success)
			return;
		if (throttled(10000, list[i].id))
			return;
		if (list[i].stattype === RESOURCE_EXTRACTOR)
			return;
		var truck = getFreeTruckAround(list[i].x, list[i].y);
		if (!defined(truck))
			return;
		if (orderDroidObj(truck, DORDER_HELPBUILD, list[i]))
			success = true;
	};
	return success;
}

function buildStructureAround(statlist, loc, unique) {
	var truck = getFreeTruckAround(loc.x, loc.y);
	if (!defined(truck))
		return BUILDRET.FAILURE;
	var stat = statlist.filter(isAvailable).filter(function(s) {
		if (unique !== true)
			return true;
		var list = enumStruct(me, s);
		for (var i = 0; i < list.length; ++i)
			if (distance(list[i], loc) < baseScale / 2)
				return false;
		return true;
	}).last();
	if (!defined(stat))
		return BUILDRET.UNAVAILABLE;
	loc = pickStructLocation(truck, stat, loc.x, loc.y);
	if (!defined(loc))
		return BUILDRET.FAILURE;
	if (orderDroidBuild(truck, DORDER_BUILD, stat, loc.x, loc.y)) 
		return BUILDRET.SUCCESS;
	return BUILDRET.FAILURE;

}

function captureOil(oil) {
	if (!defined(oil))
		return BUILDRET.FAILURE;
	var truck = getFreeTruckAround(oil.x, oil.y);
	if (!defined(truck))
		return BUILDRET.FAILURE;
	var stat = structures.derricks.filter(isAvailable).last();
	if (!defined(stat))
		return BUILDRET.UNAVAILABLE;
	if (throttled(90000, oil.y * mapWidth + oil.x))
		return BUILDRET.FAILURE;
	if (orderDroidBuild(truck, DORDER_BUILD, stat, oil.x, oil.y)) 
		return BUILDRET.SUCCESS;
	return BUILDRET.FAILURE;
}

function buildTowers() {
	var oils = enumStructList(structures.derricks);
	if (oils.length === 0)
		return false;
	var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
	if (withChance(70)) {
		var list = [];
		weaponPaths.forEach(function(statlist) {
			list = list.concat(weaponStatsToDefenses(statlist, DEFROLE.STANDALONE));
		});
		return buildStructureAround(list, oils.random()) !== BUILDRET.UNAVAILABLE;
	}
	var list = [];
	weaponPaths.forEach(function(statlist) {
		list = list.concat(weaponStatsToDefenses(statlist, DEFROLE.FORTRESS));
	});
	list = list.concat(structures.sensors);
	return buildStructureAround(list, oils.random(), true);
}

function buildGateways() {
	var oils = countStructList(structures.derricks);
	if (oils <= 0) 
		return BUILDRET.FAILURE;
	var gates = enumGateways().filter(function(gate) {
		var l = gate.x1 - gate.x2 + gate.y1 - gate.y2;
		if (l < 0) 
			l = -l;
		var cnt = enumRange(gate.x1, gate.y1, l, ALLIES).filterProperty("stattype", DEFENSE).length;
		cnt    += enumRange(gate.x2, gate.y2, l, ALLIES).filterProperty("stattype", DEFENSE).length;
		cnt    -= enumRange(gate.x1, gate.y1, l, ENEMIES).filterProperty("stattype", DEFENSE).length;
		cnt    -= enumRange(gate.x2, gate.y2, l, ENEMIES).filterProperty("stattype", DEFENSE).length;
		return cnt >= 0 && cnt < l;
	}).sort(function(one, two) { return distanceToBase({x: one.x1, y: one.y1}) - distanceToBase({x: two.x1, y: two.y1}); });
	if (gates.length === 0)
		return;
	var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
	var list = [];
	weaponPaths.forEach(function(statlist) {
		list = list.concat(weaponStatsToDefenses(statlist, DEFROLE.GATEWAY));
	});
	if (random(2))
		return buildStructureAround(list, {x: gates[0].x1, y: gates[0].y1}) !== BUILDRET.UNAVAILABLE;
	else
		return buildStructureAround(list, {x: gates[0].x2, y: gates[0].y2}) !== BUILDRET.UNAVAILABLE;
}

function buildArty() {
	var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
	var list = [];
	weaponPaths.forEach(function(statlist) {
		list = list.concat(weaponStatsToDefenses(statlist, DEFROLE.ARTY));
	});
	list = list.reverse();
	return buildBasicStructure(list);
}

_global.buildMinimum = function(statlist, count, importance) {
	if (countStructList(statlist) < count)
		if (buildBasicStructure(statlist, importance) !== BUILDRET.UNAVAILABLE)
			return true;
	return false;
}

function captureSomeOil() {
	if (throttled(500))
		return true;
	function getOilList() {
		var oils = [];
		oilResources.forEach(function(stat) { oils = oils.concat(enumFeature(-1, stat)); });
		oils = oils.concat(enumStructList(structures.derricks).filterProperty("status", BEING_BUILT));
		oils = oils.sort(function(one, two) {
			return distanceToBase(one) - distanceToBase(two);
		});
		if (oils.length > 10)
			oils.length = 10;
		return oils;
	}
	var oils = cached(getOilList, 5000);
	if (countFinishedStructList(structures.derricks) >= 4 * structListLimit(structures.gens))
		return false;

	for (var i = 0; i < oils.length; ++i)
		if (captureOil(oils[i]) === BUILDRET.SUCCESS)
			return true;
	return false;
}

_global.buildMinimumDerricks = function(count) {
	if (countFinishedStructList(structures.derricks) < count)
		if (captureSomeOil())
			return true;
	return false;
}

_global.checkConstruction = function() {
	if (finishStructures())
		return;
	queue("checkConstructionOne");
}

// global only to use with timers
_global.checkConstructionOne = function() {
	if (buildOrder())
		return;
	if (myPower() > personality.maxPower) {
		switch (chooseObjectType()) {
			case 0:
				if (buildMinimum(structures.factories, Infinity)) return true;
			case 1:
				if (buildMinimum(structures.templateFactories, Infinity)) return true;
			case 3:
				if (buildMinimum(structures.vtolFactories, Infinity)) return true;
		}
	}
	queue("checkConstructionTwo");
}

_global.checkConstructionTwo = function() {
	var oils = countFinishedStructList(structures.derricks);
	var gens = countStructList(structures.gens);
	if (oils > 4 * gens)
		if (buildBasicStructure(structures.gens) !== BUILDRET.UNAVAILABLE)
			return;
	if (withChance(50))
		if (captureSomeOil())
			return;
	var str = [];
	for (var i = 0; i < modules.length; ++i)
		str = str.concat(enumStruct(me, modules[i].base));
	for (var i = 0; i < str.length; ++i)
		if (buildModule(str[i]) !== BUILDRET.UNAVAILABLE)
			return;
	if (buildMinimum(structures.vtolPads, enumDroid(me, DROID_WEAPON).filter(isVTOL).length / 2))
		return;
	if (buildBasicStructure(structures.extras) !== BUILDRET.UNAVAILABLE) 
		return;
	if (chooseObjectType() === 2)
		queue("buildDefenses");
}

_global.buildDefenses = function() {
	if (withChance(33)) {
		if (buildTowers()) // includes sensor towers and forts
			return true;
	} else if (withChance(50)) {
		if (buildGateways())
			return true;
	} else
		if (buildArty())
			return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
