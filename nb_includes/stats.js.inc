
/*
 * This file contain functions for manipulating stats defined by rulesets.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.isHoverPropulsion = function(str) {
	return propulsionStats.hover.someProperty("stat", str);
}

_global.isVtolPropulsion = function(str) {
	return propulsionStats.vtol.someProperty("stat", str);
}

_global.iHaveHover = function() {
	for (var i = 0; i < propulsionStats.hover.length; ++i)
		if (componentAvailable(propulsionStats.hover[i].stat))
			return true;
	return false;
}

_global.iHaveVtol = function() {
	for (var i = 0; i < propulsionStats.vtol.length; ++i)
		if (componentAvailable(propulsionStats.vtol[i].stat))
			return true;
	return false;
}

// works with stored droid objects too!
_global.safeIsVtol = function(droid) {
	return isVtolPropulsion(droid.propulsion);
}

_global.enumStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ.concat(enumStruct(player, new_value)); }, []);
}

_global.countStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ + countStruct(new_value, player); }, 0);
}

_global.enumFinishedStructList = function(list, player) {
	return enumStructList(list, player).filterProperty("status", BUILT);
}

_global.countFinishedStructList = function(list, player) {
	return enumFinishedStructList(list, player).length;
}

_global.enumIdleStructList = function(list, player) {
	return enumFinishedStructList(list, player).filter(structureIdle);
}

_global.structListLimit = function(list) {
	var ret = 0;
	for (var i = 0; i < list.length; ++i)
		ret += getStructureLimit(list[i]);
	return ret;
}

_global.guessWeaponRole = function(name) {
	for (var stat in weaponStats) {
		if (weaponStats[stat].weapons.someProperty("stat", name))
			return weaponStats[stat].role;
		if (weaponStats[stat].vtols.someProperty("stat", name))
			return weaponStats[stat].role;
		if (weaponStats[stat].templates.some(function(i) { return i.weapons.indexOf(name) > -1; }))
			return weaponStats[stat].role;
	}
	niceDebug("Ruleset warning: Couldn't guess weapon role of", name);
}

function guessWeaponMicro(name) {
	function uncached() {
		for (var stat in weaponStats)
			if (weaponStats[stat].weapons.someProperty("stat", name))
				return weaponStats[stat].micro;
	}
	return cached(uncached, Infinity, name);
}

_global.guessDroidMicro = function(droid) {
	for (var i = 0; i < droid.weapons.length; ++i) {
		var ret = guessWeaponMicro(droid.weapons[i].name);
		if (ret !== MICRO.RANGED)
			return ret;
	}
	return MICRO.RANGED;
}

_global.guessBodyClass = function(name) {
	if (bodyStats.thermal.someProperty("stat", name))
		return BODYCLASS.THERMAL;
	if (bodyStats.kinetic.someProperty("stat", name))
		return BODYCLASS.KINETIC;
	niceDebug("Ruleset warning: Couldn't guess body class of", name);
}

_global.guessBodyWeight = function(name) {
	var ret;
	for (var stat in bodyStats) {
		ret = bodyStats[stat].filterProperty("stat", name).last();
		if (defined(ret))
			return ret.weight;
	}
	niceDebug("Ruleset warning: Couldn't guess body weight of", name);
}

_global.guessWeaponWeight = function(name) {
	var ret;
	for (var stat in weaponStats) {
		ret = weaponStats[stat].weapons.filterProperty("stat", name).last();
		if (defined(ret))
			return ret.weight;
		ret = weaponStats[stat].vtols.filterProperty("stat", name).last();
		if (defined(ret))
			return ret.weight;
	}
	niceDebug("Ruleset warning: Couldn't guess weapon weight of", name);
}

//
// here be functions for querying out research paths 
//

function statsToResList(path) {
	var ret = [];
	for (var i = 0; i < path.length; ++i) 
		ret.push(path[i].res);
	return ret;
}

_global.bodyStatsToResList = function(path) {
	return statsToResList(path);
}

_global.propulsionStatsToResList = function(path) {
	return statsToResList(path);
}

_global.weaponStatsToResList = function(path, aspect) {
	var ret = [];
	if (!defined(aspect) || aspect === RESASPECTS.WEAPONS) {
		for (var i = 0; i < path.weapons.length; ++i)
			ret.push(path.weapons[i].res);
		for (var i = 0; i < path.vtols.length; ++i)
			ret.push(path.vtols[i].res);
		for (var i = 0; i < path.templates.length; ++i)
			ret.push(path.templates[i].res);
	}
	if (!defined(aspect) || aspect === RESASPECTS.DEFENSES) {
		for (var i = 0; i < path.defenses.length; ++i)
			ret.push(path.defenses[i].res);
	}
	if (!defined(aspect) || aspect === RESASPECTS.EXTRAS)
		ret = ret.concat(path.extras);
	return ret;
}

//
// here be functions for querying out component lists
//

_global.bodyStatsToComponents = function(stats, usage, weight) {
	var ret = [];
	for (var i = 0; i < stats.length; ++i)
		if (!defined(weight) || stats[i].weight === weight)
			if (stats[i].usage & usage)
				ret.push(stats[i].stat);
	ret.reverse();
	return ret;
}

_global.propulsionStatsToComponents = function(stats) {
	var ret = [];
	for (var i = 0; i < stats.length; ++i)
		ret.push(stats[i].stat);
	ret.reverse();
	return ret;
}

_global.weaponStatsToComponents = function(stats, weight) {
	var ret = [];
	for (var i = 0; i < stats.weapons.length; ++i) {
		if (!defined(weight) || stats.weapons[i].weight === weight)
			ret.push(stats.weapons[i].stat);
	}
	ret.reverse();
	return ret;
}

_global.weaponStatsToVtolComponents = function(stats, weight) {
	var ret = [];
	for (var i = 0; i < stats.vtols.length; ++i) {
		if (!defined(weight) || stats.vtols[i].weight === weight)
			ret.push(stats.vtols[i].stat);
	}
	ret.reverse();
	return ret;
}

//
// here be functions for querying out defensive structures
//

_global.weaponStatsToDefenses = function(stats, defrole) {
	var ret = [];
	for (var i = 0; i < stats.defenses.length; ++i)
		if (!defined(defrole) || stats.defenses[i].defrole === defrole)
			ret.push(stats.defenses[i].stat);
	// reverse not needed here
	return ret;
}

_global.weaponStatsListToAvailableDefenses = function(list, defrole) {
	return list.reduce(function(one, two) { return one.concat(weaponStatsToDefenses(two, defrole).filter(isAvailable)); }, []);
}


////////////////////////////////////////////////////////////////////////////////////////////
})(this);
