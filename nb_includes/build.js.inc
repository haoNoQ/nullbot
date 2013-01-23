
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

_global.bringTrucksToBase = function() {
	enumTrucks().filter(truckFree).forEach(function(truck) {
		orderDroid(truck, DORDER_RTB);
	});
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
	enumStruct(me).filterProperty("status", BEING_BUILT).forEach(function(struct) {
		if (throttled(10000, struct.id))
			continue;
		var truck = getFreeTruckAround(struct.x, struct.y);
		if (!defined(truck))
			return false;
		if (orderDroidObj(truck, DORDER_HELPBUILD, struct))
			return true;
	});
	return false;
}

function buildStructureAround(statlist, loc) {
	var truck = getFreeTruckAround(loc.x, loc.y);
	if (!defined(truck))
		return BUILDRET.FAILURE;
	var stat = statlist.filter(isAvailable).last();
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
	if (throttled(4 * personality.defenseTimer / oils.length))
		return false;
	var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
	var list = [];
	weaponPaths.forEach(function(statlist) {
		list = list.concat(weaponStatsToDefenses(statlist, DEFROLE.STANDALONE));
	});
	return buildStructureAround(list, oils.random()) !== BUILDRET.UNAVAILABLE;
}

function buildGateways() {
	var oils = countStructList(structures.derricks);
	if (oils <= 0) 
		return BUILDRET.FAILURE;
	var gates = enumGateways().filter(function(gate) {
		var l = gate.x1 - gate.x2 + gate.y1 - gate.y2;
		if (l < 0) 
			l = -l;
		var cnt = enumRange(gate.x1, gate.y1, l, me).filterProperty("stattype", DEFENSE).length;
		cnt    += enumRange(gate.x2, gate.y2, l, me).filterProperty("stattype", DEFENSE).length;
		return cnt < l;
	}).sort(function(one, two) { return distanceToBase({x: one.x1, y: one.y1}) - distanceToBase({x: two.x1, y: two.y1}); });
	if (gates.length === 0)
		return;
	if (throttled(4 * personality.defenseTimer / oils))
		return false;
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

_global.buildMinimum = function(statlist, count, importance) {
	if (countStructList(statlist) < count)
		if (buildBasicStructure(statlist, importance) !== BUILDRET.UNAVAILABLE)
			return true;
	return false;
}

function captureSomeOil() {
	var oils = [];
	oilResources.forEach(function(stat) { oils = oils.concat(enumFeature(-1, stat)); });
	oils = oils.sort(function(one, two) {
		return distanceToBase(one) - distanceToBase(two);
	});
	if (countStructList(structures.derricks) >= 4 * structListLimit(structures.gens))
		return BUILDRET.UNAVAILABLE;
	for (var i = 0; i < oils.length; ++i)
		if (captureOil(oils[i]) === BUILDRET.SUCCESS)
			return true;
	return false;
}

_global.buildMinimumDerricks = function(count) {
	if (countStructList(structures.derricks) < count)
		if (captureSomeOil())
			return true;
	return false;
}

_global.checkConstruction = function() {
	if (finishStructures())
		return true;
	if (buildOrder())
		return true;
	var oils = countStructList(structures.derricks);
	var gens = countStructList(structures.gens);
	if (oils > 4 * gens)
		if (buildBasicStructure(structures.gens) !== BUILDRET.UNAVAILABLE)
			return true;
	if (captureSomeOil())
		return true;
	var str = [];
	for (var i = 0; i < modules.length; ++i)
		str = str.concat(enumStruct(me, modules[i].base));
	for (var i = 0; i < str.length; ++i)
		if (buildModule(str[i]) !== BUILDRET.UNAVAILABLE)
			return true;
	if (buildMinimum(structures.vtolPads, enumDroid(me, DROID_WEAPON).filter(isVTOL).length / 2))
		return true;
	if (random(2)) {
		if (buildTowers())
			return true;
	} else {
		if (buildGateways())
			return true;
	}
	if (buildBasicStructure(structures.extras) !== BUILDRET.UNAVAILABLE) 
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
